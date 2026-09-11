# 慢环实施记录

`supervisor/llc_slow_task.c`实现显式上下文，按每1ms一次调用保留5ms业务/RelayOld相位和100ms初始化任务；输出带序号的一次性命令，不能通过共享全局变量影响Fast。

已运行test_slow：无握手唤醒、BMS CC、PMS CV与握手3归一化、online3禁止、双实例、RelayOld计数、辅源/继电器时间窗保护、复位、非法输入、PFC恢复。实际DLL组合/拆分对照覆盖80000快拍、45输出完全一致。

模型Slow输出延迟1ns，上一快拍快照由非直接馈通CScript保存；同一1ms时刻Combined输出的Slow trace与拆分版延迟输出不要求直接相等，Fast在下一25μs拍消费同一命令。

SysReset仅实现慢环拥有的初始化/故障清除范围，完整跨DLL自动快故障清除还缺许可与确认合同。自动满电入口、CAN传输和真实温度采样不在当前接口内。
