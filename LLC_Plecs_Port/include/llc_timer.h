#ifndef LLC_TIMER_H
#define LLC_TIMER_H
#include <stdint.h>
typedef struct {
    uint32_t pre, c[4], d[4];
    uint8_t drv_h, drv_l, sr_enable;
} LlcTimerImage;
int llcBuildPrimaryImage(int pfm, float frequency_hz, float duty, LlcTimerImage *out);
#endif
