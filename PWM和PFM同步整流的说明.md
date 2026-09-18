# PWM / PFM 同步整流（查表窗口，不是近似）

一对同步管 `FETD23` / `FETD24`。固件两套开门算法，由 `CtrMode` 在 `SHRTIMERdrive()` 里切换。DLL 已按 **Timer D 比较值原式** 算出窗口并输出，模型用 **同一把 0～1 锯齿** 比较沿触发，**不要**再用 `SynDrv AND DutyH/L`。

## 1. 固件里到底谁在切窗

| | PWM（模式 5、8） | PFM（模式 7） |
|---|---|---|
| 使能 `SynDrv` | `Iout>4 A` 连续 25 ms 开，`<2 A` 关 | FIR`>6 A` 开，adc 或 FIR`<4 A` 关 |
| 窗口算法 | 原边脉宽推迟 200 count，宽 `2·half·Duty−200` | `Sr_compute()` 查 `M_highPlv` / `M_lowPlv` |
| 表值去向 | 不查表，`Sr_Atime=200` | `≥102 kHz`：`Sr_Atime=(表+200)×0.68`，`Sr_Btime=200×0.68`；`<102 kHz` 对调 |
| 窗口宽度 | 滤波后的 `Sr_Dtime` | `half − 2×120 − Sr_Atime − Sr_Btime`，再 0.999/0.001 滤波 |

`SynDrv=0` 时 DLL **把四路沿清 0**（台架写极窄占位脉冲，仿真不能当真开门）。

PFM 窗口（计数 → 归一化 0～1）：

```
起点 = (120 + Sr_Atime) / pre
宽度 = Sr_Dtime / pre
窗1 = [起点, 起点+宽度]          → SrOn1 / SrOff1
窗2 = [0.5+起点, 0.5+起点+宽度]  → SrOn2 / SrOff2
pre = (680e6 / Plv) 对齐到 4 的倍数
```

## 2. 快环 DLL 现为 8 入 17 出

输入不变。输出：

| 序号 | 名字 | 单位 | 含义 |
|---|---|---|---|
| 0 | Duty | 0～1 | 原边占空比指令（示波器用，不再拿去切窗） |
| 1 | SynDrv | 0/1 | SR 总使能 |
| 2 | DrvH | 0/1 | 原边高允许（TC2 / 上管） |
| 3 | DrvL | 0/1 | 原边低允许（TC1 / 下管） |
| 4 | Plv | Hz | 开关频率 |
| 5 | CtrMode | 枚举 | 当前模式 |
| 6 | SrOn1 | 0～1 | 前半周 SR 开 |
| 7 | SrOff1 | 0～1 | 前半周 SR 关 |
| 8 | SrOn2 | 0～1 | 后半周 SR 开 |
| 9 | SrOff2 | 0～1 | 后半周 SR 关 |
| 10 | SrA | s | `Sr_Atime / 680e6` |
| 11 | SrB | s | `Sr_Btime / 680e6` |
| 12 | SrD | s | `Sr_Dtime / 680e6` |
| 13 | PwmOn1 | 0～1 | Timer C CMP1，下管开 |
| 14 | PwmOff1 | 0～1 | Timer C CMP2，下管关 |
| 15 | PwmOn2 | 0～1 | Timer C CMP3，上管开 |
| 16 | PwmOff2 | 0～1 | Timer C CMP4，上管关 |

PWM↔PFM **不必在模型里用 Switch 选原边或 SR 算法**：`CtrMode` 一变，6～16 路自己换成对应比较值。模型只拿同一把 `Saw` 比沿。

## 3. 试验模型怎么改（对着现有 `PWM control`）

快环子系统：

1. DLL 的 `Number of outputs` 和后面 `Demux` 的 Width 都改成 **17**。
2. 原 0～12 路接线不动。
3. 新增 4 个 Output：`PwmOn1` `PwmOff1` `PwmOn2` `PwmOff2`，外面 Goto 同名。`SrA/SrB/SrD` 仍只进示波器。

`DT` 里：只留积分器，0～1 锯齿 **`Saw`** 给原边和 SR 共用。**断开** `Duty` 进 DT 互补比较那一路，门极不再从 DT 的 LS/HS 出来。

`PWM control` 里原边改成和 SR 同一结构（`DrvH`/`DrvL` 的 Switch 保留）：

```
pulseL = (Saw > PwmOn1) AND (Saw < PwmOff1)
pulseH = (Saw > PwmOn2) AND (Saw < PwmOff2)
DutyL = DrvL AND pulseL    → FETD21
DutyH = DrvH AND pulseH    → FETD22
```

SR 维持：

```
pulse1 = (Saw > SrOn1) AND (Saw < SrOff1)
pulse2 = (Saw > SrOn2) AND (Saw < SrOff2)
SrH = SynDrv AND pulse1    → FETD23
SrL = SynDrv AND pulse2    → FETD24
```

极性：窗 1 对前半周（下管）、窗 2 对后半周（上管）。副边反相只对调 `SrH`/`SrL`。

原边门极改用 Timer C 四路沿（见第 5 节），**不要**再拿 `Duty` 进 `DT` 互补切窗，也**不要**用 `DutyH/L` 去切 SR。

## 4. 怎么确认是查表，不是近似

模式 7、`SynDrv=1` 时：

- `SrA`、`SrB` 随 `Plv`、电流跳变，不是常数。
- `Plv ≥ 102 kHz`：`SrA` 明显大于 `SrB`（`SrB ≈ 200×0.68 / 680e6 ≈ 200 ns`）。
- `Plv < 102 kHz`：对调，`SrB` 大。
- `SrH`/`SrL` 脉宽 ≈ `SrD`，**窄于** 原边半周（缩进 `SrA+SrB+死区`），不是 50% 互补。
- `SynDrv=0`：`SrOn/Off` 全 0，门极全 0。

模式 5、`SynDrv=1` 时：

- `SrA ≈ 200 / 680e6 ≈ 294 ns`，`SrB=0`。
- 窗口贴在原边脉宽内侧，起点比原边晚约 294 ns。

5→8→6→7：`CtrMode` 回读变 7 的当拍，`SrA/SrB` 从 PWM 常数变成表值，门极从「跟 Duty」变成「缩进半周」。`CtrMode` 输入要先 5 再改 **-1**，否则强制 5 进不了 PFM。

## 5. 原边 Timer C（和 SR 同一套办法）

台架 HRTIM 死区插入是关的。TC1=`DrvL`/下管：CMP1 置位、CMP2 复位。TC2=`DrvH`/上管：CMP3 置位、CMP4 复位。

| | PWM（非模式 7） | PFM（模式 7） |
|---|---|---|
| 下管窗 | `[T/4 − Duty·T/2, T/4 + Duty·T/2]` | `[120/pre, 0.5 − 120/pre]` |
| 上管窗 | `[3T/4 − Duty·T/2, 3T/4 + Duty·T/2]` | `[0.5 + 120/pre, 1 − 120/pre]` |
| 形态 | 两短脉冲，中心 90°/270°，脉宽=`Duty·T` | 近 50%，固定 120 count 死区 |
| `Duty` | 决定脉宽 | **不进** Timer C |

`DT` 只留积分器，把 0～1 锯齿 `Saw` 送出来。原边比较照抄 SR：

```
pulseL = (Saw > PwmOn1) AND (Saw < PwmOff1)
pulseH = (Saw > PwmOn2) AND (Saw < PwmOff2)
DutyL = DrvL AND pulseL    → FETD21
DutyH = DrvH AND pulseH    → FETD22
```

快环 DLL：`Number of outputs` 和后面 `Demux` 的 Width 都改成 **17**。0～12 路接线不动，13～16 新增四个 Output，外面 Goto 同名，拉进 `PWM control`。

不要：用 `Duty` 做互补切窗；用 Turn-on Delay + NOT 做原边；用 `CtrMode` Switch 在模型里选 PWM/PFM 公式。

核对：

- 模式 5、`Duty=0.4`：两管都是短脉冲，脉宽约 `0.4/Plv`，相差半周，**不是** 40%+60% 互补。
- 模式 7：接近 50%，死区约 176 ns，与 `Duty` 无关。
- `DrvH=0` / `DrvL=0`：对应管门极为 0，沿仍可在示波器上看。

## 6. 那张互补 + Turn-on Delay 图

`Duty` 比载波、`NOT` 出互补、`Turn-on Delay` 做死区、`PWMHEN/PWMLEN` 选通——这是通用半桥互补调制，**只近似 PFM**，发不出 PWM 的 T/4、3T/4 短脉冲。`DT` 也不要再给 PFM 用。原边和 SR 都只比 DLL 送来的沿。
