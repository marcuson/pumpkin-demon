# Firmware implementation action plan

**Overall status:** software delivery complete (T01–T08); prototype validation awaits physical hardware (T09).

**Current task:** T09, awaiting physical prototype measurements. No software implementation tasks remain.

**Latest update:** G3 passed all three native executables, the Nano Every target build, and focused integrated code review. Documentation is updated; no flash/upload or physical measurements were performed.

## Sources and scope

- [Functional specification](../specs/firmware-functional-specs.md): confirmed scenes, effect capabilities, and scheduling.
- [Technical specification](../specs/firmware-technical-specs.md): implementation decisions, formulas, timing, and validation.
- [Electronics specification](../specs/electronical-specs.md) and [mechanical specification](../specs/mechanical-specs.md): hardware constraints.
- [Repository instructions](../../AGENTS.md): English code/documentation, GitNexus checks, and change discipline.

Starting baseline (before T01): `src/main.cpp` was the two-channel diagnostic sketch; production lighting logic and host tests were absent. Those artifacts are now implemented and tracked below. Preserve the existing `nano_every` target and 115200 baud monitor configuration. This plan is an execution tracker, not a replacement for the specifications.

## Tracking rules

- Status values: `TODO`, `IN PROGRESS`, `READY FOR INTEGRATION`, `DONE`, `BLOCKED`, `AWAITING HARDWARE`.
- Update this file when starting or finishing each task, when a check fails, when scope changes, and before handing work back to the user. Record the next action so work can resume without repeating completed tasks.
- Mark a task `READY FOR INTEGRATION` when its branch-local checks pass. Mark it `DONE` only after integration and its applicable checks pass on the integration branch; record the task commit and commands/results in the evidence log. Do not mark future tests as passed or hardware behavior as verified from compilation or serial logs.
- Allow independent tasks to run concurrently in separate worktrees after their prerequisites are available. Keep one active owner per lane and per file; tasks within a lane remain sequential. Extend the core host assertion executable incrementally, with separate adapter tests in the hardware lane.
- Before editing existing code symbols, follow the repository's GitNexus impact requirements. Resolve unknown graph results against current source. Run graph change analysis before any implementation commit. Task commits and worktree creation belong to the future implementation workflow; this plan update does not execute them.
- If an implementation detail changes an agreed requirement, record the discrepancy and update the relevant specification once resolved. Do not silently weaken requirements to pass a test.
- Hardware access is not required to finish software delivery. Record physical checks as `AWAITING HARDWARE` with an actionable checklist when they cannot be performed.

## Task board

| ID | Task | Depends on | Status | Completion evidence |
|---|---|---|---|---|
| T01 | Confirm implementation baseline and tools | — | DONE | clean starting tree at 5ff04d6; g++ available; baseline Nano Every build passed (3707 B flash, 177 B RAM); index source unchanged, metadata one commit behind |
| T02 | Implement channel model and validated effect assignment | T01 | DONE | g++ C++11 with -Wall -Wextra -Werror -pedantic; channel, brightness, invalid assignment assertions passed |
| T03 | Implement steady, linear breathing, and flash evaluation | T02 | DONE | integrated core 2522be8 (lane 64875a2); native core and integrated assertions passed |
| T04 | Implement Sleep, Awake, and Chaos scenes | T03 | DONE | integrated core 2522be8; paired delays, scene restart and full output checks passed |
| T05 | Implement autonomous scheduler and bounded random selections | T04 | DONE | integrated core 2522be8; full deterministic 90/10 partition, ranges, deadlines and rollover passed |
| T06 | Prepare Nano Every outputs, startup, and cooperative loop | T02 contract; final integration after T05 | DONE | integrated hardware 4dffae6 (lane 4042b48); adapter + real-core integration checks and target build passed |
| T07 | Add bounded serial diagnostics | T06 on hardware lane | DONE | integrated hardware 4dffae6; partial/zero TX, English logs, lengths and autonomous progress assertions passed |
| T08 | Integrate lanes, verify software, and update operator docs | T05 and T07 | DONE | G3: native suites and board build pass; integrated read-only review found no concrete bugs; English README and agent guidance updated |
| T09 | Validate the assembled prototype | T08 | AWAITING HARDWARE | physical output, timing, optical, power, and thermal results recorded, or explicitly awaiting hardware |

## Parallel execution with Git worktrees

### Shared checkpoint before branching

T01 and T02 run on the integration branch first. Before opening parallel worktrees, establish a committed checkpoint containing the approved specifications, this plan, repository instructions, shared lighting declarations, validated assignment model, and the initial host test entry point. Inspect the working tree first: worktrees start from commits and do not inherit uncommitted or untracked specifications. Never sweep unrelated user changes into a checkpoint or discard them. If the required checkpoint cannot be created safely, record the blocker before starting dependent lanes.

At T02, freeze only the interfaces needed by both lanes: channel definitions and capabilities, effect/state fields, mode and scene event data, the common-time update/evaluation entry points, bounded random-selection interface, assignment error result, and rendered brightness representation. Agree signatures and ownership before implementation; do not add generic interfaces for hypothetical features. Declarations may precede implementations, but do not put fake production implementations behind them merely to make a partial branch build.

### Lane ownership and dependencies

| Lane / suggested branch | Tasks | Owns | May start |
|---|---|---|---|
| Coordinator / integration branch | T01, T02, T08, T09; central tracking | shared `src/lighting.h`, specifications, this plan, `AGENTS.md`, README, build configuration | immediately |
| Core / `firmware/core` | T03 → T04 → T05 | `src/lighting.cpp`, `tests/lighting_test.cpp` | after committed T02 checkpoint |
| Hardware / `firmware/hardware` | T06 → T07 | `src/main.cpp`, `tests/adapter_test.cpp`, minimal test-only Arduino stub files if needed | after committed T02 checkpoint, concurrently with core |

Core retains the specified compact `lighting.h`/`lighting.cpp` organization. Parallelism does not require splitting the engine, scenes, and scheduler into extra production modules. T03, T04, and T05 share implementation and tests, so keep them sequential in one worktree. T06 and T07 both edit `main.cpp`, so keep them sequential in the other worktree. Maximum useful concurrency is two implementation lanes plus coordination; add workers only for an actual independent need.

The hardware lane can implement startup, GPIO mapping, seed collection, loop wiring against the frozen declarations, and bounded serial output before the core implementation is finished. Its adapter test executable uses test-only controlled lighting results to isolate hardware calls. These test doubles must never enter the production build or be reported as integrated core verification. If the lane needs an unavailable core behavior rather than just its agreed interface, wait for the corresponding core commit instead of guessing or duplicating logic.

### Worktree isolation and coordination

- Create a separate branch and worktree per lane from the same recorded T02 checkpoint. Record actual paths and branch names below; suggested names are not evidence of created resources.
- Place worktrees under a writable directory and keep each lane's build/test outputs local to its worktree. Do not share `.pio` project build directories or generated test executables. Serialize tool operations if a shared PlatformIO cache or GitNexus index cannot safely serve both lanes.
- Bind GitNexus checks to the actual worktree and repository. Do not treat the original checkpoint's graph as current for newly written lane code; confirm unknown or unavailable analysis against source and record limitations.
- Shared headers and configuration have one owner: the coordinator. A lane requesting an interface change pauses affected work, reports the change, and receives the same updated contract commit in both branches before continuing. Unaffected work can proceed.
- Only the coordinator edits the central plan, shared specifications, README, and agent guidance. Workers report task state, commit IDs, changed files, commands/results, blockers, and next actions to the coordinator, who updates the plan promptly. Worker-local check results do not automatically mark central tasks complete.
- Each lane commits only its owned changes after required checks and graph analysis. Preserve unrelated changes; do not use forced cleanup, forced checkout, or destructive reset to resolve worktree problems.

### Integration gates

1. **G0 — shared contract:** T01/T02 checks pass and the common checkpoint is recorded. Start both lanes.
2. **G1 — core ready:** T03–T05 checks pass on the core branch. Integrate its reviewed task commits in dependency order and rerun core assertions.
3. **G2 — hardware ready:** T06/T07 adapter checks pass on their branch. After G1, integrate hardware commits in order. Resolve any shared-contract conflicts deliberately; do not accept a whole side blindly.
4. **G3 — integrated delivery:** run the real core and adapter tests plus the complete `nano_every` build on the combined tree. Confirm startup, transitions, diagnostics, and documentation against the actual implementation. If integration changes either lane's code, rerun the affected checks. Only then finalize T03–T08 as `DONE`.
5. **G4 — prototype:** perform T09 against the integrated firmware; the physical device has one operator at a time. Keep missing hardware evidence explicit.

Parallel branch-local passes are not a substitute for G3. If a lane is delayed, continue independent tasks or documentation preparation; do not merge incomplete behavior just to keep another lane moving. Retain worktrees until integrated results are verified, then remove only clean, fully integrated task worktrees through normal Git worktree management. Never delete an unmerged branch or worktree with uncommitted work as routine cleanup.

### Live worktree register

| Lane | Actual branch / worktree path | Base / task commits | Owner | State |
|---|---|---|---|---|
| Integration | `firmware/implementation`, project root | shared 05fdfe6; core 2522be8; hardware 4dffae6 | coordinator | G3 software verified; T09 pending |
| Core | `firmware/core`; worktree `/tmp/pumpkin-demon-core` removed after verification | base 05fdfe6; core 64875a2 → integrated 2522be8 | core agent | verified and integrated |
| Hardware | `firmware/hardware`; worktree `/tmp/pumpkin-demon-hardware` removed after verification | base 05fdfe6; hardware 4042b48 → integrated 4dffae6 | hardware agent | verified and integrated |

## Task details

### T01 — Baseline and tools

- Check current specifications and working tree before editing; preserve unrelated user changes.
- Check GitNexus repository/index identity and the impact on existing `setup()` and `loop()` before modifying them. The current index may still describe the diagnostic sketch; recheck freshness when implementation starts.
- Locate the native C++ compiler and PlatformIO executable. Run a baseline `pio run -e nano_every`, recording the core/toolchain versions and any existing failure.
- Confirm the planned file boundaries: existing `src/main.cpp`; new `src/lighting.h`, `src/lighting.cpp`, and `tests/lighting_test.cpp`. These new paths are planned artifacts, not existing code.

### T02 — Channel model and assignment validation

- Add fixed group/mode/effect enums, six channel states, pin/capability definitions, and scene constants in the platform-independent lighting module.
- Use 32-bit unsigned timestamps/durations and explicit widening for AVR arithmetic. Keep fixed-size data and avoid runtime heap allocation.
- Validate before replacing an effect: nonzero cycle durations, safe sums/products, ordered brightness bounds, and no white breathing or intermediate brightness. Invalid assignments preserve previous state.
- Freeze the cross-lane contracts and record the shared checkpoint before worktree creation. Add the host assertion executable against the production module. Check percentage conversion: 20% = 51; ember bounds = 26–153; off/full = 0/255.

### T03 — Effect engine

- Implement steady levels, the confirmed integer triangular breathing curve, and fixed on/off flashes with an optional initial delay.
- Derive phases from elapsed time; do not accumulate observed toggle times or use blocking waits.
- Check breathing at 0/1000/2000/3000/4000 ms gives 26/89/153/90/26. Check flash boundaries and delays of 0 and 1500 ms.
- Exercise five concurrent breathing states with white flashes, mixed effects, replacing one channel without resetting others, invalid inputs, and time-counter rollover.

### T04 — Mode scenes

- Apply all six channel states together at a common scene timestamp. Sleep: eyes/mouth 51, embers breathing 26–153 over 4 seconds, whites off. Awake: five PWM groups fully on, whites 100/400 ms flashes immediately.
- Chaos: 100/200 ms flashes; draw four independent 0–1500 ms initial delays for eyes, mouth, embers, and whites. Copy state within each pair, keeping six independent engine channels.
- Restart effects on entry; outputs remain off during initial delays. Equal delays and equal eventual phases are valid. No crossfades or extra ember behavior.
- Assert full scene replacement, paired equality, per-entry delay selection, and no per-pulse redraws.

### T05 — Scheduler and randomness

- Start in Sleep. Select inclusive millisecond durations: Sleep 180000–600000, Awake 5000–10000, Chaos 2000–5000.
- At Sleep expiry, select 0–99: 0–89 Awake, 90–99 Chaos. Return from either episode to a newly timed Sleep; permit repeated Chaos selections separated by Sleep.
- Supply controlled selections in host tests; use the Arduino PRNG adapter only in board integration. Do not add a general-purpose random framework.
- Give transitions priority over expired effects; include initial delays in Chaos duration; never extend an episode to finish a pulse. An overdue transition starts its new scene at the current time.
- Assert range endpoints, selection boundary values, reset behavior, overdue updates, and rollover. Test the 90/10 partition deterministically rather than relying on random samples.

### T06 — Hardware integration preparation (parallel lane)

- Replace the diagnostic sequence with one cooperative loop: read time once, update scheduler, evaluate all channels, apply changed outputs, then service logs.
- Initialize all output latches LOW before output pin configuration. Use digital LOW/HIGH at brightness endpoints and hardware PWM only for intermediate levels on D3/D5/D6/D9/D10. Cache writes, but force the first application. Keep D11 digital.
- Seed once from 16 samples of confirmed-unconnected A0 and timing, producing a nonzero seed; no EEPROM or new hardware. Document that reboot sequences are not guaranteed unique.
- Add a minimal Arduino-call recording stub if needed to verify the actual adapter: correct pins, startup order, endpoint/PWM calls, and re-enabling PWM after a digital endpoint. Keep the stub limited to APIs actually used.
- Validate the hardware adapter against the frozen contracts using a separate test-only stub. Full target linking and flash/RAM inspection wait for the integrated core at G3; do not claim a passing full build from stubbed code. No `delay()`, serial connection wait, custom timers, software PWM, or new runtime dependencies.

### T07 — Serial diagnostics

- Log startup, mode entries, and invalid assignments in English at 115200 baud. Include timestamp and selected duration; include all four delays for Chaos.
- Use bounded storage and incremental transmission limited by available TX capacity. Render outputs first; never wait for capacity or call `Serial.flush()`.
- Check zero transmit capacity, partial writes, complete events, disconnected-monitor behavior at the adapter level, and autonomous loop progress. Avoid per-brightness logs and an unnecessary general-purpose queue.

### T08 — Sequential integration and software delivery

- Integrate core then hardware commits under G1–G3; record commit IDs and resolve conflicts before verification. Run the complete host assertions against production logic and any minimal adapter stub. Rebuild the target after final code changes; record commands, results, and memory usage.
- Cross-check every acceptance scenario in the functional specification and every test row in the technical specification. Classify each as software verified or requiring physical measurement.
- Update the README in English for autonomous behavior, wiring references, build/upload/monitor use, and diagnostic interpretation. Update outdated agent guidance about the current diagnostic sketch and link the firmware specifications and this tracker.
- Review the final diff for scope, blocking operations, unsupported D11 effects, paired timing, random bounds, and numerical overflow. Record unresolved issues instead of claiming completion.
- Report software delivery separately from prototype validation. Do not flash a connected device as an implicit part of a compile check.

### T09 — Prototype validation

- Verify startup/reset, all six physical channels, PWM-to-digital transitions, and independent white/yellow groups on the shared board.
- Observe the three scenes and synchronized pairs. Check white brightness, breathing smoothness, and all four initial delays against serial output and measured signals.
- Measure the agreed ≤5 ms output-evaluation/edge/transition lateness target under normal operation and serial logging; record instrumentation, conditions, and worst observed values. Host timing tests alone do not establish this result.
- Run dimmest and brightest conditions using the electrical specification's current, voltage, temperature, and 30-minute tests. Verify the power bank remains on and the monitor can be disconnected without stopping effects.
- If no physical access or measurement equipment is available, leave this task `AWAITING HARDWARE`, list exactly what remains, and provide steps for the user to collect results. Do not invent measurements.

## Verification record

| Check | Latest result | Evidence / next action |
|---|---|---|
| Existing diagnostic baseline build | PASS | `pio run -e nano_every`: Atmel megaAVR 1.10.0, Arduino core 1.8.8, AVR GCC 7.3.0; flash 3707 B, RAM 177 B |
| Production host assertions | PASS | `bash scripts/test-firmware.sh`: core, isolated adapter, and actual-core integrated adapter pass with C++11 warnings as errors; lane ASan/UBSan pass with unsupported LeakSanitizer disabled |
| Production Nano Every build | PASS | `pio run -e nano_every`: flash 7566 B (15.6%), static RAM 524 B (8.5%), Arduino core 1.8.8 / AVR GCC 7.3.0 |
| Functional acceptance coverage | SOFTWARE PASS / HARDWARE PENDING | effect, scene, scheduler, startup/GPIO, serial and integrated transition assertions pass; physical output and timing remain T09 |
| ≤5 ms physical timing target | NOT MEASURED | T09, with instrumentation and logging active |
| Optical/current/thermal/power-bank checks | NOT PERFORMED | T09 on assembled prototype |
| Integrated code review | PASS | read-only review: no concrete functional, AVR arithmetic, timer-phase, output-adapter, or serial-backpressure bugs found |
| Graph analysis before lane commits | PASS | worktree-specific fresh indexes; core 19 changed symbols / 2 expected flows; hardware 80 symbols / 4 expected flows; both medium risk, no partial/truncated result |

## Progress log

| Event | Result | Next action |
|---|---|---|
| T04 / scheduler checks passed locally | scenes, random bound/order checks, 90/10 partition, durations, repeated Chaos, overdue entry and rollover pass; final core checks in progress | finish core verification |
| T03 branch-local checks passed | breathing sample values, regular/delayed flashes, rollover, concurrency, and independent updates pass | T04 scenes and T05 scheduler |
| Worktree cleanup | both worktrees clean; owned paths matched integrated tree exactly; removed worktrees normally and retained task branches | hardware checklist remains T09 |
| G3 software delivery verified | all native tests and board build passed; review found no concrete bugs; docs aligned; bounded-log drop policy documented | T09 physical validation |
| G1/G2 integration passed | lane core 64875a2 integrated as 2522be8; hardware 4042b48 integrated as 4dffae6; no conflicts; native suites and board build pass | T08 review/docs, T09 physical checks |
| G0 / lanes opened | checkpoint 05fdfe6; core and hardware agents assigned isolated worktrees and disjoint files | core T03–T05 and hardware T06–T07 |
| T02 contract validated | shared lighting.h declarations, setEffect validation, and native assertions passed; GitNexus index refreshed with runner 1.6.12 | commit checkpoint and open worktrees |
| Implementation started | clean tree at 5ff04d6; native compiler and baseline build verified | prepare T02 shared contract |
| Plan created | specifications and confirmed decisions decomposed into T01–T09; no production work started | begin T01 when implementation is requested |
| Worktree parallelism planned | shared T02 contract; parallel core and hardware lanes; single coordinator; ordered integration gates and worktree register added | complete T01/T02 before creating worktrees |

## Completion rule

Software delivery is complete when T01–T08 are `DONE` with passing recorded checks and accurate documentation. Overall validation is complete only when T09 is also `DONE`. If hardware checks remain pending, report that explicitly and keep their status visible in this plan.

## Hardware handoff checklist

T09 is awaiting hardware, not complete. The production image is built locally but has not been uploaded.

- Upload deliberately to the six-channel assembly using the README instructions; monitor at 115200 baud.
- Check reset/startup, all six outputs, pair synchronization, white/yellow separation, and PWM/digital transitions. Observe Sleep for its selected 3–10 minute interval; use the logged duration to know when the next episode is due.
- Capture output edges and mode deadlines with suitable instrumentation. Record maximum measured lateness and output-evaluation interval against the ≤5 ms target, with diagnostics active.
- Confirm smooth breathing, white brightness, and operation without a serial monitor.
- Measure total current, voltage under load, ULN2003A temperature, and power-bank low-load retention using the electrical specification's 30-minute tests.
- Record board/wiring revision, measurement conditions, results, and any adjustments before changing T09 to DONE.

## Execution notes

- T03–T05 were delivered as one core lane commit and T06–T07 as one hardware lane commit because the paired tasks share files; their checks were performed incrementally before integration.
- The supplied plan is a live action tracker, not a generated GitNexus provenance-pack plan. It was intentionally updated throughout execution as requested.
- Initial sandboxed analyzer/Git subprocesses required elevated execution for local cache/index operations. Index refreshes and worktree-specific graph gates subsequently succeeded. Canonical MCP staleness metadata lagged the CLI refresh during startup; current source and fresh worktree indexes were used for the actual commit gates.
- ASan/UBSan passed in both lanes. LeakSanitizer was disabled because the traced execution environment does not support it; production code uses no dynamic allocation.
- Serial diagnostics retain one pending event and may drop later events if TX stays blocked; this limit is documented in code, README, and technical specifications.
