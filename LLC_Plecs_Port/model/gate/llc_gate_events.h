#ifndef LLC_GATE_EVENTS_H
#define LLC_GATE_EVENTS_H
/* Counts are integers represented exactly in doubles (PLECS signal ABI). */
typedef struct {
    double active[9], pending[9], epoch, sequence;
    int running, pending_valid, mask[4], reset_old;
} LlcGate;
void llcGateReset(LlcGate *g);
int llcGateEvaluate(const LlcGate *committed, double time, int capture,
                    const double in[15], LlcGate *candidate,
                    double out[4], double *next_time);
#endif
