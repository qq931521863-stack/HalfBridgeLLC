# 快环 / 慢环 DLL 移植方案（现有工程原样进 DLL）

原则：**用 `R02_LLC_APP_V1.1.1_0824/AppUser` 现有 `.c/.h`，尽量原样编进两份 DLL**。只加 PLECS 进出线和硬件空宏。不重写 PI，不另起 `LlcContext`。

Keil 工程不动。VS 里拷贝源文件。

---

## 1. SR 能不能进 DLL（结论：可以）

固件里 SR **全部在快环**，慢环不管 SR。

| 模式 | 现有函数 | 源文件 | x64 能否原样编 |
|---|---|---|---|
| PWM SR 开通/关断 | `PwmSyncDrvUpdate()` | [`mathR02.c`](R02_LLC_APP_V1.1.1_0824/AppUser/mathR02.c) | 能。纯 C，电流门槛 |
| PFM SR 查表与窗口 | `Sr_compute()` / `mathTimeHandle()` | [`Sr_Handle.c`](R02_LLC_APP_V1.1.1_0824/AppUser/Sr_Handle.c) | 能。已是源码，Keil 已加入该文件，**不是** ARM `.lib` |
| PFM 增益插值 | `mComputer()` | 同上 | 能。和 SR 表在同一个 `.c` |
| 比较值写 Timer D | `SHRTIMERdrive()` 后半 | [`ConsoleSlow.c`](R02_LLC_APP_V1.1.1_0824/AppUser/ConsoleSlow.c) | 寄存器写入用 `#ifndef PLECS_DLL` 包掉。Duty 模型不吃比较值 |

PWM SR 门槛（冻结，勿改）：

- `SynDrv==0` 且 `iOut_Bat_adc > 4 A` 连续 **1000 拍（25 ms）** → `SynDrv=1`
- 已开通后 `iOut_Bat_adc < 2 A` → `SynDrv=0`
- 仅 `ConCurPWM` / `PwmHold` 会开 SR；其它 PWM 模式 `SHRTIMERdrive` 里把 `SynDrv` 清 0

PFM SR 门槛（冻结）：

- FIR > 6 A 开通，adc 或 FIR < 4 A 关断
- `Sr_Atime` / `Sr_Btime` 由查表 `* SR_DELAY_TIME(0.68)` 得到，单位是 HRTIM 计数

**Duty 模型怎么接 SR（可行边界）：**

- 第一阶段（PWM 恒流）：DLL 出 `DriverPwm.SynDrv`（0/1）。模型 `FETD23/24` 门极用 `SynDrv` 作为使能；没有脉冲时体二极管整流。你现在的快环子系统 **已经把 Demux[1] 接到 `SynDrv`**，Goto 标签也是 `SynDrv`。差的是：`FETD23/24` 的门极（端子 3）目前还没接到 `SynDrv`，`SR_DUTY` 只进了示波器。
- 第二阶段（仍用 Duty 发生器）：用同一套三角波，在 `SynDrv==1` 时生成与原边对称、带死区的 SR 脉冲。脉宽可用现成量近似：

```text
SR_Duty ≈ DriverPwm.Sr_Dtime / (0.5 * SHRTIMER_PLV / DriverPwm.Plv)
```

`Sr_Dtime` 已在 `SHRTIMERdrive` 的 PWM 分支里按 Duty 滤出来，**不必新算法**。

- PFM 精确 SR 窗口（`Sr_Atime/Sr_Btime` 非对称）Duty 发生器做不到逐拍等价。算法仍进快环 DLL；模型侧 PFM 阶段先继续用 `SynDrv` 开关。要窗口级波形再另做比较值门极，本方案不强制。

`mathR02.h` 里的 `A_SR_Control()` 只有声明、工程里无定义，DLL **不要链接它**。用 `Sr_compute` / `mathTimeHandle` 即可。

---

## 2. 快 / 慢各跑什么（按固件中断，不按“电压环=慢环”）

```
TIM3 @ 40 kHz → interrupt_ADC1
  ADC0_Sample
  HandleFast
    SampleLpfHandle
    PowerCtrHandle          ← 电压 PI / 电流 PI / 软起 / PFM / PWM SR
    SHRTIMERdrive           ← 仿真不写寄存器，只保留 Duty/Plv/SynDrv 计算

SysTick 每 5 ms
  StateM / ChargeOn / ConVoWakeup   ← 只改 CtrMode、Volt_Ref、给定
```

电压环 PI 在快环。慢环 DLL 不是电压环。

---

## 3. 工程怎么摆

现有空壳：[`Visual studio projects/pi_controller`](Visual studio projects/pi_controller)。模型快环子系统 Mux 已是 8、Demux 已是 6，端口已用固件变量名。

建议同一 sln 两个 DLL 工程，源文件从 `AppUser` **拷贝**：

```
llc_fast  （SampleTime = 1/Fs, Fs=40e3）
  DllHeader.h
  main_fast.c                 ← 只做 plecs 进出，约几十行
  stub_hal.h / HwConfig.h     ← 去掉 stm32 HAL，GPIO 宏变空
  mathR02.c / mathR02.h       ← 原样
  ConsoleFast.c               ← HandleFast 原样；保护建议 #if 0
  user_sample.c               ← 只用 ADC0_Sample 三路赋值 + FIR
  Sr_Handle.c                 ← 原样（PWM 不调它也要能链上；PFM 要调）
  ConsoleSlow.c 中的 SHRTIMERdrive / Relay* / Discharge* / LLC_*
                              ← 函数留下，寄存器写入 #ifndef

llc_slow  （SampleTime = 5e-3, OutputDelay=1e-9）
  main_slow.c
  ConsoleSlow.c 中 StateM / ChargeOn / ConVoWakeup / StandBy / ChargeFull / StateMInit
  需要的话 operateStatus.c 的 runMainStateMachine
```

不要编：`ProtectionLLC.c`、`CAN_Control_2800W.c`、`MODBUS_SLAVE.c`、OTA、LED、风扇、`ADC2_Sample`。

硬件垫片（不是改算法）：

```c
#define PLECS_DLL 1
#define DrvH_On()
#define DrvH_Off()
#define SarH_On()
#define SarH_Off()
#define SarL_On()
#define SarL_Off()
/* RelayOn 等若是函数：只改 DataFlowFace，不要写 GPIO */
```

两份 DLL **不能共享 C 全局变量**。PLECS 导线复制同一套名字。

---

## 4. 快环 DLL 端口（对齐你现在的模型）

你当前快环 Mux/Demux 已经是下面这张表，`plecsSetSizes` 必须改成 **8 入 / 6 出**（现在 `main.c` 还是 4/4，和模型不一致）。

### 输入 `inputs[]` → 写入现有变量

| idx | 模型端子名 | 写入 |
|---|---|---|
| 0 | `vOut_Rly_adc` | `ADSample_Info.vOut_Rly_adc` |
| 1 | `vOut_Bat_adc` | `ADSample_Info.vOut_Bat_adc` |
| 2 | `iOut_Bat_adc` | `ADSample_Info.iOut_Bat_adc` |
| 3 | `PFC_ok` | `DataFlowFace.PFC_ok` |
| 4 | `CtrMode1`（慢环来） | 见第 6 节：`>=0` 才写 `Ctrl_interFace.CtrMode` |
| 5 | `Volt_Ref` | `Ctrl_interFace.Volt_Ref` |
| 6 | `CurrentMax` | `Ctrl_interFace.CurrentMax` |
| 7 | `xp_HandShake` | `gSys_State.xp_HandShake` |

单位是 **V / A**，不要再乘 `COM_VOUT_BASE`。

进环仍走现有代码：

```c
ADSample_Info.vOut_Bat_FIR = ADSample_Info.vOut_Bat_FIR * 0.999f + ADSample_Info.vOut_Bat_adc * 0.001f;
ADSample_Info.iOut_Bat_FIR = ADSample_Info.iOut_Bat_FIR * 0.999f + ADSample_Info.iOut_Bat_adc * 0.001f;
SampleLpfHandle();
HandleFast();   /* 内含 PowerCtrHandle；PWM SR 在 PwmSyncDrvUpdate */
```

### 输出 `outputs[]` ← 现有变量

| idx | 模型端子名 | 读取 |
|---|---|---|
| 0 | `Duty` | `DriverPwm.Duty` |
| 1 | `SynDrv` | `DriverPwm.SynDrv`  ← **这就是 SR 使能** |
| 2 | `DrvH` | `DriverPwm.DrvH` |
| 3 | `DrvL` | `DriverPwm.DrvL` |
| 4 | `Plv` | `DriverPwm.Plv`（Hz，给三角波频率） |
| 5 | `CtrMode` | `Ctrl_interFace.CtrMode`（回慢环，必须经 Delay） |

可选（模型若要 SR 脉宽而不是只使能），Demux 再加一路：

| idx | 建议名 | 读取 |
|---|---|---|
| 6 | `Sr_Dtime` 或 `SR_Duty` | PWM：`Sr_Dtime / half`；关 SR 时为 0 |

第一阶段不加第 7 路也能跑：门极只用 `SynDrv`。

`t=0`：`plecsStart` 调 `PowerIniPidVar()`；`plecsOutput` 在 `time==0` 直接 return。

现有 [`main.c`](Visual studio projects/pi_controller/pi_controller/main.c) 的宏是错的（`Vout` 重定义、和 Mux 顺序反了），按上表重写，不要沿用。

---

## 5. 慢环 DLL 端口

`SampleTime = 5e-3`（对齐原来 `delay_Slow>=5` 才 `StateM`）。每次 `plecsOutput` = 一次 5 ms 拍。

建议 **8 入 / 6 出**，`OutputDelay = 1e-9`。

### 输入

| idx | 来源 | 写入 |
|---|---|---|
| 0 | 同 `vOut_Rly_adc` | `ADSample_Info.vOut_Rly_adc` |
| 1 | 同 `vOut_Bat_adc` | `ADSample_Info.vOut_Bat_adc` |
| 2 | 同 `iOut_Bat_adc` | `ADSample_Info.iOut_Bat_adc` |
| 3 | 快环 `CtrMode` 经 Delay | 只作观测；不要每个 5 ms 覆盖快环内部子状态 |
| 4 | 快环 `preOK` 经 Delay | `Ctrl_interFace.preOK` |
| 5 | 充电使能常量 | `gSys_State.xp_ChrgEna` |
| 6 | 握手常量 | `gSys_State.xp_HandShake` |
| 7 | `Vreq`（V） | 见下 |

`ChargeOn()` **函数体不改**。入口把物理量写成它正在读的 0.1 单位字段：

```c
gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgReqVolt = (uint16_t)(Vreq * 10.f);
gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgReqCur  = (uint16_t)(Ireq * 10.f);
gSys_State.xp_CVmode = 1;   /* handshake==1 时 ChargeOn 会强制 CC */
```

内部仍是 `* 0.1f`，和台架一致。

### 输出 → 快环输入

| idx | 现有变量 | 接到快环 |
|---|---|---|
| 0 | `CtrMode` 请求 | Fast in4，无新沿时出 **-1** |
| 1 | `Ctrl_interFace.Volt_Ref` | Fast in5 |
| 2 | `CurrentMax`（A） | Fast in6 |
| 3 | `gSys_State.xp_HandShake` | Fast in7 |
| 4 | `DataFlowFace.RelaySta` | 模型无继电器则只看波形 |
| 5 | `DataFlowFace.DisCharge` | 同上 |

慢环 **不输出 SR**。

---

## 6. 模式所有权（两 DLL 唯一必须加的几行）

固件慢环会写 `Ctrl_interFace.CtrMode = BmsStar`，快环再迁到 `ConCurPWM`。  
若慢环每 5 ms 都赋 `BmsStar`，快环会卡在预充。

- 慢环只在 `ChargeOn` / `ConVoWakeup` **进入沿**改一次：`SoftStar` / `BmsStar` / `SoftCurSt` / `NoSelect`
- 给快环：有新请求出枚举，否则 **-1**
- 快环：`if (inputs[4] >= 0) Ctrl_interFace.CtrMode = (CtrMode_st)inputs[4];` 再调现有 `HandleFast()`
- 快→慢的 `CtrMode` 必须 Delay 一拍

---

## 7. `plecsOutput` 串现有函数（快环）

```c
void plecsOutput(struct SimulationState *s)
{
    if (s->time == 0) return;

    ADSample_Info.vOut_Rly_adc = (float)s->inputs[0];
    ADSample_Info.vOut_Bat_adc = (float)s->inputs[1];
    ADSample_Info.iOut_Bat_adc = (float)s->inputs[2];
    DataFlowFace.PFC_ok        = (uint8_t)s->inputs[3];
    if (s->inputs[4] >= 0)
        Ctrl_interFace.CtrMode = (CtrMode_st)s->inputs[4];
    Ctrl_interFace.Volt_Ref    = (float)s->inputs[5];
    Ctrl_interFace.CurrentMax  = (float)s->inputs[6];
    gSys_State.xp_HandShake    = (uint8_t)s->inputs[7];

    ADSample_Info.vOut_Bat_FIR = ADSample_Info.vOut_Bat_FIR * 0.999f
                               + ADSample_Info.vOut_Bat_adc * 0.001f;
    ADSample_Info.iOut_Bat_FIR = ADSample_Info.iOut_Bat_FIR * 0.999f
                               + ADSample_Info.iOut_Bat_adc * 0.001f;
    SampleLpfHandle();
    HandleFast();   /* PowerCtrHandle + PwmSyncDrvUpdate + SHRTIMERdrive 计算 */

    s->outputs[0] = DriverPwm.Duty;
    s->outputs[1] = DriverPwm.SynDrv;   /* SR 使能 */
    s->outputs[2] = DriverPwm.DrvH;
    s->outputs[3] = DriverPwm.DrvL;
    s->outputs[4] = DriverPwm.Plv;
    s->outputs[5] = Ctrl_interFace.CtrMode;
}
```

`HandleFast` 里建议 `#if 0` 掉 `SwOCP` / 62 V / 开路。`SHRTIMERdrive` 里 `hhrtim1.Instance->...` 整段 `#ifndef PLECS_DLL`；PWM 分支里算 `Sr_Dtime` / 清 `SynDrv` 的逻辑要保留。

PFM：`ConPfmHandle` 保持原样即可（里面已经 `mComputer` + `mathTimeHandle`）。不要再写一套频率环。

---

## 8. 模型侧 SR 接线（你自己改）

当前状态：

- 快环已出 `SynDrv`，Goto `SynDrv`
- `From7` 仍读过时标签 `SR_DUTY`，只进 Scope1
- `FETD21/22` 门极已接 `DutyH/DutyL`
- `FETD23/24` 只有功率端子，**门极未接**

建议：

1. `From SynDrv` → `FETD23` 端子 3、`FETD24` 端子 3（两管先共用使能，PWM 阶段够用）。
2. 示波器改看 `SynDrv`，去掉对 `SR_DUTY` 的依赖，或把标签改成 `SynDrv`。
3. 原边三角波频率改吃 `Plv`。暂时改不了就先钉 **80 kHz** 只验 `ConCurPWM`。
4. SR 直通门极没有死区，仿真可能直通。若波形异常：门极改为 `SynDrv AND (延迟后的原边互补)`，死区用现有 `dt=150e-9`。

---

## 9. 必须冻结

时间：快环 40 kHz，PI 不乘 Ts；慢环 5 ms 一拍（`ChargeOn` 的 `cont>=40` 仍是 200 ms）；`REF_RAMP_STEP=0.0002`。

单位：模型↔DLL 为 V/A；写入 `gCAN_DATA` 仍是 0.1 单位。

`Plv` = Hz：`ConVolt` 40 kHz、`ConCurPWM` 80 kHz、`SoftStart` 120 kHz、PFM 60–250 kHz。

占空比上限 **0.4**，不要用历史宏 `PWM_MAX_DUTY=0.15`。

`CtrMode_st` 枚举不许重排。PWM→PFM：`ConCurPWM → PwmHold → Transition → OvLoadPFM`。

FIR `0.999/0.001`；`FilterOne`；`PWM_V_KP/KI`、`PWM_CUR_KP/KI` 不动。

SR：`PwmSyncDrvUpdate` 4 A / 2 A / 1000 拍；PFM 表 `M_highPlv` / `M_lowPlv` / `SR_DELAY_TIME=0.68`；`SHRTIMER_PLV=680e6`、`LLC_DEADTIME=120`。

场景：`PFC_ok=1`，握手=1，54 V / 4 A，电池约 32 V。

本轮不碰：保护、CAN 协议栈、OTA、Keil、HRTIM 寄存器级门极。

---

## 10. 自己改的顺序

1. 拷 `mathR02.c/.h`、`Sr_Handle.c` 进 VS，HwConfig 去 HAL，先能编过 `PowerCtrHandle`。
2. `main_fast.c` 按第 7 节把 `plecsSetSizes` 改成 8/6，对齐现模型。慢环先用 Constant：`CtrMode` 第一拍 `BmsStar` 其后 `-1`，`CurrentMax=4`，`xp_HandShake=1`。目标：Duty 不再是 0.20，电流起来后 `SynDrv` 从 0 变 1。
3. `SynDrv` 接到 `FETD23/24` 门极。
4. `Plv` 接三角波，或先写死 80 kHz。
5. 再拷 `StateM/ChargeOn` 做慢环 DLL，加 Delay，按第 6 节做模式保持。
6. 对照：同一给定下 `CtrMode`、`Duty`、`Plv`、`SynDrv` 与固件逻辑一致，不要求先对上台架波形。

先前 `llc_loop` 那套抽上下文的工程不要继续扩。以本文件和现有 `AppUser` + 已改过端口的 PLECS 模型为准。
