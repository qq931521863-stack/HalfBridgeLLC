# DLL 接口与接线（ABI 1 / PWM 开发配置）

参数统一 `[1,25e-6,1e-3,680e6,1]`。第五项 profile=1；profile=0 缺少原 ARM 库算法，启动即报错。端口表索引均从0开始；PLECS Selector 索引从1开始。DLL 的输入/输出向量分别使用元件端子1/2。

| DLL | 输入宽度 | 输出宽度 | 调用周期 | 输出延迟 |
|---|---:|---:|---|---|
| LLC_Fast | 22 | 28 | 25 μs | 0 |
| LLC_Slow | 20 | 17 | 1 ms | 1 ns |
| LLC_Combined | 23 | 45 | 25 μs | 0 |
| LLC_Probe | 22 | 28 | 25 μs | 0；参数仅 `[1,25e-6]` |

## Fast 输入

| 索引 | 信号 |
|---|---|
| 0–2 | 继电器前电压 V_RLY、电池端电压 V_BAT、电池电流 I_BAT，单位 V/V/A |
| 3–6 | PFC许可、硬故障位图、交流RMS FIR值、OTP等级0/1/2 |
| 7–9 | command_seq、mode_request（-1保持，0–8模式）、init_flags |
| 10–13 | 电流请求A、电压请求V、归一化握手0/1/2、CC/CV状态0/1/2 |
| 14–17 | 业务run_state、RelayOld、慢故障位图、reset上升沿 |
| 18–21 | GPIO动作mask、relay_cmd、discharge_cmd、llc_enable_cmd |

## Fast 输出

| 索引 | 信号 |
|---|---|
| 0 | 定时器周期pre（680 MHz计数） |
| 1–4 | TC比较值c1/c2/c3/c4 |
| 5–8 | TD比较值d1/d2/d3/d4 |
| 9–11 | DrvH、DrvL、SynDrv许可 |
| 12–16 | LLC使能、继电器、放电、控制模式、preOK |
| 17–22 | Iref、Ilimit、频率Hz、Duty参数、Vbat_FIR、Ibat_FIR |
| 23–27 | fast_fault_bits、OvFault、command_ack、fast_tick、diagnostics |

诊断位：bit0缺PFM、bit1非法输入、bit2非法定时器值，三者锁存并关门极/LLC/继电器、打开泄放；bit3清故障请求被拒绝，不单独封锁门极。reset可清开发诊断。故障位保持原FaultSta位布局。

模式：0 NoSelect；1 SoftStar；2 BmsStar；3 SoftCurSt；4 ConVolt；5 ConCurPWM；6 Transition；7 OvLoadPFM；8 PwmHold。进入7的当拍就关闭输出并报告缺PFM，不输出替代控制律。

## Slow 输入/输出

| 输入索引 | 信号 |
|---|---|
| 0–2 | V_RLY / V_BAT / I_BAT |
| 3–9 | 上一快拍 Vbat_FIR、Ibat_FIR、relay_applied、mode、preOK、OvFault、fast_fault_bits |
| 10–12 | PFC许可、交流RMS、辅助12V电压 |
| 13–16 | 原始握手0–3、online0–3、充电使能、CV请求0CC/1CV |
| 17–19 | 电压请求V、电流请求A、reset |

| 输出索引 | 信号 |
|---|---|
| 0–2 | command_seq、mode_request、init_flags |
| 3–9 | Ireq、Vreq、握手、CC/CV、run_state、RelayOld、slow_fault_bits |
| 10–13 | GPIO mask、relay_cmd、discharge_cmd、llc_enable_cmd |
| 14–16 | power_on_ms（饱和2000）、slow_tick、timer_running |

握手3映射成2；online3禁止使能。标准V/A输入不得再乘0.1。

## 命令动作

seq=0表示无事件；第一条为1。相同seq只消费一次动作，I/V请求持续刷新。init_flags的bit0–10分别为：Iref清0、preOK清0、litate清0、Hold计数/阶段清0、清电压PI、清电流PI、PowerIniPidVar、仅电流PI oldout清0、仅电压PI oldout置0.01、申请清故障、清DriverPwm。GPIO mask bit0/1/2对应继电器/放电/LLC。

## Combined 与门极

Combined输入前20项采用Slow表，3–9由包装内部填充；in20=VacRmsFir，in21=硬故障位图，in22=OTP。输出前28项为Fast，28–44为Slow。

Gate输入15项：0–11=Fast.out0–11，12=fast_tick，13=timer_running且有效定时器帧，14=Scenario.reset。输出TC1→FETD21低管，TC2→FETD22高管。TD1/TD2仅记录；模型中的FETD23/24保持0，暂用体二极管整流。

快反馈通过Fast_snapshot_z1的28维一拍延迟进入Slow。t=0只初始化；t=1ms慢环读取975μs快照；其输出在1ms+1ns发布，1.025ms快环首次采纳。Gate比较值在周期边界生效，GPIO许可即时生效。硬故障在模型Hardware_fault_latch锁存，经Immediate_gate_inhibit直接关门极，随后Fast采纳故障。

## 电气连接

`C15正端 → R_RELAY_ON(1mΩ) → K_RELAY → R16上端 → 原电池等效`。
`C15正端 → R_DIS(100Ω) → K_DIS → Am7端子2公共负端`。
V_RLY使用Vm15；V_BAT新增V_BAT表；I_BAT使用原Am7。R24/C16原滤波保留。继电器和放电开关使用PLECS单刀开关`Switch`，初始断开。

R_DIS/继电器电阻为仿真初值，尚未按硬件标定。原380V母线、32V电池和谐振腔参数保留。
