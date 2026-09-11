# 复核记录

1. Fast mode_request=-1保持当前模式、归一化握手只允许0–2、非法输入/PFM/定时器诊断关闭执行器：已修复并有回归断言。
2. PFC恢复不重新启动：原ConsoleFast清OldState的跨任务动作在拆分后丢失。C测试修复前失败；Slow通过PFC低电平和已运行状态的NoSelect快照重置启动历史后通过。覆盖短PFC脉冲被Fast捕获但未被Slow直接采样的情况。
3. Gate reset原为0常量：已接Scenario reset；Fast/Slow/Combined各包装t=0和复位保持实例隔离。
4. PLECS中文绝对DLL路径被错误解码：改为模型相对路径，真实PLECS加载通过。
5. PLECS文件中根输出需要Terminal登记；元件、连接、注释必须按顺序声明。生成器增加规范化步骤，解决了PLECS忽略后追加元件/连接造成的端子未连接错误。使用PLECS getModelTree读取实际解析树定位，而非关闭连接检查。
6. 继电器/泄放使用单刀开关Switch；内部BasicSwitch和双刀Switch2不适用于单输入合同。

所有源函数迁移保持原算法边界；PFM依赖缺失、PWM SR电气未验证、完整SysReset恢复缺显式许可等限制见README。

7. RPC可能在提前停止后返回无异常的部分数据。run_plecs.py现在要求结果达到请求结束时间，新增4项测试覆盖部分结果、完整结果、空结果和维度错误。一次拆分版2s请求仅返回1.095s，已明确列为不完整并重跑，不能沿用旧的PASS打印作为验收证据。
8. Gate增加复位上升沿、复位保持高电平和再次复位的断言；连续高电平不应反复重置开关相位。
9. 增加模型声明顺序、顶层输出Terminal登记、Switch类型及三端连接回归检查。
