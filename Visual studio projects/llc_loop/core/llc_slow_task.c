/* Slow supervisor: ChargeOn / ConVoWakeup entry edges only. No protection or comms.
 * One call is one 5 ms StateM tick. mode_cmd pulses for that tick, then -1. */
#include "llc_slow.h"
#include "llc_types.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

enum { S_INIT = 0, S_WAKE = 1, S_STANDBY = 2, S_CHARGE = 3, S_FULL = 4 };

struct LlcSlowContext {
	LlcConfig cfg;
	uint32_t tick;
	uint16_t delay_ini, wake_count, charge_count;
	uint8_t sys_state, run_state, old_state;
	uint8_t handshake, enable, cv_mode, last_cv_mode, pre_ok;
	int ctr_mode, pulse_mode;
	float vr, vb, ib, v_request, i_request;
	uint8_t relay_cmd, discharge_cmd;
};

LlcSlowContext *llcSlowCreate(const LlcConfig *cfg)
{
	LlcSlowContext *c;
	if (!cfg || cfg->abi_version != LOOP_ABI_VERSION || cfg->slow_ts != LOOP_SLOW_TS)
		return NULL;
	c = (LlcSlowContext *)calloc(1, sizeof(*c));
	if (c) {
		c->cfg = *cfg;
		llcSlowReset(c);
	}
	return c;
}

void llcSlowReset(LlcSlowContext *c)
{
	LlcConfig cfg;
	if (!c)
		return;
	cfg = c->cfg;
	memset(c, 0, sizeof(*c));
	c->cfg = cfg;
	c->pulse_mode = LOOP_MODE_HOLD;
}

void llcSlowDestroy(LlcSlowContext *c)
{
	free(c);
}

static int integer_in_range(double v, double max)
{
	return isfinite(v) && v >= 0.0 && v <= max && floor(v) == v;
}

static int slow_inputs_valid(const double *in)
{
	int k;
	for (k = 0; k < LOOP_SLOW_IN_N; k++) {
		if (!isfinite(in[k]))
			return 0;
	}
	if (fabs(in[SLOW_IN_V_RLY]) > 1e6 || fabs(in[SLOW_IN_V_BAT]) > 1e6 ||
		fabs(in[SLOW_IN_I_BAT]) > 1e6)
		return 0;
	if (in[SLOW_IN_V_REQ] < 0.0 || in[SLOW_IN_V_REQ] > 1e6)
		return 0;
	if (in[SLOW_IN_I_REQ] < 0.0 || in[SLOW_IN_I_REQ] > 1e6)
		return 0;
	if (!integer_in_range(in[SLOW_IN_CTR_MODE], 8.0))
		return 0;
	if (!integer_in_range(in[SLOW_IN_PRE_OK], 2.0))
		return 0;
	if (!integer_in_range(in[SLOW_IN_CHARGE_ENABLE], 1.0))
		return 0;
	if (!integer_in_range(in[SLOW_IN_HANDSHAKE_REQ], 2.0))
		return 0;
	if (!integer_in_range(in[SLOW_IN_CV_REQ], 2.0))
		return 0;
	return 1;
}

static void pulse(LlcSlowContext *c, int mode)
{
	c->pulse_mode = mode;
}

static void application(LlcSlowContext *c)
{
	switch (c->sys_state) {
	case S_INIT:
		if (!c->handshake)
			c->sys_state = S_WAKE;
		else
			c->sys_state = S_STANDBY;
		break;
	case S_WAKE:
		if (c->handshake)
			c->sys_state = S_STANDBY;
		break;
	case S_STANDBY:
		if ((c->handshake == 1 || c->handshake == 2) && c->enable)
			c->sys_state = S_CHARGE;
		else if (!c->handshake)
			c->sys_state = S_WAKE;
		break;
	case S_CHARGE:
		if (c->handshake == 2) {
			if (c->cv_mode == 0 && c->i_request >= 0.1f)
				c->cv_mode = 1;
			else if (c->cv_mode == 0)
				c->cv_mode = (uint8_t)(c->v_request >= 3.0f ? 2 : 0);
		} else if (c->handshake == 1)
			c->cv_mode = 1;
		if ((c->handshake == 1 || c->handshake == 2) && c->enable) {
			/* remain */
		} else if ((c->handshake == 1 || c->handshake == 2) && !c->enable)
			c->sys_state = S_STANDBY;
		else if (!c->handshake)
			c->sys_state = S_WAKE;
		if (c->sys_state != S_CHARGE)
			c->cv_mode = 0;
		break;
	default:
		c->sys_state = S_INIT;
		break;
	}
	if (c->delay_ini >= 100) {
		if (c->run_state != c->sys_state)
			c->old_state = S_INIT;
		c->run_state = c->sys_state;
	} else
		c->run_state = S_INIT;
}

static void charging(LlcSlowContext *c)
{
	int start;
	if (c->handshake == 1)
		c->cv_mode = 1;
	start = (c->old_state != c->run_state) ||
		(c->cv_mode != c->last_cv_mode && c->cv_mode);
	if (!start)
		return;
	pulse(c, NoSelect);
	if (c->ib >= 1.0f)
		return;
	if (c->cv_mode == 1) {
		if (c->vr < c->vb - 2.0f && c->v_request >= 30.0f) {
			if (c->vb >= 29.0f && c->vb < 60.0f) {
				c->discharge_cmd = 0;
				c->relay_cmd = 0;
				c->charge_count++;
				if (c->charge_count >= 40) {
					c->old_state = c->run_state;
					pulse(c, BmsStar);
					c->last_cv_mode = c->cv_mode;
					c->charge_count = 0;
				}
			} else {
				c->charge_count = 0;
				c->discharge_cmd = 1;
				c->relay_cmd = 0;
			}
		} else {
			c->charge_count = 0;
			c->discharge_cmd = 1;
			c->relay_cmd = 0;
		}
	} else if (c->cv_mode == 2) {
		if (c->vr < c->v_request - 2.0f && c->v_request >= 30.0f) {
			c->discharge_cmd = 0;
			c->relay_cmd = 0;
			c->charge_count++;
			if (c->charge_count >= 30) {
				c->old_state = c->run_state;
				pulse(c, SoftCurSt);
				c->last_cv_mode = c->cv_mode;
				c->charge_count = 0;
			}
		} else {
			c->charge_count = 0;
			c->discharge_cmd = 1;
			c->relay_cmd = 0;
		}
	}
}

static void supervisor(LlcSlowContext *c)
{
	switch (c->run_state) {
	case S_INIT:
		pulse(c, NoSelect);
		c->old_state = c->run_state;
		c->discharge_cmd = 1;
		if (c->enable) {
			if (c->delay_ini < 100)
				c->delay_ini++;
		} else
			c->delay_ini = 0;
		break;
	case S_WAKE:
		if (c->delay_ini < 100)
			break;
		if (c->old_state != c->run_state) {
			if (c->vr < 28.0f && c->vb < 5.0f && !c->relay_cmd) {
				c->wake_count++;
				if (c->wake_count >= 3) {
					c->old_state = c->run_state;
					c->discharge_cmd = 0;
					c->relay_cmd = 0;
					pulse(c, SoftStar);
					c->wake_count = 0;
				}
			} else {
				c->wake_count = 0;
				c->discharge_cmd = 1;
				c->relay_cmd = 0;
			}
		} else
			c->wake_count = 0;
		break;
	case S_STANDBY:
	case S_FULL:
		if (c->delay_ini >= 100 && c->old_state != c->run_state) {
			c->old_state = c->run_state;
			c->discharge_cmd = 1;
			c->relay_cmd = 0;
			pulse(c, NoSelect);
		}
		break;
	case S_CHARGE:
		if (c->delay_ini >= 100)
			charging(c);
		break;
	default:
		break;
	}
}

void llcSlowStep(LlcSlowContext *c, const double in[LOOP_SLOW_IN_N], double out[LOOP_SLOW_OUT_N])
{
	float v_ref;
	if (!out)
		return;
	memset(out, 0, LOOP_SLOW_OUT_N * sizeof(*out));
	if (!c)
		return;
	c->tick++;
	c->pulse_mode = LOOP_MODE_HOLD;
	if (!in || !slow_inputs_valid(in)) {
		out[SLOW_OUT_MODE_CMD] = NoSelect;
		return;
	}

	c->vr = (float)in[SLOW_IN_V_RLY];
	c->vb = (float)in[SLOW_IN_V_BAT];
	c->ib = (float)in[SLOW_IN_I_BAT];
	c->ctr_mode = (int)in[SLOW_IN_CTR_MODE];
	c->pre_ok = (uint8_t)in[SLOW_IN_PRE_OK];
	c->enable = (uint8_t)in[SLOW_IN_CHARGE_ENABLE];
	c->handshake = (uint8_t)in[SLOW_IN_HANDSHAKE_REQ];
	c->v_request = (float)in[SLOW_IN_V_REQ];
	c->i_request = (float)in[SLOW_IN_I_REQ];
	if (c->handshake != 1)
		c->cv_mode = (uint8_t)in[SLOW_IN_CV_REQ];

	(void)c->ctr_mode;
	(void)c->pre_ok;

	application(c);
	supervisor(c);

	if (c->pulse_mode == BmsStar)
		v_ref = c->vb + 2.0f;
	else if (c->pulse_mode == SoftStar)
		v_ref = 30.0f;
	else
		v_ref = c->v_request;

	out[SLOW_OUT_ENABLE] = c->enable;
	out[SLOW_OUT_MODE_CMD] = c->pulse_mode;
	out[SLOW_OUT_V_REF] = v_ref;
	out[SLOW_OUT_I_MAX] = c->i_request;
	out[SLOW_OUT_HANDSHAKE] = c->handshake;
	out[SLOW_OUT_CV_MODE] = c->cv_mode;
	out[SLOW_OUT_RELAY_CMD] = c->relay_cmd;
	out[SLOW_OUT_DISCHARGE_CMD] = c->discharge_cmd;
}
