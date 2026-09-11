#include "llc_timer.h"
#include <math.h>
#include <string.h>

/* Source: SHRTIMERdrive in real firmware. One count is 1/680 MHz.
 * TC1 drives the LOW switch; TC2 drives the HIGH switch. */
int llcBuildPrimaryImage(int pfm, float frequency_hz, float duty, LlcTimerImage *out)
{
    uint32_t pre, half, q, b, w;
    float raw;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if ((pfm != 0 && pfm != 1) || !isfinite(frequency_hz) ||
        !isfinite(duty) || frequency_hz <= 0 || duty < 0 || duty > .5f) return 0;
    raw = 680000000.0f / frequency_hz;
    /* Refuse overflow instead of reproducing undefined float-to-uint16 casts. */
    if (raw < 4 || raw >= 65536.0f) return 0;
    pre = ((uint32_t)raw) & ~3u;
    half = pre >> 1;
    if (pfm) {
        if (half <= 240) return 0;
        out->c[0] = 120; out->c[1] = half - 120;
        out->c[2] = half + 120; out->c[3] = pre - 120;
    } else {
        q = pre >> 2; b = q + half;
        w = (uint32_t)(duty * (float)half);
        out->c[0] = q - w; out->c[1] = q + w;
        out->c[2] = b - w; out->c[3] = b + w;
    }
    out->pre = pre;
    return 1;
}
