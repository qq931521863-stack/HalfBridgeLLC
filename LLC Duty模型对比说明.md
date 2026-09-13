结论先说：**单位全部按“真实物理量 / 标幺”走，不是 0.1、也不是 0.01。**  
`Duty` 是 **0～1 的占空比**（常见 0.02～0.4），`Plv` 是 **赫兹 Hz**（PWM 恒流应看到约 **80000**），`SynDrv/DrvH/DrvL` 是 **0/1**。

**目前只拿试验模型搭。** 同步整流先不加。主模型仅作对照，不再作为当前验收对象。

对照文件：

- 当前在用：`Visual studio projects/pi_controller/x64/Debug/LLC1600_PWM - DLL - test.plecs`
- 主模型（先不管）：`.../LLC1600_PWM - DLL.plecs`
- 快环抽壳：`main.c`（8 入 6 出，原样写固件变量）

---

# 当前波形：模式 5 在跑什么、电压为什么钉在 32 V

试验模型 Constant：`CtrMode2=5`、`CurrentMax=4`、`Volt_Ref=54`、`xp_HandShake=1`。  
你现在看到的是：**Duty 从 0.02 慢慢往上（约到 0.055）**，输出电压一直 **32 V**。这和固件一致，**不是电压环没起来**。

## 模式 5 不是抬压，是 PWM 恒流

`CtrMode = 5` = `ConCurPWM`。`PowerCtrHandle` 里只进 **`ConCurrHandle()`**，不进 `ConVoltHandle()`（那是模式 4）。

| 量 | 模式 5 管不管 | 实际怎么变 |
|---|---|---|
| 输出电压 | **不管** | 被电池源 `V_dc3=32 V` 钉死，不会往 54 V 走 |
| `Volt_Ref=54` | **本模式不读** | 预充/恒压才用；恒流里可以当空气 |
| 输出电流 | **要管** | 目标 `CurrentMax=4 A`，给定 `Curr_REF` 每拍最多加 `0.0002` |
| `Duty` | 电流 PI 的输出 | 下限 **0.02**，上限 **0.40**，电流不够就往上加 |
| `Plv` | 写死 | **80000 Hz**（DLL 指令；三角波若仍是 150 kHz，电路频率对不上） |

电池源 `V_dc3` 仍是 32 V。示波器 `Vout` 若接在输出电容上，且串了 `R16`，则 `Vout≈32+Iout×R16`。R16=5 Ω、Iout≈1.25 A 时约 **38 V**，不是电压环在调压。详见 `LLC模式5_ConCurPWM分析.md`。`vOut_Bat_adc >= 40` 才会离开模式 5。

## 程序现在走哪几层（每拍 40 kHz）

```
plecsOutput
  → ADC0_Sample          只做 FIR，物理量已由模型写入
  → HandleFast
      → SampleLpfHandle  电压/电流一阶低通
      → PowerCtrHandle
          → switch(5) → ConCurrHandle()     ← 你现在在这里
              → UpdateCurrentMaxFromCan()   握手=1 时 CurrentMax 仍约 4 A
              → RefRampPwmCurr()            Curr_REF 缓升，步进 0.0002
              → PiPwmCompute()              电流增量 PI → Duty
              → DriveSet(DrvH,1,0, 80000, Duty)
              → PwmSyncDrvUpdate()          SR 先不管；电流<4 A 时 SynDrv 一直 0
      → SHRTIMERdrive    仿真不算寄存器，只保留 Duty/Plv/SynDrv
```

**不会进：** `SoftCurStart`（2）、`ConVoltHandle`（4）、`ConPfmHandle`（7）。  
判断方法：示波器 `CtrMode` 保持 **5**，`Plv` 钉在 **80000**（上轴若是 ×1e4，线在 8.0）。

## Duty 从 0.02 往上，值怎么变

`ConCurrHandle` 电流环：

```
err = Curr_REF - iOut_Bat_LPF          限幅 ±0.2 A
ΔDuty = 0.0005*Δerr + 0.001*err
Duty  = Duty + ΔDuty                   钳在 [0.02, 0.40]
Duty<=0.02 时 DrvH=0；超过 0.02 后 DrvH=1、DrvL=1
```

`Curr_REF` 不是一步到 4 A：

- 每拍最多 `REF_RAMP_STEP=0.0002`（40 kHz 下约 8 A/s）
- 还要求 `Curr_REF` 落在实测电流 ±1 A 窗内才继续爬
- 所以电流应是一条慢慢往 4 A 靠的斜线，不是阶跃

你这张下轴（×1e-2）：先水平 **0.02**（PI 下限），再台阶式升到约 **0.055**。说明已经进了 `ConCurrHandle`，电流还小于给定，PI 在加占空比。这是环在动，不是卡死。

对应你应同时看的量：

| 示波器 | 现在期望 |
|---|---|
| `CtrMode` | 一直 5 |
| `Plv` | 一直 80000 |
| `Duty` | 0.02 起往上，未到 0.40 都正常 |
| `DrvH` | Duty 刚贴 0.02 时可能是 0，起来后变 1 |
| `Iout` | 应往 4 A 爬（体二极管整流，会偏小、偏慢） |
| `Vout` | 电容电压 `≈32+I×R16`；R16=5 且 1.25 A 时约 38 V，不要等 54 V |
| `SynDrv` | 本轮不加 SR，且 Iout 未连续 >4 A 时保持 0 |

## 电流如果几乎不涨、只有 Duty 在涨

环认为“电流不够”，就会一直加 Duty。常见原因（先不改模型，只用来对波形）：

1. 试验模型 PWM 里若还有 **`Duty × 0.01`**，电路实际占空比只有 0.0002～0.00055，电流几乎起不来，DLL 里的 Duty 仍会继续往 0.40 爬。
2. 三角波仍是 **150 kHz**，固件按 80 kHz 出 Duty，增益对不上，电流也会偏小。
3. SR 没接时靠体二极管，能充电流，但同样电流需要更大 Duty。

先确认 **`Iout` 是不是在动**。Duty 在动 + CtrMode=5 + Plv=80000 = 程序路径对；电压 32 不是故障。

---


# LLC Duty 模型对照说明（不改模型）

## 1. 单位总表（模型 ↔ DLL ↔ 固件）

模型导线和示波器看到的，就是 DLL `outputs[]` 的数，**不要再乘 0.1 或 0.01**。

| 信号                                               | 单位          | 倍数           | 典型值                                   | 含义                        |
| -------------------------------------------------- | ------------- | -------------- | ---------------------------------------- | --------------------------- |
| `vOut_Rly_adc` / `vOut_Bat_adc`                    | **V**         | **×1**         | 继电器侧 / 电池约 32                     | 电压表读数直接进环          |
| `iOut_Bat_adc`                                     | **A**         | **×1**         | 0～10                                    | 电流表读数直接进环          |
| `Volt_Ref`                                         | **V**         | **×1**         | 54                                       | 电压给定                    |
| `CurrentMax`                                       | **A**         | **×1**         | 4                                        | 电流上限                    |
| `PFC_ok` / `xp_HandShake` / `xp_ChrgEna` / `preOK` | 逻辑          | **0 或 1**     | 1                                        | 不是 0.1                    |
| `CtrMode` / `CtrMode1`                             | 枚举整数      | **×1**         | 0～8，或 **-1**                          | -1 = 慢环无新沿，快环保持   |
| **`Duty`**                                         | **占空比 pu** | **×1（0～1）** | PWM 约 **0.02～0.40**；PFM 固定 **0.50** | 三角波比较阈值              |
| **`Plv`**                                          | **Hz**        | **×1**         | 见下表                                   | 开关频率指令                |
| `SynDrv`                                           | 使能          | **0/1**        | 电流起来后变 1                           | SR 允许开通，**不是占空比** |
| `DrvH` / `DrvL`                                    | 使能          | **0/1**        | 1=允许该桥臂出波                         | 不是占空比                  |
| `RelaySta` / `DisCharge`                           | 逻辑          | **0/1**        | —                                        | 慢环观测                    |

**0.1 倍只存在固件内部 CAN 字段**（`BMS2_ChrgReqVolt` 等）。抽壳已经 `物理量 × 10` 再写进去，模型侧仍送 **54、4**，不要送 540、40。

**0.01 倍不属于设计。** 试验模型里 PWM 比较前有一个 `Gain=0.01`，会把 0.20 变成 0.002，几乎没脉冲。这是错的。

---

## 2. `Plv` 是什么？PFM 看哪个数？

`Plv` = `DriverPwm.Plv` = **开关频率，单位 Hz**。  
固件里 HRTIM 用 `680e6 / Plv` 算周期计数；仿真里 **三角波频率就应该等于 `Plv`**。现在两份模型三角波都写死 **150000**，**没吃 `Plv`**。

| `CtrMode` | 名字                        | `Plv` 应看到              | `Duty` 应看到           |
| --------- | --------------------------- | ------------------------- | ----------------------- |
| 0         | `NoSelect` 空闲             | 250000                    | ~0.02，且 `DrvH=0`      |
| 1 / 2     | `SoftStar` / `BmsStar` 预充 | 120000 或 40000           | 开环约 0.01～0.07       |
| 4         | `ConVolt` PWM 恒压          | **40000**                 | 电压 PI，上限约 0.4     |
| **5**     | **`ConCurPWM` PWM 恒流**    | **80000**                 | 电流 PI，**0.02～0.40** |
| 8         | `PwmHold`                   | 80000（后段 250000）      | 约 0.04                 |
| 6         | `Transition`                | 250000                    | 斜坡                    |
| **7**     | **`OvLoadPFM`**             | **60000～250000（变频）** | **固定 0.50**           |

PFM 时：

- **频率就是 `Plv`（Hz）**，例如 120000 = 120 kHz。
- 范围宏：`POW_MIN_PLV=60000`，`POW_MAX_PLV=250000`。
- 电流越大通常频率越低（往 60 kHz 走），空载/关波靠近 250 kHz。
- **不要看 `Duty` 判断 PFM**，PFM 的 `Duty` 被写成 0.5，调的是频率不是占空比。

PWM 占空比对应的就是 **`Duty`**（Demux 第 0 路，Goto 标签 `Duty`）。  
示波器若看到一直 0.20，说明还在吃旧 DLL；新 DLL 空闲约 0.02，恒流会在 0.02～0.40 间动。

---

## 3. 各模块在干什么

```
电路电压/电流(V/A)
    → 快环 DLL(40 kHz) → Duty, Plv, SynDrv, DrvH, DrvL, CtrMode
         Duty+DrvH/L → PWM control → DutyH/DutyL → FETD21/22 门极
         Plv        → 设计应接三角波频率（现在没接）
         SynDrv     → 设计应接 FETD23/24 门极（现在没接）
    → 慢环 DLL(设计 5 ms) → CtrMode 请求(-1=保持), Volt_Ref, CurrentMax
```

### 3.1 快环 DLL（`pi_controller`，`1/Fs`，`Fs=40e3`）

Mux 8 / Demux 6，和抽壳一致：

| Mux  | 端子名         | 写入           | 单位      |
| ---- | -------------- | -------------- | --------- |
| 0    | `vOut_Rly_adc` | 继电器侧电压   | V         |
| 1    | `vOut_Bat_adc` | 电池电压       | V         |
| 2    | `iOut_Bat_adc` | 输出电流       | A         |
| 3    | `PFC_ok`       | PFC 就绪       | 0/1       |
| 4    | `CtrMode1`     | `>=0` 才改模式 | 枚举或 -1 |
| 5    | `Volt_Ref`     | 电压给定       | V         |
| 6    | `CurrentMax`   | 电流上限       | A         |
| 7    | `xp_HandShake` | 1=BMS，2=PMS   | 0/1/2     |

| Demux | 端子名    | 来自               | 单位 |
| ----- | --------- | ------------------ | ---- |
| 0     | `Duty`    | `DriverPwm.Duty`   | 0～1 |
| 1     | `SynDrv`  | `DriverPwm.SynDrv` | 0/1  |
| 2     | `DrvH`    | 原边高端允许       | 0/1  |
| 3     | `DrvL`    | 原边低端允许       | 0/1  |
| 4     | `Plv`     | 开关频率           | Hz   |
| 5     | `CtrMode` | 当前快环模式       | 0～8 |

里面跑的就是固件：`ADC0_Sample`（只做 FIR）→ `HandleFast` → `PowerCtrHandle` + `SHRTIMERdrive`（不算 HRTIM 寄存器）。

### 3.2 PWM control（三角波比占空比）

1. 三角波 `LEAD7`：0～1。
2. `Duty > 三角波` → 一条脉冲，宽度 = `Duty × 周期`。
3. 死区 `dt=150e-9` 拆成互补 `DutyH` / `DutyL`。
4. `DrvH=0` 则高端强制 0；`DrvL=0` 则低端强制 0。
5. `DutyH` → `FETD22` 门极（端子 3），`DutyL` → `FETD21` 门极（端子 3）。

这和固件 PWM 比较值一致：脉冲宽度 ≈ `Duty × 开关周期`。  
**前提：三角波频率 = `Plv`。** 现在是 150 kHz，PWM 恒流固件是 80 kHz，PFM 还要变，对不上。

试验模型在比较前 **`Duty × 0.01`**，和固件单位冲突。主模型这里是直连，这一处主模型是对的。

### 3.3 慢环 DLL（仅主模型已放）

端口名字和 `main_slow.c` 对得上（8/6），但：

- 文件名 `llc_slow.dll`：对
- SampleTime = `1/1e3` = **1 ms**，设计是 **5 ms**
- `OutputDelay=0`，设计要极小延时（如 `1e-9`）打破代数环
- 快→慢的 `CtrMode` **没有 Delay**
- `CAN` 常量现在是 **1**，慢环当 **Vreq=1 V**；`ChargeOn` 要求请求电压 ≥ 30 V，**进不了充电沿**
- `vOut_Rly` 和 `vOut_Bat` 都来自标签 `Vout`，电池 32 V 源没有单独进 DLL

慢环只改模式/给定，不跑 PI。无新沿时 `CtrMode` 输出 **-1**。

### 3.4 功率级

- `V_dc` 母线约 350 V，电池源 `V_dc3=32 V`
- `FETD21/22`：原边半桥，门极已接 `DutyL/DutyH`
- `FETD23/24`：同步整流，功率端子已接，**门极（端子 3）悬空**，只有体二极管

---

## 4. 同步整流怎么控、时序是什么

DLL 出的 `SynDrv` 是 **允许位**，不是 SR 占空比。精确开通窗口在固件 `SHRTIMERdrive` / `Sr_Handle.c`，Duty 模型这一轮做不到逐拍等价。

### PWM（`ConCurPWM` / `PwmHold`）

`PwmSyncDrvUpdate()`：

- `SynDrv==0` 且 `iOut_Bat_adc > 4 A` 连续 **1000 拍（25 ms @ 40 kHz）** → `SynDrv=1`
- 已开通后 `iOut_Bat_adc < 2 A` → `SynDrv=0`
- 其它 PWM 模式（预充、恒压）`SHRTIMERdrive` 会把 `SynDrv` 清 0

开通后固件还会算 `Sr_Atime≈200` 计数（约百 ns 级错开），再写 Timer D。模型里没有这些比较值。

### PFM（`OvLoadPFM`）

- FIR > **6 A** 开，adc 或 FIR < **4 A** 关
- `Sr_Atime` / `Sr_Btime` 查表，单位是 HRTIM 计数 × `SR_DELAY_TIME=0.68`
- 原边仍近 50%（`Duty=0.5`），SR 窗口随频率/电流变，**不是一条固定占空比**

### 模型实际接到哪

| 项目                  | 主模型                               | 试验模型                      |
| --------------------- | ------------------------------------ | ----------------------------- |
| `SynDrv` 有 Goto      | 有                                   | 有                            |
| 接到 `FETD23/24` 门极 | **没有**                             | **没有**                      |
| 示波器                | `From7` 读 **`SR_DUTY`（无此标签）** | 已改成 `SynDrv`，仍只进 Scope |

所以现在 SR 管一直靠体二极管，`SynDrv` 变 1 也看不到同步整流电流波形变化。

---

## 5. 两份模型是否符合设计

### 已经对齐的

- 快环 Filename = `pi_controller`，Mux=8、Demux=6
- 快环 `SampleTime=1/Fs`，`Fs=40e3`
- 端口命名已用固件变量名
- 原边 `DutyH/DutyL` 已上门极
- 试验模型给定更接近验收：`CtrMode2=5`（恒流）、`CurrentMax=4`、`Volt_Ref=54`、`xp_HandShake=1`

### 还没搭好（按优先级）

1. **三角波频率写死 150 kHz，不吃 `Plv`** — PWM/PFM 频率都错  
2. **`FETD23/24` 门极没接 `SynDrv`** — SR 没真正控  
3. **主模型** 快环 From 找 `vOut_Rly_adc` / `vOut_Bat_adc` / `iOut_Bat_adc`，电路 Goto 仍是 `Vout` / `Iout` — **采样可能悬空为 0**  
4. **试验模型 `Duty × 0.01`** — 和固件 0～1 冲突  
5. 主模型慢环 1 ms、`Vreq=1`、无 Delay；试验模型没有慢环（先验快环可以）  
6. 主模型 `Constant1=4` 的标签是 `CtrMode1`，那是 **`ConVolt` 枚举**，不是 4 A

---

## 6. 怎么判断“已经搭好”

仿真后看 Scope（不要看旧的 0.20 死值）：

| 检查项           | 搭好时应看到                            | 没搭好                                                       |
| ---------------- | --------------------------------------- | ------------------------------------------------------------ |
| `CtrMode`        | 恒流试验保持 **5**                      | 一直 0，或被慢环反复打回 2                                   |
| `Plv`            | 恒流 **≈80000**                         | 一直 150000（那是三角波，不是 DLL）；或 250000（空闲）       |
| `Duty`           | 在 **0.02～0.40 之间动**，不是常数 0.20 | 一直 0.20=旧 DLL；≈0.002=被 ×0.01；一直 0.02 且 DrvH=0=没进环 |
| `DrvH/DrvL`      | 出波时多为 1                            | 一直 0                                                       |
| `Iout`           | 往 `CurrentMax`（4 A）爬                | 接近 0：采样没接到，或 Duty 被 ×0.01                         |
| `Vout`           | 试验模型钉在 **32 V** 算对              | 不要用电压有没有升判断模式 5                                 |
| `SynDrv` / SR    | **本轮不加**，先忽略                    | 门极悬空是预期                                               |

PFM 是否搭好：`CtrMode=7`，**`Plv` 在 60k～250k 间动**，`Duty` 钉在 **0.5**。现在三角波仍 150 kHz，电路频率不会跟 `Plv` 变。

---

## 7. 你自己改模型时的最小清单（我这边不改）

1. 确认打开的是刚编的 `pi_controller.dll`（和 `.plecs` 同目录）。  
2. 主模型：把 `Vout`/`Iout` 的 Goto 改成快环 From 同名，或把 From 改回 `Vout`/`Iout`；电池电压应用电池侧表，不要和 `Vout` 共用。  
3. 试验模型：删掉 PWM 里 `Duty` 的 `Gain 0.01`。  
4. 三角波 `f` 改接 `Plv`（或先钉 80000 只验恒流）。  
5. `From SynDrv` → `FETD23`、`FETD24` 端子 3。  
6. 慢环先断开也行；要接则 5 ms、`Vreq=54`、`CurrentMax=4`、快→慢 `CtrMode` 加 Delay，无新沿为 -1。

**一句话记单位：** 电压电流是 1 V / 1 A；占空比是 0.25 这种小数；频率是 80000 这种整数 Hz；SR 是 0/1。没有 0.1 显示、没有 0.01 增益。