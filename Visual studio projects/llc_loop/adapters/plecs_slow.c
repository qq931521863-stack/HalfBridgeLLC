#include "DllHeader.h"
#include "llc_slow.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
	LlcSlowContext *core;
	double last_time;
	double held[LOOP_SLOW_OUT_N];
} SlowAdapter;

DLLEXPORT void plecsSetSizes(struct SimulationSizes *s)
{
	s->numInputs = LOOP_SLOW_IN_N;
	s->numOutputs = LOOP_SLOW_OUT_N;
	s->numStates = 0;
	s->numParameters = LOOP_SLOW_PARAM_N;
}

DLLEXPORT void plecsStart(struct SimulationState *s)
{
	LlcConfig cfg;
	SlowAdapter *a;
	s->userData = 0;
	s->errorMessage = 0;
	memset(s->outputs, 0, LOOP_SLOW_OUT_N * sizeof(double));
	if (!s->parameters || s->parameters[0] != LOOP_ABI_VERSION ||
		s->parameters[1] != LOOP_SLOW_TS) {
		s->errorMessage = "llc_slow requires [1, 5e-3]";
		return;
	}
	cfg.fast_ts = LOOP_FAST_TS;
	cfg.slow_ts = LOOP_SLOW_TS;
	cfg.abi_version = LOOP_ABI_VERSION;
	a = (SlowAdapter *)calloc(1, sizeof(*a));
	if (a) {
		a->core = llcSlowCreate(&cfg);
		a->last_time = s->time;
		a->held[SLOW_OUT_MODE_CMD] = LOOP_MODE_HOLD;
		if (s->inputs) {
			a->held[SLOW_OUT_ENABLE] = s->inputs[SLOW_IN_CHARGE_ENABLE];
			a->held[SLOW_OUT_HANDSHAKE] = s->inputs[SLOW_IN_HANDSHAKE_REQ];
			a->held[SLOW_OUT_CV_MODE] = s->inputs[SLOW_IN_CV_REQ];
			a->held[SLOW_OUT_V_REF] = s->inputs[SLOW_IN_V_REQ];
			a->held[SLOW_OUT_I_MAX] = s->inputs[SLOW_IN_I_REQ];
		}
	}
	if (!a || !a->core) {
		if (a)
			free(a);
		s->errorMessage = "Slow context initialization failed";
		return;
	}
	s->userData = a;
	memcpy(s->outputs, a->held, sizeof(a->held));
}

DLLEXPORT void plecsOutput(struct SimulationState *s)
{
	SlowAdapter *a = (SlowAdapter *)s->userData;
	if (!a) {
		memset(s->outputs, 0, LOOP_SLOW_OUT_N * sizeof(double));
		return;
	}
	if (s->time > a->last_time) {
		llcSlowStep(a->core, s->inputs, a->held);
		a->last_time = s->time;
	}
	memcpy(s->outputs, a->held, sizeof(a->held));
}

DLLEXPORT void plecsTerminate(struct SimulationState *s)
{
	SlowAdapter *a = (SlowAdapter *)s->userData;
	if (a) {
		llcSlowDestroy(a->core);
		free(a);
	}
	s->userData = 0;
}
