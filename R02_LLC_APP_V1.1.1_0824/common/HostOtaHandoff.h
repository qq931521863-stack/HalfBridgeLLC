/******************************************************************************
    HostOtaHandoff.h — APPA2→APPA1 软跳转标记（固定 RAM 地址，两镜像 BSS 布局无关）
******************************************************************************/
#ifndef _HostOtaHandoff_H_
#define _HostOtaHandoff_H_

#include "stdint.h"

#define HOST_OTA_A2_TO_A1_MAGIC        ((uint32_t)0xA2A1F1A0U)
/* STM32G474 128KB SRAM 末字，仅 handoff 使用 */
#define HOST_OTA_HANDOFF_RAM_ADDR      ((volatile uint32_t *)0x2001FFFCLU)

void HostOtaHandoff_MarkA2ToA1(void);
uint8_t HostOtaHandoff_ConsumeA2ToA1(void);

#endif /* _HostOtaHandoff_H_ */
