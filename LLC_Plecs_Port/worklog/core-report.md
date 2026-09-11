# Pure-C core implementation report

2026-09-10. Scope: `include/llc_api.h`, `include/llc_types.h`, `include/llc_context.h`, `core/llc_control.c`, `core/llc_fast_task.c`, `tests/test_core.c`. The timer engine, wrappers, build projects and PLECS models belong to the parent task.

## Implementation

- `llc_control.c` adapts the visible `AppUser/mathR02.c` control statements. Globals and function-static mutable state become explicit `LlcContext` members; stateful handlers receive `ctx` arguments. No global current-instance pointer and no STM32/HAL dependency.
- Preserved PWM delta PI versus PFM position PI math, coefficients, raw/LPF/FIR separation, per-handler PI reset distinctions, current ramp/tracking band and 10 A clamp.
- Preserved SoftStar, BmsStar, SoftCurSt/PMS CV, 30 V CV, PWM CC, Hold and Transition handlers. BmsStar's two 20-tick counters and two voltage-dependent 20000-tick counters, SoftStar/PMS CV's `>1000` boundaries, and Hold's accumulated rather than consecutive low-current count remain source behavior.
- Standard V/A requests replace CAN values multiplied by 0.1. Handshake accepts 0/1/2. `mode_request=-1` preserves the current mode while a new transaction can apply other actions; 0..8 are mode enum values.
- Fast input/output shape is 22/28. Sequence 0 means no transaction; first 1 is accepted. Repeated sequence does not repeat initialization or GPIO actions. Continuous request/measurement inputs update every call. Integer/enumeration validation precedes casts. Reset is an edge; each explicit Fast call increments the post-reset tick once.
- `llcSampleStep` computes original ADC-side `0.999*old+0.001*new` V/I filter (called FIR by firmware) plus the original `FilterOne` LPFs. One Fast call samples exactly once. Analog boundary accepts finite values with magnitude at most 1e6; standard request voltages/currents and RMS are nonnegative.
- `llc_fast_task.c` reproduces visible ConsoleFast software protection and special OvFault recovery. Both short-circuit and 42 A software OCP remain conditional on RelayOld, as the actual source states. Fault bit positions preserve HW order: OVP 128, OCP 512, short 4096. Hardware bitmap is latched and slow faults remain externally supplied.
- PWM SR handling uses the visible `PwmSyncDrvUpdate` and `SHRTIMERdrive` statements, including handler rewriting of SynDrv, 200-count advance, 150-count minimum duration, and the source's asymmetric TD windows. This does not imply electrical SR polarity has been validated.

## Explicit limits and host-side safety changes

- Profile 0 creation fails because the true PFM gain/time dependency source is missing. Profile 1 permits visible PWM/startup/Hold/Transition logic, but rejects PFM even on Transition's 400th frame when the new mode is adopted. There is no invented gain, table or substituted PFM/SR algorithm. The PFM current-reference arithmetic remains in source for traceability but no executable PFM loop calls it.
- Diagnostic bits: 1 missing PFM dependency; 2 invalid input; 4 rejected timer image/window; 8 denied fault-clear request. Bits 1/2/4 latch until reset and force all three gate permits, LLC_enable and relay to zero, with discharge enabled. Diagnostic 8 reports refusal without independently shutting down an otherwise healthy state.
- Host validation, explicit same-frame final gate closure and persistent diagnostic shutdown are intentional host safeguards beyond the raw MCU implementation. PWM SR comparisons are range/order checked before being emitted. The root timer implementation also validates primary conversions.
- `init_flags` bit 9 cannot fully reproduce SysReset because this Fast-only contract lacks `iniOk`, raw AC RMS and the timer-start history. Current developer implementation permits clearing fast faults only for the explicit low-AC condition (`vac_rms_fir < 30`, no current hardware/slow faults); otherwise it sets diagnostic 8. Filtered RMS is not claimed equivalent to original raw RMS. Full slow integration must restore the complete permission contract.
- GPIO transaction actions are applied once, then the source fast handlers may overwrite them in the same tick; LLC_enable in particular follows original HandleFast afterward. Timer-running remains a separate model/slow signal.
- Missing-PFM rejection retains mode 7 for diagnostics, emits no timer frame for that mode and shuts down permits. It does not claim to reproduce missing PFM operation. Automatic reverse PFM-to-PWM switching, Burst and power extension were not added.
- PLECS waveform, MOS/SR polarity, electrical stability and full slow business-state tests are outside this core report. No claim of 1600 W operation or complete true-algorithm port is made.

## Test evidence

Tests were written first. With the x64 Visual Studio environment loaded, their initial compilation failed because `llc_api.h` did not yet exist, confirming the unimplemented interface. An earlier direct compiler attempt without vcvars failed on `assert.h`; that environment failure is not counted as algorithm evidence.

The parent built the solution with MSVC x64, UTF-8 source handling and warnings treated as errors, then reported all core/timer/gate test executables passing. The final core executable was independently rerun from `LLC_Plecs_Port`:

```powershell
& './bin/x64/test_core.exe'
```

Observed output: `core tests passed`; exit code 0.

`test_core.c` covers literal delta/position PI values, LPF/FIR values and reset, two independent contexts, profile rejection, sequence idempotency/zero, mode -1, invalid float/integer/handshake input, 10 A and 0.4 PWM limits, 0.1 A shutdown threshold, 39.9/40.0 V mode boundary, SoftStar 1000/1001, PMS CV 1000/1001, BmsStar 20/20/20000, Hold 39/40 plus phase-two tick, Transition 399/400 refusal, RelayOld protection gating, OVP/OCP/short/hardware final closure, reset-edge behavior, and actual OvFault generation/recovery with an interruption and the 139999/140000 boundary.

The numerical waveform engine tests are maintained by the parent. PWM SR threshold/window behavior has been source-reviewed, but dedicated end-to-end SR numerical and polarity tests remain outstanding.
