# 快慢环真实场景演示（一场景一文件）

母版：`Visual studio projects/pi_controller/x64/Debug/LLC1600_PWM - DLL - test - Sr - Slow loop - dty - FullTest.plecs`

原则：**功率级、快环、慢环、PWM control 只在母版里搭一次。** 每个要演示的场景另存一份，只改 CAN / 握手 / 使能 / 电池。不要用 Manual Switch 把四五个互相冲突的植物塞进一个模型——它只有两路。

切文件后必须 **Simulation → Reinitialize**（或关掉再开这份 `.plecs`）。DLL 内部状态机不跟文件名复位，同一次仿真里改 Constant 也不算新场景。

---

## 1. 母版 FullTest 检查结论（先看再复制）

当前 FullTest **已经是快+慢联调骨架**，不要拆功率级和 `PWM control`。对照如下。

| 项 | 现状 | 结论 |
|---|---|---|
| 快环 DLL | `pi_controller`，8 入 17 出 | 保留 |
| 慢环 DLL | 8 入 6 出 | 保留 |
| 快环 `CtrMode`（端子 7） | 接慢环命令（脉冲后为 -1） | **正确**，场景 0～3 不要改 |
| 快环 `Handshake` | 接慢环 Handshake 回读 | 正确（与慢环同一根 Constant5） |
| 快环 `CurrentMax` | `Constant3 = 4`，**没有**接慢环 | 正确（慢环这路常为 0） |
| 快环 `Volt_Ref` | `Constant4 = 54` | 与慢环 CAN 一致 |
| 慢环 `ChrgEna` | `Constant8 = 1`（约 x=645, y=1015） | 现模型是充电使能 |
| 慢环 `Handshake` | `Constant5 = 1`（约 y=1235） | 现模型是 BMS 握手 |
| 慢环 `CAN/Vreq` | `Constant9 = 54`（约 y=1300） | 充电电压给定 |
| 慢环 `vOut_Rly` | `Constant10 = 0` → Goto `vOut_Rly` | ChargeOn 需要继电器侧低于电池，**充电场景保持 0** |
| 慢环 `preOK` | `Constant2 = 0` | 保持 |
| 电池 | `V_dc = 32 V`，串联 `R16 = 5 Ω` | 充电植物 |
| `TimeSpan` | **1.5 s** | 场景 0 够用；充电建议 2 s |
| PWM 沿 | Timer C `PwmOn/Off` 已进 `PWM control` | 不要改 |
| Manual Switch | **没有** | 不用再加拨杆切场景 |
| 预充偏置 | `Relational CtrMode==2` | 模式 2 预充用，保留 |

**不要把 FullTest 直接当「PWM 恒流」演示。**  
`32 + 4 A × 5 Ω = 52 V`，会冲过 40 V，模式 5 站不住。恒流 32 V 那场要把 `CurrentMax` 改成 **0.8**。

现慢环 DLL 限制（所有副本都一样）：

- `ChrgEna=1` 才进 `Charging` → `ChargeOn`。
- `Handshake=1` → `xp_CVmode=1`（CC，发 2）；`Handshake=2` → `xp_CVmode=2`（CV，发 3）。
- `ChrgEna=0` 且 `Handshake=0` → `Wakeup`（发 1）。

---

## 2. 场景文件（演示哪个就开哪个）

在资源管理器里把 FullTest **另存**，建议文件名：

| 文件 | 场景 | 台架含义 | 现 DLL 能否自动进 |
|---|---|---|---|
| `...dty - S0-Idle.plecs` | 0 待机 | 未允许充电，关波 | 能（改两个 Constant） |
| `...dty - S1-Wakeup.plecs` | 1 空载唤醒 → 30 V | 无电池、无握手 | 现 `llc_slow.dll` 已能发 1 |
| `...dty - S2-CC32.plecs` | 2 CC@32 V PWM | 有电池、握手、低于 40 V 恒流 | 能 |
| `...dty - S3-CC42.plecs` | 3 CC@42 V PFM | 电池已 ≥40 V | 能 |
| `...dty - S4-CV30.plecs` | 4 恒压 30 V | PMS 恒压（模式 3） | 现 DLL：HS=2、Ena=1 发 **3** |
| `...dty - S5-CC-SR.plecs` | 5 PWM 大电流 + SR | 模式 5，`Iout>4 A` 开通同步管 | 能（要改 R16） |
| `...dty - S6-CC42-SR.plecs` | 6 PFM 42 V + SR | 模式 7，FIR`>6 A` 开通同步管 | 能 |

不要每个场景复制 DLL。七份模型指向同一份 `pi_controller.dll` / `llc_slow.dll`。

公共 Scope（每份都已有就别拆）：慢环命令、快环 `CtrMode`、`Plv`、`Duty`、`DrvH`、`Vout`、`Iout`、`Handshake`。  
判据看快环回读 `CtrMode`（台阶），不要看慢环那根脉冲，也不要看 Delay 后的 `CtrMode1`。

---

## 3. 各文件只改这些旋钮

母版里左侧一列 Constant（约 x=645）对应关系：

| 画面上的块 | 物理量 | 接到 |
|---|---|---|
| `Constant8` | `ChrgEna` | 慢环使能 |
| `Constant5` | `Handshake` | 慢环 + 快环握手 |
| `Constant9` | `CAN` / `Vreq` | 慢环电压给定 |
| `Constant4` | `Volt_Ref` | 快环电压给定 |
| `Constant3` | `CurrentMax` | **只接快环** |
| `Constant10` | 慢环 `vOut_Rly` | 充电场景保持 0 |
| `V_dc` | 电池电压 | 输出经 R16 |
| `R16` | 充电电阻 | 现 5 Ω |

| 旋钮 | S0 | S1 | S2 | S3 | S4 | S5 PWM+SR | S6 PFM+SR |
|---|---|---|---|---|---|---|---|
| `ChrgEna` | **0** | **0** | **1** | **1** | **1** | **1** | **1** |
| `Handshake` | **0** | **0** | **1** | **1** | **2** | **1** | **1** |
| `Vreq` / `Volt_Ref` | 任意 | 30 | **54** | **54** | **30** | **54** | **54** |
| `CurrentMax` | 4 | 4 | **0.8** | **4** | 4 | **4.5** | **8** |
| `V_dc` | 任意 | **0** | **32** | **42** | **0** | **32** | **42** |
| `R16` | 5 | — | **5** | 5 | — | **1.0** | **2** |
| 慢环 `vOut_Rly` | 0 | **Vout** | **0** | **0** | **Vout** | **0** | **0** |
| 快环 `CtrMode` | 慢环 | 慢环 | 慢环 | 慢环 | 慢环 | 慢环 | 慢环 |
| `TimeSpan` | 1 s | 2 s | 2 s | 3 s | 2 s | **3 s** | **4 s** |

S2 用 `CurrentMax=0.8`、R16=5：停在模式 5，电流不够开 SR。  
S5：R16=1.0，`32+4.5×1=36.5 V<40`，PWM 门槛 **4 A**。  
S3：`CurrentMax=4`，FIR 到不了 6 A，**开不了 PFM 的 SR**。S6 把电流提到 8 A。

---

## 4. 从第一个场景开始搭：S0 待机

下面只做 **S0**。做完、跑通，再复制出 S2。不要一次改五个文件。

### 4.1 复制母版

1. 关掉 PLECS 里已打开的 FullTest（避免改错母版）。
2. 资源管理器中复制  
   `LLC1600_PWM - DLL - test - Sr - Slow loop - dty - FullTest.plecs`  
   粘贴并改名为  
   `LLC1600_PWM - DLL - test - Sr - Slow loop - dty - S0-Idle.plecs`  
   仍放在 `x64/Debug/`（和 DLL 同目录）。
3. 用 PLECS 打开 **S0-Idle**，确认窗口标题是新文件名。

### 4.2 只改两个数

在画布左侧、慢环子系统附近找到：

1. **`Constant8`（ChrgEna）**：双击，Value 从 `1` 改成 **`0`**。
2. **`Constant5`（Handshake）**：双击，Value 从 `1` 改成 **`0`**。

其余全部不动：

- 快环 `CtrMode` 继续接慢环命令（不要手拨 0，待机应由慢环不发 1/2 来维持）。
- `CurrentMax`、`Volt_Ref`、`V_dc=32`、`R16=5` 都可以留着——待机根本不开波。
- `PWM control`、门极、预充偏置链不要动。

### 4.3 仿真设置

- `Simulation` 参数：`TimeSpan` 改成 **`1`**（1 秒足够）。
- `MaxStep` 保持母版的 `1e-7`。
- Scope 至少看：快环 `CtrMode`、`DrvH`、`DrvL`、`Plv`、`Vout`。

### 4.4 运行

1. **Simulation → Reinitialize**（或 Ctrl+Shift+I）。
2. 运行。

### 4.5 通过 / 失败

通过：

- 慢环命令全程 **-1** 或偶发一针 **0**，**没有 1、没有 2、没有 3**。
- 快环 `CtrMode` 保持 **0**。
- `DrvH = DrvL = 0`（判断关波只看这两根，不看 Duty）。
- `Plv` 停在约 **250000**，`Duty≈0.02`。
- 输出电容电压往电池（32 V）掉或保持，没有充电电流往上爬。

失败对照：

| 现象 | 原因 |
|---|---|
| `CtrMode` 变成 2 或 5 | `ChrgEna` 还是 1，ChargeOn 仍在发 2 |
| `DrvH` 有脉冲 | 快环端子 7 被别的 Constant 钉死成非 0 |
| 一开始就出波、模式 5 | 打开的还是 FullTest，不是 S0 |

S0 通过之后，母版接线已被这份副本验证过。后面每个场景都是「再复制一份 + 改表里那几个数」。

---

## 5. 后面四个场景（S0 通过后再做）

### S2 CC@32 V（第二份，最稳的充电演示）

从 **S0-Idle** 或 FullTest 另存为 `S2-CC32`。

| 块 | 值 |
|---|---|
| `ChrgEna` | **1** |
| `Handshake` | **1** |
| `CAN` / `Volt_Ref` | 54 |
| `CurrentMax` | **0.8**（必须改，否则过 40 V） |
| `V_dc` | 32 |
| 慢环 `vOut_Rly` | **0** |
| 快环 `CtrMode` | 慢环命令 |
| `TimeSpan` | **2** |

通过：~0.28 s 慢环一针 **2** → 快环 `0→2` 预充（40 kHz）→ 约 0.8 s 进 **5**；`Plv=80 kHz`；`Iout` 爬向 0.8 A；`Vout≈32+I×5` **低于 40 V**；原边两短脉冲，不是互补 50%。

失败：`CurrentMax` 仍为 4 → 冲过 40 V；慢环 `vOut_Rly` 接了 `Vout` → ChargeOn 进不去。

**进了 5 但没有 PWM（Duty→0、DrvH=0、Plv 仍 250 kHz）：**  
慢环 `CurrentMax` 出口（端子 10）被接到了快环 `CurrentMax`。慢环这路恒为 **0**，模式 5 里 `CurrentMax≤0.1` 会关波并把 Duty 清零。  
`Constant3=0.8` 只进了 Goto，**没有进快环**。拆掉慢环→快环那根电流线，把 `From(CurrentMax)` 接到快环端子 9。Handshake 回读可以留。

### S3 CC@42 V（过 40 V，PFM）

另存为 `S3-CC42`，在 S2 基础上只改：

- `V_dc` → **42**
- `CurrentMax` → **4**
- `TimeSpan` → **3**

通过：慢环一针 2；快环 `2→6→7`（不长期停 5，不应出现 8）；进 7 后 `Duty=0.5`，`DrvH=1`，`Vout≥39`；`Plv` 从 250 kHz 往下走，电流爬向 4 A。`CurrentMax=4` **开不了 SR**（PFM 门槛 6 A），要开 SR 用 **S6**。

失败：电池仍是 32 V → Hold 把电压砸穿 39 V，模式停在 7 但 `DrvH=0`。那是植物，不要改 PWM control。

### S1 空载唤醒 → 30 V

另存为 `S1-Wakeup`（现 `llc_slow.dll` 已能自动发 1）。

| 块 | 值 |
|---|---|
| `ChrgEna` | 0 |
| `Handshake` | 0 |
| `V_dc` | **0**（关掉电池源） |
| 慢环 `vOut_Rly` | 改接到 **`Vout`**（唤醒要看继电器侧 <28 V） |
| 快环 `CtrMode` | 慢环命令 |

通过：约 0.2 s 慢环一针 **1**；快环 `0→1`（120 kHz 开环）→ `1→4`；`Plv=120 kHz`；**Vout 稳在约 30 V**；Handshake 全程 0。

失败：电池没切到 0（`Vbat>5`）→ 唤醒条件不满足。PLECS 若仍加载旧 DLL，先关掉模型再开，或看 `llc_slow.dll` 时间戳。

### S4 恒压 30 V（模式 3）

另存为 `S4-CV30`（或用现有 `...FullTest -CV-30V.plecs`）。**现 DLL 已能发 3**，快环 `CtrMode` 接慢环命令，不要钉死 3。

| 块 | 值 |
|---|---|
| `ChrgEna` | **1** |
| `Handshake` | **2**（不要用 1） |
| `CAN` / 快环 `Volt_Ref` | 都是 **30**。快环 Volt_Ref 用 Constant，**不要接慢环出口** |
| `CurrentMax` | Constant **4**（不要接慢环） |
| `V_dc` | **0** / 拆掉 |
| 慢环 `vOut_Rly` | **Vout**（须 `< 30−2` 才能进 CV） |
| 快环 `CtrMode` | 慢环命令 |
| `TimeSpan` | **2** |

通过：约 0.2 s 慢环可能先一针 **0**，再一针 **3**；快环进 3；`Plv=40 kHz`；Vout 稳在约 **30 V**。

失败：`ChrgEna` 仍为 0 → Stadby 不发 3；`CAN=54` → 预充冲不过；快环 `Volt_Ref` 接了慢环 0 → CAN 电压为 0 关波。

`Volt_Ref=54` **到不了 54 V**：预充出口是 `vRly>Volt_Ref`，`vRly≥40` 后开环 Duty 钉在 0.05。54 V 功率输出是模式 **7 PFM**。

### S5 PWM 大电流 + 同步整流

从 **S2-CC32** 另存为 `S5-CC-SR`（不要从 CV 那场复制）。

固件 PWM 开 SR（`PwmSyncDrvUpdate`，模式 5）：

- `Iout > 4 A` 连续 **1000 拍**（40 kHz 下约 25 ms）→ `SynDrv=1`
- 已开通后 `Iout < 2 A` 才关
- `SynDrv=0` 时 DLL 把 SR 四路沿清 0，副边门极全关

| 块 | 值 |
|---|---|
| `ChrgEna` / `Handshake` | **1** / **1** |
| `CAN` / 快环 `Volt_Ref` | 54（快环用 Constant，不要接慢环） |
| `CurrentMax` | **5**（只接快环 Constant） |
| `V_dc` | **32** |
| `R16` | **1.5 Ω**（必须改，母版 5 Ω 会过 40 V） |
| 慢环 `vOut_Rly` | **0** |
| 快环 `CtrMode` | 慢环命令 |
| `TimeSpan` | **3** |

Scope 在公共 8 路之外再加：`SynDrv`、`SrH`、`SrL`（或 FETD23/24 门极）、`Iout`。

通过：

- 慢环一针 **2** → 快环 `0→2→5`，`Plv=80 kHz`，**停在 5**（不要出现 8/6/7）
- `Iout` 按约 8 A/s 爬到约 **5 A**（进 5 后大约 0.6 s）
- `Vout ≈ 32+I×1.5`，约 39 V，**低于 40 V**
- `Iout` 连续 >4 A 约 25 ms 后 **`SynDrv` 从 0 变 1**
- 之后副边两管出现窗口脉冲（贴在原边脉宽内侧，比原边窄），不是一直 0

失败对照：

| 现象 | 原因 |
|---|---|
| `Iout` 只有 0.8 A，`SynDrv` 一直 0 | 打开的还是 S2，`CurrentMax` 没改 |
| 进 5 后很快闪 8 / 进 7 | `R16` 还是 5 Ω，电压过 40 V |
| **`Iout` 刚过 4 A 就停波，`CtrMode` 变成 8 再 7** | SR 开通瞬间电压尖峰踩过 40 V，Hold 把电压砸穿 39 V，模式 7 关波。`R16` 再改成 **1.0 Ω**，`CurrentMax=4.5`，让 `32+4.5×1.0=36.5 V` 离 40 V 远一点 |
| `Iout>4 A` 但 `SynDrv=0` | 没连够 25 ms，或 Scope 看的不是快环 `SynDrv` |
| `SynDrv=1` 但副边门极仍 0 | `PWM control` 里 SR 窗没和 `SynDrv` 相与，见 `PWM和PFM同步整流的说明.md` |

### S6 PFM 42 V + 同步整流

从 **S3-CC42** 另存为 `S6-CC42-SR`（不要从 S5 复制：S5 是 32 V PWM，R16=1 Ω）。

固件 PFM 开 SR（`Sr_compute()`，模式 7）：

- `iOut_Bat_FIR ≥ 6 A` → `SynDrv=1`（滤波电流，比 ADC 略滞后）
- 已开通后 `iOut_Bat_adc < 4 A` 才关
- 窗口是 **频率查表**（`SrA`/`SrB`），不是 PWM 那套贴原边内侧的固定窄窗
- `Plv ≥ 102 kHz` 时 `SrA > SrB`；降到 102 kHz 以下换另一张表

| 块 | 值 |
|---|---|
| `ChrgEna` / `Handshake` | **1** / **1** |
| `CAN` / 快环 `Volt_Ref` | 54（快环用 Constant，不要接慢环） |
| `CurrentMax` | **8**（只接快环 Constant；S3 的 4 A 不够开 SR） |
| `V_dc` | **42** |
| `R16` | **2 Ω**（`42+8×2=58 V`，植物能出。母版 5 Ω 时 `42+8×5=82 V` 太高） |
| 慢环 `vOut_Rly` | **0** |
| 快环 `CtrMode` | 慢环命令 |
| `TimeSpan` | **4** |

Scope 在公共 8 路之外再加：`SynDrv`、`SrH`、`SrL`（或 FETD23/24 门极）、`Iout`。电流优先看 FIR 或平滑后的 Iout。

通过：

- 慢环一针 **2** → 快环 `0→2→6→7`（电池已 42 V，**不长期停 5**，不应出现 8）
- 进 7 后 `Duty=0.5`，`DrvH=1`，`Plv` 从 250 kHz 往下走
- `Iout` 按约 8 A/s 爬到约 **8 A**（进 7 后大约 1 s）
- `Vout ≈ 42+I×2`，约 50～58 V，**始终 ≥39 V**（模式 7 锁波门槛）
- FIR 过 6 A 后 **`SynDrv` 从 0 变 1**
- 副边两管出现查表窗口（半周各一扇，宽度随 `Plv` 变），不是一直 0

失败对照：

| 现象 | 原因 |
|---|---|
| `Iout` 停在 4 A，`SynDrv` 一直 0 | 打开的还是 S3，`CurrentMax` 没改成 8 |
| 停在模式 5，80 kHz | `V_dc` 还是 32，当成 S5 了 |
| 进 7 但 `DrvH=0`、电压掉穿 39 V | 植物塌了，或误用了 S5 的 1 Ω / 32 V |
| `Iout` 瞬时过 6 A 但 `SynDrv=0` | 看的是 ADC 尖峰；门槛是 **FIR ≥ 6 A**，再等几十毫秒 |
| `SynDrv=1` 但副边门极仍 0 | `PWM control` 里 SR 窗没和 `SynDrv` 相与 |
| `SrH`/`SrL` 对调、电流反向尖峰 | 副边两路接反，对调 `FETD23`/`FETD24` |

和 S5 的差别：门槛 **6 A 不是 4 A**；模式 **7 不是 5**；窗口查表不是 PWM 嵌套；电池 **42 V** 本来就在 40 V 以上，SR 开通一般不会再踩 Hold。

---

## 6. 推荐演示顺序

1. **S0**（本文第 4 节）确认关波、DLL 加载、快环被慢环按住为 0。
2. **S2** 演示 PWM 恒流（和母版植物最接近）。
3. **S3** 演示过 40 V / PFM。
4. **S4** 演示恒压 30 V（HS=2、Ena=1，慢环发 3）。
5. **S1** 演示唤醒 30 V（现 DLL 已能自动发 1）。
6. **S5** 演示 PWM 大电流并开通 SR（32 V，改 R16 和 CurrentMax）。
7. **S6** 演示 PFM 大电流并开通 SR（42 V，从 S3 改 CurrentMax=8、R16=2）。

现场：打开对应 `.plecs` → Reinitialize → 运行。不要在一份模型里中途改 Handshake 指望切场景。

---

## 7. 接线总图（七份模型共用）

```
ChrgEna / Handshake / Vreq     → 慢环
Handshake 同一根               → 快环 Handshake
CurrentMax Constant            → 快环（不要接慢环）
Volt_Ref Constant              → 快环（S2/S3/S5/S6 为 54；S4 为 30；不要接慢环）
V_dc → R16 → 输出电容          电池植物（S1/S4 拆掉）
慢环 vOut_Rly                  S2/S3/S5/S6 用 0；S1/S4 用 Vout

慢环 OUT 命令 ──────────────── 快环 CtrMode（所有场景）

Vout ── 快环 vOut_Rly
Vout ── 50ms 偏置 ── Switch(CtrMode==2) ── 快环 vOut_Bat（仅 CC 预充）

快环 PWM/SR 沿 ── PWM control ── 门极
```

---

## 8. 慢环补丁（仅仿真，Keil 不走）

**已写入 `main_slow.c` 并编进 `x64/Debug/llc_slow.dll`。** 快环 DLL、Keil 工程都没动。

`ChrgEna=1` → `Charging`（S2/S3 发 2，S4 在 HS=2 时发 3）；`ChrgEna=0` 且 `Handshake=0` → `Wakeup`（S1 发 1）；其余 → `Stadby`。

`Handshake=1` → `xp_CVmode=1`（CC）；`Handshake=2` → `xp_CVmode=2`（CV）。S2/S3 用 HS=1，不受 CV 补丁影响。

S0 若无电池且 Handshake=0，会变成唤醒。要关波：Handshake=1、ChrgEna=0。
