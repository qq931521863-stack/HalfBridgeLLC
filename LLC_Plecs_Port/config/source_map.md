# 原函数到移植模块

| 原固件来源 | 移植位置 | 说明 |
|---|---|---|
| mathR02.c，PiPwmCompute/Compensator_Pid/OneOrderForm/SecondOrderForm | core/llc_control.c | 原增量/位置PI语义；不额外乘Ts |
| user_sample.c采样、ConsoleFast SampleLpfHandle | core/llc_fast_task.c、llc_control.c | 输入物理V/A，保留原始值、快LPF和慢FIR |
| mathR02.c PowerCtrHandle及9态Handler | core/llc_control.c | 上下文传参；函数局部static全部实例化 |
| SoftStartHandle、SoftCurStart、SoftCurStartTwo | core/llc_control.c | 保留预充/继电器等待的阈值与计数 |
| ConCurPwmHandle、ConVoltHandle、ConVoltHandleTwo | core/llc_control.c | PWM CC/30V CV/PMS CV |
| ConPwmHoldHandle、ConTransHandle | core/llc_control.c | 保留Hold累计40次与Transition400拍；PFM入口拒绝 |
| ConsoleSlow.c SHRTIMERdrive | core/llc_timer_image.c、llc_fast_task.c timer_image | 比较值计算与硬件寄存器写入解耦；PWM SR原非对称TD窗口保留 |
| ConsoleFast.c HandleFast、SwOCP | core/llc_fast_task.c | 原快保护、OvFault及3.5s恢复；无HAL |
| operateStatus.c get_CAN_States/runMainStateMachine | supervisor/llc_slow_task.c normalize/application | 有效协议请求接口；非完整CAN传输层 |
| ConsoleSlow.c StateM、ConVoWakeup、ChargeOn等 | supervisor/llc_slow_task.c supervisor/charging | 改为一次性命令事务 |
| HAL_IncTick、OTP_Protection有效RelayOld部分 | supervisor/llc_slow_task.c | 1/5/100ms相位；RelayOld400次5ms |
| ProtectionLLC.c AuxProtect/RelayProtect | supervisor/llc_slow_task.c protect | 20次违规的时间窗；保留锁存 |
| ProtectionLLC.c SysReset | supervisor/llc_slow_task.c sys_reset | 重现慢初始化时序；跨DLL快故障清除保守限制 |
| main.c HRTIM启动 | slow reset timer_running=1 | 定时器启动早于业务初始化；不把约1s业务等待误当计数器启动 |
| 真实寄存器边沿与影子寄存器行为 | model/gate/llc_gate_events.c | 680MHz计数、周期边界采纳；属于明确的仿真策略，未声称逐寄存器周期等价 |
| DLL ABI | vendor/plecs/DllHeader.h、adapters | 采用本机PLECS4.9头，pack4；各DLL各实例独立分配状态 |

缺失：`mComputer/mathTimeHandle/Sr_compute`只有ARM库；PFM闭环和PFM SR不可恢复。没有猜测增益、查表或SR补偿。PWM SR比较值已迁移，电气驱动极性仍需单独确认。

源状态修复：原Fast在PFC掉电时清OldState。拆分后Slow通过PFC输入及已运行状态收到NoSelect快照重新启动；覆盖快环已检测、慢采样未直接检测的短PFC脉冲。

行为边界：原SysReset会清全局FaultSta。分DLL后Slow仅清自己拥有的故障，不自动清Fast锁存；手动Fast bit9申请还受VacRmsFir<30及无外部故障限制。完整自动故障恢复需后续扩展显式清除许可/确认合同。FullCharged处理分支保留，但当前有效请求接口及原活动切换逻辑没有可用的自动满电入口。

DLL相关官方依据：本机PLECS4.9 `include/plecs/DllHeader.h` 与安装包C-Script例程；可参阅[Plexim C-Script说明](https://www.plexim.com/content/using-c-script-block)。
