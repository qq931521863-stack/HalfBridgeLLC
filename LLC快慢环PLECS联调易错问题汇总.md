# LLC 快慢环 PLECS 联调易错问题汇总

面向数字电源软件：用原固件（`PLECS_DLL` 宏隔开寄存器/GPIO）编成快环 `pi_controller.dll`、慢环 `llc_slow.dll` 做联调时，下面这些问题最容易把现象判错。算法在 `mathR02.c` / `StateM` / `ChargeOn`，不要为仿真去改状态机。

试验模型：`Visual studio projects/pi_controller/x64/Debug/LLC1600_PWM - DLL - test - Sr - Slow loop.plecs`

---

## 1. 先分清三根 CtrMode，不要看错线

模型里至少有三路都叫 CtrMode / CtrMode1，**不是同一个量**。

| 信号 | 位置 | 正常波形 | 作用 |
|---|---|---|---|
| 慢环命令 | 慢环 DLL 输出端子 2 → 快环输入端子 7 | 约 0.28 s **一根 5 ms 的 2**，然后一直 **-1** | 只在模式变化时通知快环 |
| 快环状态 | 快环 DLL 输出 → Goto `CtrMode` | 0 → 2 后**保持 2**，预充完再变 5 | 快环内部锁存的模式 |
| Delay 观测 | `CtrMode` → Transport Delay 5 ms → 标签 `CtrMode1` | 比快环状态晚 5 ms，数值相等 | 仅示波器 / 慢环观测口，慢环 DLL 里 `(void)` 掉了 |

慢环抽壳只在内部 `CtrMode` 发生变化时向外送该模式，否则送 **-1**。快环规则：

```c
if (IN_CTRMODE >= 0.0)
    Ctrl_interFace.CtrMode = (CtrMode_st)(int)IN_CTRMODE;
/* 输入为 -1 则保持，不改模式 */
```

因此：

- 看 Delay 后的 `CtrMode1` 和快环 `CtrMode` 相等，只能说明快环停在某个模式，**不能**说明慢环还在持续发 2。
- 要确认慢环有没有发出沿，必须看 **慢环子系统输出端子 2**（接到快环输入的那根）。

**Delay 后的 `CtrMode1` 不要接到快环输入。** 快环输入只接慢环命令。若把 Delay(快环输出) 接回快环输入，0.28 s 后输入会一直是 2，模式被钉死，预充更出不去。

---

## 2. 两个 DLL 不共享内存

快环、慢环是两份进程内的 `Ctrl_interFace` / `ADSample_Info` / `gCAN_DATA`。实机同一片 RAM 能互通的量，仿真里**只能靠 PLECS 连线**。

常见误接：

| 量 | 错法 | 后果 |
|---|---|---|
| `CurrentMax` | 把慢环输出接到快环 | 慢环 `StateM`/`ChargeOn` **不写** `CurrentMax`，输出一直是 0；快环每拍用输入覆盖，恒流给定变 0，管关掉 |
| `Volt_Ref` | ChargeOn 成功前就接慢环输出 | 成功前是 0，快环给定被写成 0 |
| `preOK` / `litate` / `RelaySta` | 以为慢环 `RelaySta` 能驱动快环预充 | `RelayOn()` 发生在快环 `SoftCurStart` 内部，慢环 `ChargeOn` 里是 `RelayOff()`；试验电路也没有继电器开关 |
| `IN_CTRMODE_OBS` | 以为 Delay 后的模式会改变慢环决策 | `main_slow.c` 里未使用 |

快环 `CurrentMax` 试验阶段用 Constant **4 A**。慢环 CAN 电流在抽壳里也是写死 4 A，但那份 CAN 到不了快环。

---

## 3. `ChargeOn` 进不了模式 2

慢环 5 ms 调 `ChargeOn()`。进 `BmsStar`（2）必须连续满足约 40 拍（约 200 ms）：

- `ChrgEna = 1`，`Handshake = 1`（`xp_CVmode = 1` 走恒流预充）
- `Vreq ≥ 30`（慢环端子 9，物理伏特，不是 0.1 单位）
- `29 ≤ Vbat < 60`
- `Ibat < 1 A`
- **`Vrelay < Vbat − 2`**
- `Delay1ms.delay_ini ≥ 100`（慢环 `plecsStart` 里已置 100）

容易踩的点：

1. **`Vreq` 接成 0**  
   标签 `CAN` 的 Constant 写成 0，电压条件永远不成立，快环一直停在 0。应接 **54**。

2. **慢环 `vOut_Rly` 和 `vOut_Bat` 接成同一个 `Vout`**  
   `Vrelay < Vbat − 2` 不成立。慢环 `vOut_Rly` 试验里接 Constant **0**（只给 ChargeOn 用），`vOut_Bat` 接真实 `Vout`。

3. **电容初值 0 + 32 V 电池经 R 充电**  
   前几十毫秒 `Vbat < 29` 或 `Iout > 1 A`，计数被清零。要等电容充到 ≥29 V 后再计 200 ms，最早大约 **0.28 s** 才出 2。仿真只跑 0.1 s 会误判「慢环没工作」。

4. **From 标签写错 / 没有对应 Goto**  
   PLECS From 无匹配时输出 0。`HandShake` 对不上 `xp_HandShake`、`CAN` 没有 Goto，都会让 ChargeOn 静默失败。

5. **把空闲时的 32 V 当成 LLC 升压**  
   `CtrMode = 0` 时 `ConIdleHandle` 关波。输出电容被 32 V 源经 R16 充到 32 V，和预充无关。

6. **慢环 DLL 参数**  
   `SampleTime = 5e-3`，`OutputDelay = 1e-9`（打破代数环）。`Filename` 与快环一样不要后缀：`llc_slow`。1 ms 采样会把 40 拍变成 40 ms，和实机 200 ms 不一致。

---

## 4. 进了模式 2 却出不去：预充电压是同一点

快环 `BmsStar` → `SoftCurStart()` 三段：

| 阶段 | 条件 |
|---|---|
| 抬压 `litate = 0` | `vOut_Rly_LPF ≥ vOut_Bat + 2` |
| 保持 `litate = 1` | 约 20 拍内不要掉到 `vBat + 0.5` 以下 |
| 合继电器 `preOK = 1` | `vOut_Rly < vOut_Bat + 0.2` 后再 `RelayOn`，再等 0.5 s（20000 拍 / 40 kHz）且 `vBat < 40` → 模式 5 |

实机：`vOut_Rly` = 继电器前电容，`vOut_Bat` = 电池侧。试验模型没有继电器，`Vout` 与 32 V 源之间只有电阻，**电流≈0 时两点电压相同**。

若快环两路都接 `Vout`：

- `Vout ≥ Vout + 2` 永远不成立 → 卡在模式 2
- 占空比只有 0.03～0.05，几乎灌不进电流，电压钉在 32 V

**不能靠功率级「拉开 2 V」。** 改的是进快环 DLL 的采样值。

### 4.1 不要一直 `Vbat = Vout − 2`

固定减 2：第一段立刻满足，第三段 `Vout < Vout − 1.8` 永远不成立，仍然卡在 2。

正确是**分段偏置**（只改采样，不进功率线）：

```text
Pulse = (CtrMode == 2) 持续 50 ms 的高电平
Vpre  = Vout - 3 * Pulse
```

- 进 2 后 50 ms：`vBat` 比 `vRly` 低 3 V → 抬压/保持能过
- 50 ms 后撤偏置：`vRly ≈ vBat` → 第三段能过
- 再 0.5 s → 模式 5

造 Pulse 不要用 **Hit Crossing + Turn-on Delay**：CtrMode 是 0→2 台阶，Hit Crossing 只出一个尖脉冲，Delay 不会变成 50 ms 高电平。用：

```text
A = (CtrMode == 2)           % Relational，下口 Constant 2
B = Delay(A, 50e-3)          % Transport Delay，不要用 Turn-on Delay
Pulse = A AND (NOT B)
```

### 4.2 加法器极性

`Vpre = Vout + (−3)×Pulse`，Sum 的 **Inputs 必须是 `++`**。

若图标是 `+-` 且 `Vout` 进了减端：`Vpre = −Vout`，示波器上一正一负。此时 `32 ≥ −32 + 2` 立刻「抬压完成」，第三段 `32 < −32` 失败，Duty 只亮一针，模式仍停在 2。

---

## 5. 停在模式 5 不是故障

`ConCurrHandle` 只有 `vOut_Bat_adc ≥ 40` 才 `RequestMode(PwmHold)`（8），再 8→6→7。

预充偏置在 `CtrMode ≠ 2` 后为 0，`Vbat = Vout`。若输出钉在 32 V，就不会切 PFM。

模式 ≥5 时快环 `vOut_Bat` 必须跟**电容电压 `Vout`**（可高于 32）。用 Signal Switch（与 PWM 里 DrvH 那个同类）：

| Switch 端子 | 接法 |
|---|---|
| 2（控制为真） | `Vout` |
| 3（控制为假） | `Vpre` |
| 4（控制） | `CtrMode >= 5` |
| 1（输出） | Goto `Vbat` → 快环端子 11 |

预充 Sum 的 Goto 改名为 `Vpre`，避免和 Switch 出口抢 `Vbat`。快环 `vOut_Rly` **始终接 `Vout`**，不要走 Switch。

对调端子 2/3 的现象：进入模式 5 后 `Vbat` 仍比 `Vout` 低 3 V。

---

## 6. 负载 R16 决定多大电流碰到 40 V

电路：`Vout — R16 — V_dc3(32 V)`，故 `Vout ≈ 32 + Iout × R16`。

切 PFM 电流：`I_trig = (40 − 32) / R16 = 8 / R16`。电流给定按 4 A。

| R16 | 4 A 时 Vout | 到 40 V 的电流 | 适用 |
|---|---|---|---|
| 2.2 Ω | 40.8 V | ≈ 3.6 A | 推荐，在 4 A 附近切 |
| 2.0 Ω | 40 V | 4.0 A | 刚好顶到门槛 |
| 5.5 Ω | 54 V | ≈ 1.45 A | 会提前切走 |
| 18 Ω（`54/3`） | 104 V | **0.44 A** | 模式 5 一有电流就切走 |

Duty 已到 0.4、频率 80 kHz、电压仍 32 V → **Iout ≈ 0**，改采样和 R16 都不会变模式。先看 Iout / DrvH，对照以前强制模式 5 能升压的试验。

电流环斜坡约 `0.0002 × 40 kHz = 8 A/s`，3.6 A 大约 0.45 s。模式 5 若在 0.83 s 进入，40 V 大约 1.3 s。`TimeSpan` 建议 **≥ 3 s**。

---

## 7. 抽壳与宏（不要污染 Keil）

- 算法文件（`mathR02.c`、`Sr_Handle.c`）不要加 `PLECS_DLL`。
- 写 HRTIM / GPIO / ADC 寄存器的代码放在 `#ifndef PLECS_DLL`；`#else` 只填仿真用的 `PlecsSrWin`、空宏。
- 快环 40 kHz：`ADC0_Sample` + `HandleFast` → `PowerCtrHandle` → `SHRTIMERdrive`。慢环 5 ms：只 `StateM()`，不跑 PI。
- 保护（`SwOCP` 等）在 `PLECS_DLL` 下跳过，仿真不会走软件过压把模式打回 0。
- 单位：模型 ↔ DLL 用 V / A / 占空比 0～1 / `Plv`=Hz。CAN 0.1 单位只在 DLL 内部 `×10`。

---

## 8. 同步整流与代数环

- SR 窗口只由快环 DLL 按 Timer D 比较值归一化（0～1）输出，和原边共用 `Saw`。不要用 `SynDrv AND DutyH/L` 近似。
- 原边同样只比 Timer C 四路沿（`PwmOn1/Off1`→下管，`PwmOn2/Off2`→上管），再 AND `DrvL`/`DrvH`。`DT` 只出锯齿，不要互补切窗，也不要用 Turn-on Delay+NOT 给 PFM 单独做一套再 Switch。
- 快环 DLL 现为 **8 入 17 出**。Demux Width 必须改成 17，否则后几路是旧值或 0。
- 慢环不管门极、不管 SR。
- 快↔慢有环时：慢环 `OutputDelay = 1e-9`；快环 `CtrMode` 进慢环要 Delay 一拍。快环 `OutputDelay` 可保持 0。

PWM 同步整流要 `Iout > 4 A` 连续一段时间才 `SynDrv = 1`。模式 5 电流还没到 4 A 时 SynDrv=0 是正常的。

---

## 9. 示波器怎么接才不会误判

建议同一张 Scope：

1. 慢环输出端子 2（命令，应看到针）
2. 快环 `CtrMode`（台阶：0 → 2 → 5 → 8/6/7）
3. `Vout`、`Vpre`/`Vbat`（确认 50 ms 窗口和 Switch）
4. `Duty`、`Plv`、`Iout`、`DrvH`

判据：

| 现象 | 优先查 |
|---|---|
| 全程 CtrMode=0，电压到 32 V | ChargeOn 条件、Vreq、慢环 vOut_Rly、仿真是否太短 |
| 快环=2，命令已是 2 再 -1 | 预充采样：两路是否同一 `Vout`；偏置 Pulse/Sum |
| Vbat 与 Vout 反号 | Sum 减成了 `−Vout` |
| 一直 2，Duty 只有一针 | 偏置窗口没维持，或 Vbat 为负导致「假抬压」 |
| 一直 5，Vout≈32，Duty=0.4 | Iout 是否为 0；`vOut_Bat` 是否已切到 `Vout`；R16 |
| Delay 的 CtrMode1 一直等于快环 | 正常，不是慢环命令 |

---

## 10. 推荐最小联调顺序

1. 不接慢环，快环 `CtrMode` 手拨 5，确认功率级、SR、PI。
2. 接慢环命令到快环输入，确认 0.28 s 命令针和快环变 2。
3. 加上 50 ms `Vout−3` 偏置，确认约 0.83 s 变 5。
4. Switch：模式 ≥5 时 `vOut_Bat=Vout`，R16=2.2 Ω，看 Iout 爬升后 5→8→6→7。

每一步只改一类接线。不要用 Delay 观测线代替命令线，也不要在功率回路里串电容/变压器去「造」2 V。
