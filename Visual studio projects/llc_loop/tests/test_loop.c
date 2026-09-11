#include "llc_api.h"
#include "llc_slow.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static int fail(const char *msg)
{
	fprintf(stderr, "FAIL: %s\n", msg);
	return 1;
}

int main(void)
{
	LlcConfig cfg;
	LlcContext *fast;
	LlcSlowContext *slow;
	double fin[LOOP_FAST_IN_N];
	double fout[LOOP_FAST_OUT_N];
	double sin_[LOOP_SLOW_IN_N];
	double sout[LOOP_SLOW_OUT_N];
	int i, saw_duty_move = 0, saw_bms = 0, pfm_off = 0;
	double last_duty = -1.0;

	cfg.fast_ts = LOOP_FAST_TS;
	cfg.slow_ts = LOOP_SLOW_TS;
	cfg.abi_version = LOOP_ABI_VERSION;

	fast = llcCreate(&cfg);
	slow = llcSlowCreate(&cfg);
	if (!fast || !slow)
		return fail("create");

	memset(fin, 0, sizeof(fin));
	fin[FAST_IN_V_RLY] = 10.0;
	fin[FAST_IN_V_BAT] = 32.0;
	fin[FAST_IN_I_BAT] = 0.2;
	fin[FAST_IN_ENABLE] = 1.0;
	fin[FAST_IN_MODE_CMD] = BmsStar;
	fin[FAST_IN_V_REF] = 34.0;
	fin[FAST_IN_I_MAX] = 4.0;
	fin[FAST_IN_HANDSHAKE] = 1.0;
	fin[FAST_IN_CV_MODE] = 1.0;
	fin[FAST_IN_PFC_OK] = 1.0;

	llcFastStep(fast, fin, fout);
	if (fabs(fout[FAST_OUT_DUTY] - 0.20) < 1e-12)
		return fail("duty still hardcoded 0.20");
	if (fout[FAST_OUT_CTR_MODE] != BmsStar)
		return fail("BmsStar not applied");
	if (fout[FAST_OUT_FREQ_HZ] < 1000.0)
		return fail("freq not driven");

	fin[FAST_IN_MODE_CMD] = LOOP_MODE_HOLD;
	for (i = 0; i < 200; i++) {
		fin[FAST_IN_V_RLY] = 10.0 + 0.1 * i;
		llcFastStep(fast, fin, fout);
		if (fout[FAST_OUT_DUTY] != last_duty && last_duty >= 0.0)
			saw_duty_move = 1;
		last_duty = fout[FAST_OUT_DUTY];
		if (fout[FAST_OUT_CTR_MODE] == BmsStar)
			saw_bms = 1;
	}
	if (!saw_duty_move)
		return fail("duty did not change after BmsStar hold");
	if (!saw_bms)
		return fail("lost BmsStar while mode_cmd held at -1");

	/* Force PFM: request OvLoadPFM once, expect gate/duty off. */
	fin[FAST_IN_MODE_CMD] = OvLoadPFM;
	llcFastStep(fast, fin, fout);
	if (fout[FAST_OUT_DUTY] != 0.0 || fout[FAST_OUT_DRV_H] != 0.0 ||
		fout[FAST_OUT_DRV_L] != 0.0)
		return fail("OvLoadPFM must shut off");
	pfm_off = 1;

	memset(sin_, 0, sizeof(sin_));
	sin_[SLOW_IN_V_RLY] = 10.0;
	sin_[SLOW_IN_V_BAT] = 32.0;
	sin_[SLOW_IN_I_BAT] = 0.2;
	sin_[SLOW_IN_CHARGE_ENABLE] = 1.0;
	sin_[SLOW_IN_HANDSHAKE_REQ] = 1.0;
	sin_[SLOW_IN_CV_REQ] = 1.0;
	sin_[SLOW_IN_V_REQ] = 54.0;
	sin_[SLOW_IN_I_REQ] = 4.0;
	for (i = 0; i < 160; i++) {
		llcSlowStep(slow, sin_, sout);
		if (sout[SLOW_OUT_MODE_CMD] == BmsStar)
			break;
	}
	if (i >= 160)
		return fail("slow never pulsed BmsStar");
	llcSlowStep(slow, sin_, sout);
	if (sout[SLOW_OUT_MODE_CMD] != LOOP_MODE_HOLD)
		return fail("mode_cmd did not return to -1");

	llcDestroy(fast);
	llcSlowDestroy(slow);
	printf("PASS duty_move=%d pfm_off=%d slow_pulse_at=%d\n", saw_duty_move, pfm_off, i);
	return 0;
}
