# Pumpkin Demon — firmware technical specification

**Status:** implementation specification derived from the confirmed functional requirements. Firmware implementation and hardware validation are separate work.  
**Target:** Arduino Nano Every / ATmega4809, PlatformIO environment `nano_every`, Arduino C++.  
**Language:** identifiers, comments, diagnostics, and documentation shall be English.

## 1. References and implementation scope

- [Functional specification](firmware-functional-specs.md): authoritative behavior, parameters, and acceptance scenarios (FR-01 through FR-20).
- [Electronics specification](electronical-specs.md): pin assignments, ULN2003A, power distribution, and electrical validation.
- [Mechanical specification](mechanical-specs.md): 18 LEDs, six electrical groups, and five perfboard assemblies.

Implement the autonomous lighting firmware described by these documents. The existing `src/main.cpp` is a two-channel blocking diagnostic sketch, not the production implementation. Its one-second sequence and Italian log messages shall not be carried into the production behavior.

Keep the existing PlatformIO board/framework configuration. No additional runtime dependency, RTOS, custom interrupt handler, software PWM, timer-register configuration, persistent settings, or serial command protocol is required. Use the Arduino core's existing timing and PWM facilities.

## 2. Channel definitions and output contract

Use a fixed array of six channels, indexed by a small `enum class Group : uint8_t` in this order:

| Group identifier | Pin | Capability | Chaos timing entity |
|---|---|---|---|
| `LeftEye` | D3 | hardware PWM | eyes |
| `RightEye` | D5 | hardware PWM | eyes |
| `LeftMouth` | D6 | hardware PWM | mouth |
| `RightMouth` | D9 | hardware PWM | mouth |
| `Embers` | D10 | hardware PWM | embers |
| `WhiteFlashes` | D11 | digital off/on only | whites |

All outputs are active HIGH at the ULN2003A inputs: LOW turns the group off. Use logical brightness `uint8_t` in the range 0–255.

Define one output function responsible for translating brightness into Arduino calls:

- For 0, use `digitalWrite(pin, LOW)`.
- For 255, use `digitalWrite(pin, HIGH)`.
- For 1–254 on PWM channels, use `analogWrite(pin, brightness)`.
- Reject breathing or intermediate brightness on D11; do not silently convert an unsupported request to a different effect. Validate effect assignments before replacing the current channel state. Invalid assignments leave the previous state unchanged and are exposed by tests and a bounded English diagnostic.

Explicit digital writes at the endpoints avoid unnecessary PWM operation and disable any previously enabled PWM on that pin. Subsequent intermediate writes re-enable hardware PWM through the core. Do not use `digitalRead()` to observe PWM outputs: the installed core disables PWM when digitally reading those pins. Test intended levels through the output boundary and measure actual signals externally when needed.

Percentage conversion uses nearest-integer rounding: `(uint32_t(percent) * 255 + 50) / 100`, with percent constrained to 0–100. Thus Sleep steady brightness is **51**, and ember bounds are **26–153**. Electrical current remains limited by the physical resistors; software duty cycle is not a current measurement.

## 3. Minimal code organization and state

Keep Arduino setup, GPIO writes, random-number calls, and serial handling in `src/main.cpp`. Place the platform-independent state and effect calculations in `src/lighting.h` and `src/lighting.cpp` so host tests can compile the actual production logic without Arduino hardware. Use plain structs and functions rather than virtual interfaces or a general-purpose animation framework.

The logical responsibilities are:

| Responsibility | Data / behavior |
|---|---|
| Mode scheduler | current `Mode`, mode start time, selected mode duration |
| Channel effects | six independent effect descriptions and start timestamps |
| Scene application | assign all six effects from the selected mode, sharing timing within pairs |
| Effect evaluation | calculate one brightness from effect state and current time |
| Hardware adapter | apply the six resulting levels to fixed pins |
| Diagnostics | report startup and scene changes without stalling effects |

Use `enum class Mode : uint8_t { Sleep, Awake, Chaos }` and `enum class Effect : uint8_t { Steady, Breathing, Flashes }`. Off is a steady effect at brightness 0; it does not need a separate effect type.

Each channel stores its effect kind, minimum/maximum brightness, start timestamp, initial delay, and relevant durations. Use 32-bit unsigned milliseconds for timestamps and durations. Fixed-size state is sufficient; do not allocate channel state dynamically or use Arduino `String` in the runtime loop. Cache the last applied brightness per channel to avoid repeated identical GPIO/PWM writes; force the first output application after initialization.

The effect engine permits different parameters and timestamps on all six channels. Scene presets intentionally assign equal state to the eye pair and equal state to the mouth pair; do not merge those channels into one engine object. Engine-level tests may deliberately break scene pairing to verify channel independence.

## 4. Cooperative timing and loop order

Read `millis()` once per loop and pass the same `uint32_t now` to the scheduler and all six effect evaluations. Do not call `delay()`, wait for a serial connection, or wait for an effect to finish.

Each loop performs these operations in order:

1. Check whether the current mode duration has elapsed.
2. If due, select and enter the next mode using `now` as the common scene start timestamp.
3. Evaluate all six effects using the updated scene, then apply changed output levels.
4. Service pending serial output only within the available transmit capacity.

Mode transitions take priority over old effect phases. At a mode deadline, render the new mode rather than one more old pulse. Do not replay missed effect edges after a delayed loop; evaluate the phase corresponding to the current elapsed time. Do not run a burst of historical mode transitions: after an overdue transition, the new mode starts at the current `now`.

Use unsigned elapsed subtraction: `uint32_t elapsed = now - startedAt`. Compare elapsed durations, not absolute expressions such as `now >= startedAt + duration`. The maximum scheduled interval is 600000 ms, far below half the 32-bit time range. Verify elapsed arithmetic across rollover and ensure the loop continues running; no timestamp scheme here supports an unobserved full counter wrap while execution is stopped.

**Confirmed engineering timing target:** no more than 5 ms between output evaluations in normal operation, including serial logging. On/off edge and mode-transition lateness shall be at most 5 ms relative to the millisecond clock, excluding startup and reset. The implementation shall aim to meet this agreed target and measure it during validation; it is not an already measured result. Pair members are calculated from the same time and phase; sequential GPIO writes need not occur on the same CPU instruction.

## 5. Effect evaluation

### 5.1 Steady

Return the assigned brightness on every evaluation. No duration, oscillation, or startup delay applies. White steady output accepts only 0 or 255.

### 5.2 Breathing

Use the confirmed integer linear triangular envelope. Any later curve adjustment will be based on prototype results and an agreed specification update. Each cycle rises from minimum to maximum and falls back without pauses. For the Sleep preset, rise and fall are each 2000 ms.

For `elapsed = now - startedAt`, define `phase = elapsed % (riseMs + fallMs)` and `range = maximum - minimum`:

- When `phase < riseMs`: `minimum + (uint32_t(range) * phase) / riseMs`.
- Otherwise: `maximum - (uint32_t(range) * (phase - riseMs)) / fallMs`.

At entry, output the minimum; at the rise/fall boundary, output the maximum; at the cycle boundary, return to the minimum. Intermediate integer division truncates. Widen multiplication before calculating to avoid AVR 16-bit integer overflow. Reject zero rise/fall durations, inverted brightness bounds, and unsupported channel capabilities before assigning the effect. Bound accepted durations to the project's operating ranges so cycle sums and interpolation products fit 32 bits.

No gamma correction, floating-point trigonometry, lookup table, or additional ember flicker is required. Confirm visual smoothness on the prototype before changing the curve.

### 5.3 Flashes

For elapsed time since effect assignment:

1. If `elapsed < initialDelayMs`, output 0.
2. Otherwise compute `phase = (elapsed - initialDelayMs) % (onMs + offMs)`.
3. Output the illuminated brightness when `phase < onMs`; otherwise output 0.

Reject zero on/off durations, overflowing cycle sums, and unsupported brightness assignments. Flash intensity is 255 in both approved scenes. Awake uses 100 ms on / 400 ms off and no initial delay. Chaos uses 100 ms on / 200 ms off after its initial delay. Equality at the on-duration boundary means off; equality at the initial-delay boundary begins the first on phase.

Effect phases are derived from elapsed time, rather than repeatedly adding a duration to the last observed toggle, to avoid accumulated loop jitter. Restarting an effect replaces its timestamp and all parameters; updating a different channel leaves this state unchanged.

## 6. Scene entry and scheduler

Maintain a single scene-entry operation that assigns the mode, duration, common timestamp, and all six channel states before the next output render. There are no transition fades.

| Scene | Eye pair | Mouth pair | Embers | Whites | Duration |
|---|---|---|---|---|---|
| Sleep | steady 51 | steady 51 | breathing 26–153, 2000 ms rise / 2000 ms fall | steady 0 | random 180000–600000 ms |
| Awake | steady 255 | steady 255 | steady 255 | flashes 255, 100/400 ms, delay 0 | random 5000–10000 ms |
| Chaos | flashes 255, 100/200 ms | flashes 255, 100/200 ms | flashes 255, 100/200 ms | flashes 255, 100/200 ms | random 2000–5000 ms |

At each Chaos entry, draw four independent initial delays from 0–1500 ms inclusive: one for both eyes, one for both mouth corners, one for embers, and one for whites. Copy the same timestamp and delay to both members of each pair. Do not redraw delays per channel within a pair or per flash. Equal random delays are valid; do not redraw to force distinct values. Since the pulse period is 300 ms, different delays may still produce matching phases after both effects have started; this is valid autonomous behavior.

Chaos duration starts at scene entry and includes all initial delays. It is never extended to complete a pulse. With the minimum 2000 ms episode and maximum 1500 ms delay, every entity starts before the episode ends.

At startup, enter Sleep. At the end of Sleep, draw an integer from 0–99: 0–89 selects Awake and 90–99 selects Chaos. After either episode, enter a new Sleep scene with a newly drawn waiting duration. Do not impose repetition limits, cooldowns, escalation, or a guarantee that Chaos occurs within a fixed number of episodes.

## 7. Random selection and reproducibility

Use the Arduino core PRNG through a small bounded selection function; the host tests supply predetermined selections to the scheduler. Interpret time ranges as inclusive integer milliseconds. Arduino `random(min, max)` excludes the upper bound, so the production selections are:

| Purpose | Arduino call |
|---|---|
| Sleep duration | `random(180000L, 600001L)` |
| Awake duration | `random(5000L, 10001L)` |
| Chaos duration | `random(2000L, 5001L)` |
| Episode selection | `random(100L)` |
| Each Chaos delay | `random(1501L)` |

These are ordinary decorative pseudo-random choices, not security-sensitive randomness. Core PRNG range reduction may have negligible statistical bias; the requirement is the intended 90/10 selection split, not cryptographic uniformity.

Seed once at startup. A0 is confirmed unconnected; reserve it as the analog input for startup seed sampling and combine a bounded set of 16 analog samples with `micros()` timing into a nonzero 32-bit seed. Do not reseed at each scene or flash. Any future connection or reassignment of A0 requires revisiting this seed source. Floating-input sampling is a best-effort source of variation and does not guarantee a unique sequence after every reset. No extra sensor, EEPROM write, or persistent boot counter is required.

Tests use controlled seeds or fixed bounded selections and must not depend on analog noise. Choose the random duration on scene entry and the next episode type at Sleep expiry; do not draw values on every loop iteration.

## 8. Startup, diagnostics, and resource handling

Startup order:

1. Write LOW to all six output latches, then configure all six pins as outputs.
2. Initialize serial at 115200 baud; never wait for the monitor to connect.
3. Seed the PRNG using the bounded startup procedure.
4. Enter Sleep and render its initial levels immediately.
5. Report startup and the initial scene, then continue normal cooperative updates.

Log only startup, scene changes, and invalid internal effect assignments. Use elapsed milliseconds from the board clock, English mode names, and selected scene duration. A compact line may be `t=123456 mode=Awake duration_ms=7500`. For Chaos, also record the four selected initial delays so timing can be inspected. Millisecond timestamps wrap with the underlying clock; no persistent wall clock is required.

Keep message formatting and storage bounded. Use fixed buffers or flash string literals instead of dynamic strings. Retain a bounded pending event and drain it in chunks no larger than `Serial.availableForWrite()` allows; render outputs before draining logs. Do not call `Serial.flush()` or wait for transmit space. Startup and its initial Sleep scene may be reported in one combined event. With scenes separated by seconds, a general-purpose log queue is unnecessary. A disconnected PC must not block the light sequence.

Use fixed arrays and compile-time constants for pins and scene parameters. Do not modify core timer prescalers or claim hardware sleep/power savings. Inspect flash/RAM usage in the target build; leave room for stack and serial buffers, and avoid runtime heap allocation.

## 9. Validation and functional traceability

Implement one small host test executable against the production logic using assertions and controlled time/random selections. A native C++ compiler is sufficient; no new test framework is required. Test through a narrow output boundary to record commanded brightness; this does not replace signal or optical measurements.

| Area | Required checks | Functional coverage |
|---|---|---|
| Output capabilities | fixed mapping; percentage rounding; 0/255 endpoints; PWM intermediate levels; reject white breathing/dimming without replacing state | FR-01–03, FR-08 |
| Steady/breathing | 51 steady; breathing at 0, 1000, 2000, 3000, 4000 ms gives 26, 89, 153, 90, 26; invalid bounds/durations rejected | FR-03, FR-10 |
| Flash boundaries | Awake on at 0/99 ms, off at 100/499 ms, on at 500 ms; Chaos delays 0 and 1500 ms and exact on/off boundaries | FR-03, FR-11–12 |
| Concurrency | five different breathing states plus white flashes; mixed effects; changing one channel does not reset the others | FR-04–06 |
| Scene pairing | matching parameters, timestamps, delays, and rendered levels within eye and mouth pairs; coincident delays accepted | FR-07, FR-12 |
| Transitions | startup Sleep; full scene replacement; breathing restarts at minimum; Awake starts on; Chaos starts off until its delay; no fade or leftover state | FR-09–14 |
| Scheduler | all duration endpoints; selections 0/89 choose Awake and 90/99 choose Chaos; each episode returns to a fresh Sleep; repeated Chaos accepted | FR-15–16 |
| Deadline precedence | transition wins over an old flash edge; Chaos delays do not extend an episode; overdue updates do not replay missed pulses | FR-06, FR-13, FR-16 |
| Rollover | mode duration, breathing, initial delay, and pulse phase remain correct when the 32-bit clock wraps | FR-17 |
| Diagnostics | English bounded events with durations/delays; no serial wait; pending output does not block lighting | FR-18–19 |
| Prototype | six physical outputs; endpoint/PWM transitions; measured timing target under logging; smooth breathing; correct white brightness; power-bank/current/thermal tests | FR-19–20 |

The breathing midpoint values above intentionally reflect integer truncation; a one-count slope asymmetry is acceptable. For scheduler statistics, verify the exact 90/10 selection partition deterministically rather than using a flaky finite random run as the sole correctness test.

Build the board target with `pio run -e nano_every`. Compile and run the host assertions separately. Record compiler/core versions from the build and distinguish passing software checks from measurements on the assembled device. Do not report this specification as a tested implementation.

## 10. Documentation and implementation completion

When implementing, keep this document and the functional specification consistent with any agreed design change. Add operator instructions for the production behavior to the README and update `AGENTS.md` references if needed. Do not leave the diagnostic sketch described as the production firmware, or imply hardware validation was completed merely because the build passed.

Completion requires production code, host checks, a successful Nano Every build, and separately recorded prototype results or an explicit statement that those hardware checks remain pending. No firmware source files are changed by creating this technical specification.

## 11. Technical basis

- [Arduino: Blink Without Delay](https://docs.arduino.cc/built-in-examples/digital/BlinkWithoutDelay/): cooperative elapsed-time scheduling.
- [Arduino Nano Every: getting started](https://docs.arduino.cc/tutorials/nano-every/getting-started/): five PWM outputs and no hardware PWM on D11.
- Arduino megaAVR core **1.8.8**, inspected locally: `wiring_analog.c` for output endpoints and PWM activation, `wiring_digital.c` for disabling PWM on digital operations, `WMath.cpp` for random range semantics, and `variants/nona4809/pins_arduino.h` for pin mapping. Platform package observed: Atmel megaAVR **1.10.0**. These versions describe the inspected installation, not a newly pinned dependency.
