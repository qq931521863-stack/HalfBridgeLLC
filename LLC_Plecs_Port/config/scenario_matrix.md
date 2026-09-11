# 验证场景与后续验收

| 场景 | 当前证据 | 尚待验证 |
|---|---|---|
| DLL ABI、t=0、重复调用、重启、双实例 | test_dll.py 加载实际DLL | 多架构/其他PLECS版本 |
| PWM CC、30V唤醒CV、PMS CV | test_core.c 原函数断言 | 后两种模式完整电路稳态与负载阶跃 |
| BMS预充→继电器→PWM CC | 自动Combined/Split模型；结果见startup_summary.json | 请求4A完全收敛、参数变化与容差 |
| SoftStar、PMS CV预充 | test_core.c 状态与等待计数边界 | 无电池实际电路、PMS外部负载 |
| PWM→Hold→Transition→PFM入口 | test_core.c：40次、400拍及同拍封锁 | 真实PFM闭环；依赖缺失库源码 |
| 原PWM/PFM比较窗口 | test_timer、test_gate；实际PLECS边沿 | MCU影子寄存器更新事件逐项对照 |
| PFC恢复 | test_slow.c，含快环捕获而慢环未采到的脉冲 | 电路动态PFC掉电/恢复 |
| 硬故障、OV/OCP/短路、OvFault恢复 | 核心断言、实际DLL故障回放；模型有直接关波路径 | 真实电路故障注入波形、完整自动SysReset |
| 辅源/继电器慢保护 | test_slow.c 时间窗/锁存/RelayOld计时 | 电气故障模型 |
| 快慢DLL一致性 | 2s、80000快拍，45输出逐拍相等 | 各业务组合、长期计数回绕 |
| 同步整流 | PWM比较值已迁移；门极暂置0 | 绕组极性、导通窗口、反向电流及开关损耗 |
| 1600W | 无额定验收证据 | 实际参数、PFM/SR、热限制及功率/效率工况 |

## 接下来按此顺序做

1. 固定待验收固件与模型版本。原目录仍有版本变化，先参考results/source_audit.json；当前工程使用model/baseline冻结模型，不自动覆盖为移动后的原模型。
2. 在32V电池场景延长到电流稳定，记录Iref/Ibat误差和波动；再做电流请求阶跃、PFC变化及故障注入。先完成PWM范围。
3. 获取`mComputer`、`mathTimeHandle`、`Sr_compute`的C实现、系数表和单位说明。按原函数输入/输出增加独立测试，接入PFM计算，再解除缺失算法诊断；不能仅去掉封锁。
4. 单独验证SR门极极性与绕组电压，再把Gate.TD1/TD2接到FETD23/24，验证死区及反向电流。
5. 扩展快慢接口的清故障许可与确认，补齐SysReset、温度源、满电入口和CAN回放。
6. 最后验收PWM/PFM切换、输出电压/负载范围及1600W。PFM→PWM回切、Burst、恒功率外环属于另加控制策略，原逻辑没有的行为应单列设计。
