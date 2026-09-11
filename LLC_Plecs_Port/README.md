# LLC 固件 DLL 移植工程

本工程是可构建、可运行的 **PWM 开发版本**。所有移植产物均在本目录，模型生成只读取冻结副本。已生成 `LLC_Fast.dll`、`LLC_Slow.dll`、`LLC_Combined.dll` 和接口探针。

## 从这里开始

1. 用 Visual Studio 2022 打开 `LLC_Plecs_Port.sln`，选 **x64 / Debug** 或 **x64 / Release**，生成解决方案。也可在 PowerShell 运行 `./build.ps1`，它会构建并执行四个C测试程序。
2. 用 PLECS 4.9 x64 打开 `model/LLC1600_Combined_DLL_Port.plecs`，先看组合版的启动流程；再打开 `model/LLC1600_Split_DLL_Port.plecs` 检查快慢DLL接线。
3. DLL路径相对模型文件；必须一起保留 `bin/x64`、`model` 和 `model/gate`。不要把不同接口宽度的DLL文件名直接互换。
4. 自动测试可用 Python：`tests/test_dll.py`、`tests/test_multirate.py`。真实PLECS自动运行使用 `tools/run_plecs.py`；本机已确认RPC端口 **1180**，其他机器用 `--port` 指定。

首次自动运行可执行：`python tools/run_plecs.py LLC1600_Combined_DLL_Port.plecs --duration 2 --output-step .001 --max-step 1e-6`。拆分版替换模型名即可。这里显式使用1μs最大步长和1ms记录间隔；模型文件保留原100ns最大步长，直接点运行会更慢。门极边沿由事件调度，1ms记录仅用于启动状态观察，不能用来检查死区。

所有正式控制DLL参数为 `[1,25e-6,1e-3,680e6,1]`。Fast 22入28出；Slow 20入17出；Combined 23入45出。t=0只初始化，首个快拍在25μs。详细信号见 [端口及接线表](config/port_map.md)，原函数对应关系见 [源码映射](config/source_map.md)。

## 模型怎么用

| 模型 | 用途 |
|---|---|
| benches/01_DLL_IO.plecs | 11/22/33输入回读，计数与25μs回调核对 |
| benches/03_Gate_PWM.plecs | 固定80kHz PWM边沿；Duty参数0.1 |
| benches/03_Gate_PFM_Windows.plecs | 250kHz PFM比较窗口测试；不是PFM闭环 |
| benches/04_PWM_CC_Replay.plecs | 恒定测量输入的Fast控制回放 |
| benches/05_Startup_BMS.plecs | 手动发一次BmsStar启动命令，观察电路预充 |
| LLC1600_PWM_DLL_Port.plecs | 手动Fast命令直接接电路；用于局部调试 |
| LLC1600_Combined_DLL_Port.plecs | 自动业务、快环与慢环组合，默认32V电池/BMS CC |
| LLC1600_Split_DLL_Port.plecs | 相同业务拆为Fast/Slow DLL，显式快照延迟和命令传递 |

自动模型的 `Scenario_3_to_22` 常量向量表示 Combined 输入3–22；其中输入3–9是内部快照占位，保持0。常用量对应常量向量的0基索引：7=PFC、8=VacRms、9=辅助电压、10=握手、11=online、12=使能、13=CV请求、14=请求电压V、15=请求电流A、16=reset、17=VacRmsFir、18=硬故障位图、19=OTP等级。

默认请求54V/4A、握手1、online1、使能1。无握手唤醒需要把握手/online设0，并提供适当的无电池测试电路；不要将固定32V理想电池直接设0V当成“拔掉电池”。PMS CV选择握手2、online2、CV请求1。切换动态请求建议将常量替换成按同一20维合同输出的Scenario模块。

Fast主要观测：out15模式、16预充阶段、13继电器、17电流参考、19频率、20Duty、23故障、27诊断。电气模型另外导出4路门极和原模型未使用的测量量，便于检查方向和SR请求。

## 已实现的控制与边界

采样/FIR/LPF、增量及位置PI、PWM恒流、30V唤醒恒压、PMS恒压、SoftStar/BmsStar/SoftCurSt、Hold/Transition、快保护、慢业务状态、RelayOld、继电器/辅源保护均已移入实例上下文。硬件寄存器输出改为比较值，Gate按680MHz计数产生事件边沿。

PWM限流保持原10A。PFM的增益和SR时间计算依赖 `mComputer/mathTimeHandle/Sr_compute`，当前只有不能用于x64的ARM库。因此进入PFM当拍关闭门极、LLC和继电器，置诊断bit0；未虚构控制律或查表。profile=0目前拒绝启动。

PWM SR的比较值已迁移，但电路FETD23/24仍保持关闭，以体二极管整流。需要单独确认绕组和SR门极极性后才连接TD1/TD2。完整SysReset跨DLL清故障许可、自动满电入口、温度源、完整CAN回放及1600W工况验收尚未完成。R_DIS=100Ω、继电器电阻1mΩ是仿真初值。

## 测试与结果

`test_timer/test_gate/test_core/test_slow` 对真实C函数执行断言。`test_dll.py` 加载实际DLL检查ABI、t=0、1000快拍、重复回调、双实例及故障关闭。`test_multirate.py` 比较实际Combined/Fast/Slow DLL：2秒、80,000快拍、45路输出逐拍相等，结果在 `results/multirate_replay.csv`。

真实PLECS探针、PWM/PFM窗口与Fast回放已通过。门极边沿与计数公式相符，结果在 [波形图](results/plecs_gate_waveforms.png) 和 `results/plecs_gate_edges.csv`。电路仿真的输出和实际求解器覆盖参数分别保存在 `results/plecs_*.json` 与 `*.options.json`；最终验收状态见 [实施结果](results/implementation_status.md)。不把独立DLL回放等同于电路闭环验收。

重新生成工程/模型的脚本分别为 `tools/generate_projects.py`、`tools/generate_models.py`。模型生成只读取 `model/baseline` 下冻结的基线，不读取正在编辑的根目录模型。执行生成脚本会覆盖本工程生成模型，调试修改请另存副本。

## 版本与后续工作

原目录在实施期间有修改/移动，`mathR02.c`、`user_sample.c`、`HwConfig.h`与分析时哈希不同。当前模型也增加了另一套控制子系统。核对结果见 `results/source_audit.json`；本轮不自动合并这些变化。当前采样仍采用原文档记录的0.999/0.001滤波和直接V/A接口，冻结电路模型SHA256见 `results/model_baseline.json`。

详细场景、已验证范围及下一步顺序见 [验证矩阵](config/scenario_matrix.md)。交付二进制为本机验证的x64 Debug版本，需要Visual Studio调试运行库；跨机器发布前应构建并复测Release版本。
