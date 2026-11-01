#ifndef UPGRADE_FEATURE_CONFIG_H
#define UPGRADE_FEATURE_CONFIG_H

/*
 * OTA/IAP 升级功能总开关（LLC 主机 APP_A2）
 * 1 = 编译并启用 CAN OTA 网关、Handoff、PFC 安全请求等升级逻辑
 * 0 = 裁剪为桩函数，正常运行不受升级协议影响
 *
 * 也可在 Keil 预定义宏中覆盖：OTA_UPGRADE_ENABLE=0 或 OTA_UPGRADE_ENABLE=1
 */
#ifndef NO_BOOT 
#ifndef OTA_UPGRADE_ENABLE
#define OTA_UPGRADE_ENABLE  1
#endif
#else
#define OTA_UPGRADE_ENABLE  0
#endif
#endif /* UPGRADE_FEATURE_CONFIG_H */
