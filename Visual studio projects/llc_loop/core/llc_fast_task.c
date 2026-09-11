/* Fast host: sample + mode_cmd edge + PowerCtrHandle. No protection, CAN, or HRTIM. */
#include "llc_context.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

LlcContext *llcCreate(const LlcConfig *cfg)
{
	LlcContext *c;
	if (!cfg || cfg->abi_version != LOOP_ABI_VERSION || cfg->fast_ts != LOOP_FAST_TS)
		return NULL;
	c = (LlcContext *)calloc(1, sizeof(*c));
	if (c) {
		c->cfg = *cfg;
		llcReset(c);
	}
	return c;
}

void llcReset(LlcContext *c)
{
	LlcConfig cfg;
	if (!c)
		return;
	cfg = c->cfg;
	memset(c, 0, sizeof(*c));
	c->cfg = cfg;
	c->lim_ref = 0.05f;
	c->driver.Plv = POW_MAX_PLV;
	c->last_mode_cmd = LOOP_MODE_HOLD;
	llcPowerIniPidVar(c);
}

void llcDestroy(LlcContext *c)
{
	free(c);
}

void llcSampleStep(LlcContext *c, float vr, float vb, float ib)
{
	if (!c)
		return;
	if (!isfinite(vr) || !isfinite(vb) || !isfinite(ib) ||
		fabsf(vr) > 1e6f || fabsf(vb) > 1e6f || fabsf(ib) > 1e6f) {
		c->diagnostic_bits |= LLC_DIAG_INVALID_INPUT;
		return;
	}
	c->sample.v_relay = vr;
	c->sample.v_bat = vb;
	c->sample.i_bat = ib;
	c->sample.v_bat_fir = c->sample.v_bat_fir * 0.999f + vb * 0.001f;
	c->sample.i_bat_fir = c->sample.i_bat_fir * 0.999f + ib * 0.001f;
	llcSampleLpfHandle(c);
}

static int integer_in_range(double v, double max)
{
	return isfinite(v) && v >= 0.0 && v <= max && floor(v) == v;
}

static int fast_inputs_valid(const double *in)
{
	int j;
	for (j = 0; j < LOOP_FAST_IN_N; j++) {
		if (!isfinite(in[j]))
			return 0;
	}
	if (fabs(in[FAST_IN_V_RLY]) > 1e6 || fabs(in[FAST_IN_V_BAT]) > 1e6 ||
		fabs(in[FAST_IN_I_BAT]) > 1e6)
		return 0;
	if (in[FAST_IN_I_MAX] < 0.0 || in[FAST_IN_I_MAX] > 1e6)
		return 0;
	if (in[FAST_IN_V_REF] < 0.0 || in[FAST_IN_V_REF] > 1e6)
		return 0;
	if (!integer_in_range(in[FAST_IN_ENABLE], 1.0))
		return 0;
	if (!(in[FAST_IN_MODE_CMD] == LOOP_MODE_HOLD || integer_in_range(in[FAST_IN_MODE_CMD], 8.0)))
		return 0;
	if (!integer_in_range(in[FAST_IN_HANDSHAKE], 2.0))
		return 0;
	if (!integer_in_range(in[FAST_IN_CV_MODE], 2.0))
		return 0;
	if (!integer_in_range(in[FAST_IN_PFC_OK], 1.0))
		return 0;
	return 1;
}

static void apply_mode_cmd(LlcContext *c, int mode_cmd)
{
	if (mode_cmd < 0) {
		c->last_mode_cmd = LOOP_MODE_HOLD;
		return;
	}
	if (mode_cmd == c->last_mode_cmd)
		return;
	c->control.CtrMode = (CtrMode_st)mode_cmd;
	c->last_mode_cmd = mode_cmd;
}

void llcFastStep(LlcContext *c, const double in[LOOP_FAST_IN_N], double out[LOOP_FAST_OUT_N])
{
	int blocked;
	if (!out)
		return;
	memset(out, 0, LOOP_FAST_OUT_N * sizeof(*out));
	if (!c)
		return;
	c->fast_tick++;
	if (!in || !fast_inputs_valid(in)) {
		c->diagnostic_bits |= LLC_DIAG_INVALID_INPUT;
		goto emit;
	}

	c->enable = (uint8_t)in[FAST_IN_ENABLE];
	c->pfc_ok = (uint8_t)in[FAST_IN_PFC_OK];
	c->v_request = (float)in[FAST_IN_V_REF];
	c->i_request = (float)in[FAST_IN_I_MAX];
	c->handshake = (uint8_t)in[FAST_IN_HANDSHAKE];
	c->cv_mode = (uint8_t)in[FAST_IN_CV_MODE];
	c->control.Volt_Ref = c->v_request;
	c->control.CurrentMax = c->i_request;

	llcSampleStep(c, (float)in[FAST_IN_V_RLY], (float)in[FAST_IN_V_BAT],
		(float)in[FAST_IN_I_BAT]);

	if (!c->enable || !c->pfc_ok) {
		c->control.CtrMode = NoSelect;
		c->control.Curr_REF = 0;
		c->last_mode_cmd = LOOP_MODE_HOLD;
	} else {
		apply_mode_cmd(c, (int)in[FAST_IN_MODE_CMD]);
	}

	llcPowerCtrHandle(c);

	if (c->control.CtrMode == OvLoadPFM) {
		c->diagnostic_bits |= LLC_DIAG_MISSING_PFM;
		c->driver.DrvH = 0;
		c->driver.DrvL = 0;
		c->driver.SynDrv = 0;
		c->driver.Duty = 0.0f;
	}

emit:
	blocked = !c->enable || !c->pfc_ok ||
		(c->diagnostic_bits & (LLC_DIAG_MISSING_PFM | LLC_DIAG_INVALID_INPUT));
	if (blocked) {
		c->driver.DrvH = 0;
		c->driver.DrvL = 0;
		c->driver.SynDrv = 0;
		if (c->diagnostic_bits & (LLC_DIAG_MISSING_PFM | LLC_DIAG_INVALID_INPUT))
			c->driver.Duty = 0.0f;
	}

	out[FAST_OUT_DUTY] = c->driver.Duty;
	out[FAST_OUT_SR_EN] = c->driver.SynDrv;
	out[FAST_OUT_DRV_H] = c->driver.DrvH;
	out[FAST_OUT_DRV_L] = c->driver.DrvL;
	out[FAST_OUT_FREQ_HZ] = c->driver.Plv;
	out[FAST_OUT_CTR_MODE] = c->control.CtrMode;
	out[FAST_OUT_PRE_OK] = c->control.preOK;
	out[FAST_OUT_I_REF] = c->control.Curr_REF;
}
