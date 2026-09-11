# Implementation ledger — plan: docs/superpowers/plans/2026-09-09-LLC-DLL实施与接线计划.md

2026-09-09: Implementation authorized by user.

Ruling: This directory is not a Git repository. Implement in the already-planned isolated LLC_Plecs_Port directory, preserving source firmware and user models. No worktree/branch operation applies.

Ruling: The original model has been moved/updated to workspace root LLC1600_PWM - DLL.plecs. Use a hashed copy of that current model, retain the original unchanged.

Ruling: Missing ARM-only PFM/SR implementation remains a hard boundary for fidelity. Implement and verify available PWM/startup/controller infrastructure; expose missing capability explicitly rather than fabricate gains.

Status: build environment and PLECS automation discovery in progress. Core migration is independent of model generation and waveform adapter.

2026-09-10/11 implementation update:

- Created standalone C core/supervisor, Probe/Fast/Slow/Combined x64 DLLs and VS2022 solution.
- Added exact timer-window calculations, event-driven PLECS gate adapter, generated connected electrical models and benches.
- Compiled with MSVC warnings-as-errors; native tests and loaded-DLL multirate replay pass (80000 fast calls, all 45 outputs equal).
- Real PLECS probe, PWM/PFM-window and PWM replay pass; Combined completes 2s electrical startup and enters PWM CC without faults/diagnostics.
- Fixed PFC rearm ownership, gate reset wiring, PLECS declaration order/top-level terminal registration and relative DLL paths.
- Detected partial RPC result despite no exception; added end-time validation and regression tests, rerunning Split 2s.
- Source directory has continued changes; generated models remain based on the frozen baseline. See source_audit.json instead of claiming current originals match old hashes.
- Remaining fidelity/acceptance limits: absent ARM PFM/SR source, SR electrical polarity, full fault-reset ownership, full operating-range/1600W acceptance. See results/implementation_status.md for the current delivery status.

Final verification: Split rerun completed 2s/2001 points. Both electrical models enter BmsStar at sampled1.096s, relay closes1.255s, PWM CC starts1.755s. Discrete events and Iref agree; max Ibat_FIR difference is7.45058e-5A. Fresh x64 Debug build and all9 local check groups pass; all4 default real-PLECS benches rerun and complete. Startup plots were rendered and visually inspected.
