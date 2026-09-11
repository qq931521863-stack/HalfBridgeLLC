#ifndef LOOP_ABI_H
#define LOOP_ABI_H

/* ABI-Duty-1: two PLECS DLLs for the existing Duty/SR/Drv plant.
 * Physical units are volts and amperes. Do not scale by 0.1 or ADC bases.
 * Fast does not multiply PI by Ts. t=0 initializes only; first Fast step at 25 us.
 * Slow SampleTime is 5 ms (one StateM tick). mode_cmd is a one-tick pulse, then -1.
 */

enum { LOOP_ABI_VERSION = 1 };
enum { LOOP_FAST_IN_N = 10, LOOP_FAST_OUT_N = 8 };
enum { LOOP_SLOW_IN_N = 10, LOOP_SLOW_OUT_N = 8 };
enum { LOOP_FAST_PARAM_N = 2, LOOP_SLOW_PARAM_N = 2 };

#define LOOP_FAST_TS            25e-6
#define LOOP_SLOW_TS            5e-3
#define LOOP_MODE_HOLD          (-1)

/* Fast inputs */
enum {
	FAST_IN_V_RLY = 0,
	FAST_IN_V_BAT,
	FAST_IN_I_BAT,
	FAST_IN_ENABLE,
	FAST_IN_MODE_CMD,
	FAST_IN_V_REF,
	FAST_IN_I_MAX,
	FAST_IN_HANDSHAKE,
	FAST_IN_CV_MODE,
	FAST_IN_PFC_OK
};

/* Fast outputs */
enum {
	FAST_OUT_DUTY = 0,
	FAST_OUT_SR_EN,
	FAST_OUT_DRV_H,
	FAST_OUT_DRV_L,
	FAST_OUT_FREQ_HZ,
	FAST_OUT_CTR_MODE,
	FAST_OUT_PRE_OK,
	FAST_OUT_I_REF
};

/* Slow inputs */
enum {
	SLOW_IN_V_RLY = 0,
	SLOW_IN_V_BAT,
	SLOW_IN_I_BAT,
	SLOW_IN_CTR_MODE,
	SLOW_IN_PRE_OK,
	SLOW_IN_CHARGE_ENABLE,
	SLOW_IN_HANDSHAKE_REQ,
	SLOW_IN_CV_REQ,
	SLOW_IN_V_REQ,
	SLOW_IN_I_REQ
};

/* Slow outputs — Fast in[3..8] plus two switch commands */
enum {
	SLOW_OUT_ENABLE = 0,
	SLOW_OUT_MODE_CMD,
	SLOW_OUT_V_REF,
	SLOW_OUT_I_MAX,
	SLOW_OUT_HANDSHAKE,
	SLOW_OUT_CV_MODE,
	SLOW_OUT_RELAY_CMD,
	SLOW_OUT_DISCHARGE_CMD
};

#endif
