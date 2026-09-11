#include "DllHeader.h"
#include "llc_api.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
	LlcContext *core;
	double last_time;
	double held[LOOP_FAST_OUT_N];
} FastAdapter;

DLLEXPORT void plecsSetSizes(struct SimulationSizes *s)
{
	s->numInputs = LOOP_FAST_IN_N;
	s->numOutputs = LOOP_FAST_OUT_N;
	s->numStates = 0;
	s->numParameters = LOOP_FAST_PARAM_N;
}

DLLEXPORT void plecsStart(struct SimulationState *s)
{
	LlcConfig cfg;
	FastAdapter *a;
	s->errorMessage = 0;
	s->userData = 0;
	memset(s->outputs, 0, LOOP_FAST_OUT_N * sizeof(double));
	if (!s->parameters || s->parameters[0] != LOOP_ABI_VERSION ||
		s->parameters[1] != LOOP_FAST_TS) {
		s->errorMessage = "llc_fast requires [1, 25e-6]";
		return;
	}
	cfg.fast_ts = LOOP_FAST_TS;
	cfg.slow_ts = LOOP_SLOW_TS;
	cfg.abi_version = LOOP_ABI_VERSION;
	a = (FastAdapter *)calloc(1, sizeof(*a));
	if (a) {
		a->core = llcCreate(&cfg);
		a->last_time = s->time;
	}
	if (!a || !a->core) {
		if (a)
			free(a);
		s->errorMessage = "Fast context initialization failed";
		return;
	}
	s->userData = a;
}

DLLEXPORT void plecsOutput(struct SimulationState *s)
{
	FastAdapter *a = (FastAdapter *)s->userData;
	if (!a) {
		memset(s->outputs, 0, LOOP_FAST_OUT_N * sizeof(double));
		return;
	}
	if (s->time > a->last_time) {
		llcFastStep(a->core, s->inputs, a->held);
		a->last_time = s->time;
	}
	memcpy(s->outputs, a->held, sizeof(a->held));
}

DLLEXPORT void plecsTerminate(struct SimulationState *s)
{
	FastAdapter *a = (FastAdapter *)s->userData;
	if (a) {
		llcDestroy(a->core);
		free(a);
	}
	s->userData = 0;
}
