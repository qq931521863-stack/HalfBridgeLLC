#ifndef LLC_CONTEXT_H
#define LLC_CONTEXT_H
#include "llc_api.h"

typedef struct {
	uint8_t DrvH, DrvL, SynDrv;
	float Plv, Duty, Sr_Atime, Sr_Btime, Sr_Dtime;
	uint16_t DrvDtime;
} LlcDriver;

struct LlcContext {
	LlcConfig cfg;
	Power_interFace_t control;
	pi_str pfm_pi, v_pi, i_pi;
	filter_str filter_one, filter_two, filter_rly, filter_v, filter_i;
	LlcSample sample;
	LlcDriver driver;
	uint32_t fast_tick, diagnostic_bits, fast_fault_bits, slow_fault_bits;
	int last_mode_cmd;
	uint16_t delay_sr, soft_pre_num, precharge_time, precharge_time2, cv_delay, star_time;
	uint8_t lpf_initialized, relay, discharge, llc_enable, pfc_ok;
	uint8_t handshake, cv_mode, enable;
	float i_request, v_request, vac_rms_fir, lim_ref, vtemp, test_duty;
	uint8_t otp_level;
};

void llcPowerCtrHandle(LlcContext *ctx);
void llcPowerIniPidVar(LlcContext *ctx);
void llcSampleLpfHandle(LlcContext *ctx);
#endif
