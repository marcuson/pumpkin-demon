# Pumpkin Demon — firmware functional specification

**Status:** confirmed functional requirements; all review questions resolved.  
**Scope:** autonomous lighting behavior for the first version. No audio, external sensors, or user control interface is required.

## 1. References and terminology

- [Electronics specification](electronical-specs.md): wiring, power supply, channel mapping, and electrical limits.
- [Mechanical specification](mechanical-specs.md): LED groups, perfboard assemblies, and physical placement.

The three operating modes are **Sleep**, **Awake**, and **Chaos**. Awake replaces “active.” Sleep is the confirmed name for the subdued scene previously called “quiet”; there is no fourth mode.

A **group** is one independently controlled electrical channel, not one LED or one physical board. A **mode** defines the overall scene; an **effect** determines a group's brightness over time. Sleep describes a lighting scene, not a requirement to put the microcontroller into a hardware sleep state.

## 2. Controlled groups

| Group | Output | LEDs | Physical assembly |
|---|---|---|---|
| Left eye | D3 | 3 red, 3 mm | left eye board |
| Right eye | D5 | 3 red, 3 mm | right eye board |
| Left mouth corner | D6 | 3 red, 3 mm | left mouth board |
| Right mouth corner | D9 | 3 red, 3 mm | right mouth board |
| Embers | D10 | 3 yellow, 5 mm | shared lower board, facing upward |
| White flashes | D11 | 3 white, 5 mm | shared lower board, facing upward |

**FR-01.** Control all six groups independently. The three LEDs within each group share the same effect and brightness; individual LED control is outside scope.

**FR-02.** Keep the yellow and white groups electrically and logically independent despite their shared board. The hardware has 18 LEDs across five assemblies.

## 3. Effects and concurrency

**FR-03.** The five PWM groups shall support steady, breathing, and flashes. The white group on D11 shall support steady off, steady fully on, and flashes only; breathing and intermediate brightness are explicitly excluded for this group.

| Effect | Required behavior | Parameters |
|---|---|---|
| Steady | Hold a constant brightness, including off and full brightness | brightness |
| Breathing | Repeatedly and smoothly increase and decrease brightness between a minimum and maximum | minimum, maximum, rise time, fall time; no pauses; confirmed linear rise and fall; any future curve adjustment will follow prototype evaluation |
| Flashes | Repeatedly switch between off and an illuminated level | illuminated brightness, on duration, off duration; fixed durations; Chaos uses independent initial delays as defined below |

**FR-04.** A group executes one effect at a time. All six groups shall execute any combination of their supported effects concurrently, including five simultaneous breathing effects with the whites flashing, or six simultaneous flash effects.

**FR-05.** Groups shall have independent effect parameters and progress. Updating one group's effect shall not stop, restart, or change another group's effect. A mode transition may intentionally update all groups together.

**FR-06.** Effects shall progress without waiting for another group's cycle to finish. Scheduling mode changes and serial reporting shall not visibly freeze breathing or stretch flashes. The implementation must not use blocking effect sequences that serialize the groups.

**FR-07.** Independence does not prevent intentional synchronization. The two eyes shall always have matching behavior, and the two mouth corners shall likewise match. In Sleep and Awake this requires equal brightness within each pair. In Chaos, synchronize both eyes as one pair and both mouth corners as another pair. Eyes, mouth, embers, and whites are four autonomous timing entities; this scene-level synchronization does not remove individual channel capabilities.

**FR-08.** Brightness shall be expressible from off to full output, with intermediate levels for the five PWM groups; D11 supports off and full output only. Full output means the maximum allowed by the validated electrical design, without increasing LED current beyond its hardware limits. A brightness percentage denotes a commanded output level, not a guaranteed percentage of perceived brightness.

### Confirmed hardware scope

Preserve hardware PWM on D3, D5, D6, D9, and D10, and digital on/off control on D11. The white-group exception in FR-03 removes the need for software PWM or a hardware revision. All six groups remain independently controlled within their supported effects.

## 4. Mode scenes

**FR-09.** Exactly one operating mode shall be active at a time. Its scene assigns an effect to every group.

| Group | Sleep | Awake | Chaos |
|---|---|---|---|
| Left eye | steady, reduced brightness | steady, full brightness | flashes |
| Right eye | steady, reduced brightness | steady, full brightness | flashes |
| Left mouth corner | steady, reduced brightness | steady, full brightness | flashes |
| Right mouth corner | steady, reduced brightness | steady, full brightness | flashes |
| Embers | breathing | steady, full brightness | flashes |
| White flashes | off | flashes | flashes |

**FR-10.** Sleep shall present a subdued scene: dim steady eyes and mouth, with breathing embers.

**FR-11.** Awake shall hold eyes, mouth, and embers at full brightness while the white group flashes.

**FR-12.** Chaos shall make every group flash. Groups must be capable of flashing concurrently; use four autonomous timing entities: the eye pair, the mouth pair, embers, and whites. On each Chaos entry, independently select one random initial delay from 0 to 1500 ms inclusive for each entity. Hold that entity off during its delay, then repeat 100 ms fully on / 200 ms off. Both channels of a pair share the same delay and pulse timing. Select delays once per Chaos entry, not once per pulse; pulse and gap durations remain fixed. Independent selections may coincide and do not require distinct delays.

**FR-13.** Entering a mode shall replace the previous mode's scene for every group. No previous flash sequence or breathing cycle may continue accidentally after the transition. Switch scenes immediately, without transition fades. Restart every assigned effect from its beginning on mode entry: breathing begins at its minimum and rises; Awake flashes begin with their on phase immediately; Chaos entities begin their on phase after their newly selected initial delays. Chaos duration is measured from mode entry, includes these delays, and is not extended to finish a delay or pulse.

## 5. Autonomous sequence

**FR-14.** Run automatically after power-up without a computer or operator input. Initialize all six control outputs LOW before starting effects, then enter Sleep.

**FR-15.** Spend most operating time in Sleep. Occasionally enter Awake for a few seconds; enter Chaos less frequently than Awake, also for a few seconds.

**FR-16.** Each Awake or Chaos episode shall have a finite duration. Sequencing is `Sleep → Awake → Sleep` or `Sleep → Chaos → Sleep`, with a new Sleep interval between episodes. There is no direct Awake-to-Chaos escalation.

**FR-17.** Mode selection and effect timing shall continue reliably during prolonged operation, including across internal time-counter rollover. A reset starts a new sequence; resuming a previous episode after power loss is not required.

### Confirmed operating values

These settings are confirmed functional requirements; optical performance remains to be checked on hardware. Timing ranges refer to elapsed time, and brightness values refer to commanded output.

| Parameter | Value |
|---|---|
| Sleep eyes and mouth brightness | 20% |
| Sleep ember breathing range | 10–60% |
| Sleep ember breathing cycle | 4 seconds: 2 rising, 2 falling, no pauses |
| Sleep white group | off |
| Sleep interval before an episode | randomly selected between 3 and 10 minutes (180–600 seconds) |
| Episode selection after each Sleep interval | 90% Awake, 10% Chaos |
| Awake duration | randomly selected between 5 and 10 seconds |
| Chaos duration | randomly selected between 2 and 5 seconds |
| Awake white flashes | full brightness, 100 ms on / 400 ms off |
| Chaos flashes | full brightness, 100 ms on / 200 ms off; eyes paired, mouth paired, embers and whites independent |
| Chaos initial delays | independently random 0–1500 ms inclusive for eyes, mouth, embers, and whites; outputs off until their delay expires |
| Return after either episode | Sleep, with a newly selected waiting interval |

This schedule makes Sleep the dominant mode and Chaos rarer on average. Probabilities do not guarantee a maximum wait before Chaos or prevent consecutive Chaos episodes separated by Sleep. No additional constraint on Chaos repetitions, minimum spacing, or maximum wait is required.

## 6. Diagnostics and hardware validation

**FR-18.** Provide English serial messages for startup and mode transitions, including elapsed time, mode name, and the selected episode or waiting duration. Do not log every brightness update. Serial output must not be required for autonomous operation; use the baud rate configured in `platformio.ini` (currently 115200).

**FR-19.** Diagnostic output reports intended commands, not sensed current or confirmed illumination. No electrical feedback sensors are specified. Verify visible effects and white-LED brightness on the actual hardware separately from firmware tests.

**FR-20.** Validate operation with the power bank during the dimmest scene as well as the brightest scene. Software behavior alone does not establish that the power bank will remain on under low load or that the electrical design is within its limits.

## 7. Acceptance scenarios

| Scenario | Expected result |
|---|---|
| Startup/reset | outputs initialize off, then Sleep begins; no unintended startup flash caused by firmware |
| Each effect on each group | steady, breathing, and flashes work on each PWM group; D11 supports only off, full steady output, and flashes |
| Mixed effects | steady left eye, breathing right eye, flashing left mouth, steady right mouth, breathing embers, and flashing whites run concurrently |
| Six simultaneous effects | verify five groups breathing while whites flash, then all six flashing, without lost timing or stalled channels |
| Paired scene behavior | left/right eyes match and left/right mouth corners match in every mode, including shared initial delays and flash timing in Chaos |
| Independent update | replacing one group's effect leaves the other five groups' progress unchanged |
| Sleep scene | eyes and mouth stay dim and steady; embers breathe; whites remain off |
| Awake scene | eyes, mouth, and embers stay fully on while whites flash |
| Chaos scene | four entities select independent 0–1500 ms delays, remain off during their delay, then flash 100 ms on / 200 ms off; paired channels remain synchronized |
| Chaos delay boundaries | exercise 0 ms and 1500 ms delays, including coincident delays; all entities begin flashing before the minimum 2-second episode ends, and delayed starts do not extend the episode |
| Transition during an effect | the new scene takes over immediately, with effects restarted from the beginning and no old effects left running |
| Episode completion | Awake and Chaos end after their selected durations and return immediately to Sleep with a new random 3–10 minute interval |
| Scheduler | selected intervals and durations remain within agreed bounds; controlled test choices exercise both episode types; statistical checks establish that Chaos is selected less often than Awake |
| Long operation | scheduling and effects continue across time-counter rollover; unplugging the serial monitor does not stop operation |
| Physical prototype | confirm optical results, supply stability, and thermal/current checks defined in the electronics specification |

Use controlled time and selection inputs for repeatable firmware tests; physical tests are still needed for actual illumination. Check delay bounds, fixed pulse durations, paired outputs, and mode deadlines with controlled time; verify visible smoothness and brightness on the prototype.

## 8. Resolved review decisions

### Resolved questions

| ID | Confirmed decision |
|---|---|
| Q-01 | Use Sleep, Awake, and Chaos. The subdued mode is Sleep. |
| Q-02 | White LEDs remain off during Sleep. |
| Q-03 | Brightness and breathing settings are approved. Both eyes must match; both mouth corners must match. |
| Q-04 | Use random 3–10 minute Sleep intervals, 90% Awake / 10% Chaos selection, and the approved episode durations. No additional constraints on repeated Chaos selection are needed. |
| Q-05 | Awake flashes retain the approved settings. Chaos uses full brightness and fixed 100 ms on / 200 ms off timing, with paired channels and independent initial delays as confirmed below. |
| Q-06 | Switch modes immediately and restart effects from the beginning. Keep startup in Sleep and return to Sleep after either episode. |
| Q-07 | White LEDs do not require breathing or dimming. Keep D11 digital; no software PWM or hardware change is required. |
| Q-08 | Discard the incomplete ember request: it was a typing error. No extra ember behavior is required. |
| Q-09 | Synchronize the two eyes with each other and the two mouth corners with each other. Eyes, mouth, embers, and whites run autonomously from one another. |
| Q-10 | Choose independent random initial delays between 0 and 1500 ms inclusive for the four Chaos entities. Keep subsequent pulse durations fixed at 100 ms on / 200 ms off; do not randomize each pulse or gap. |

All review questions are resolved. The incomplete ember request was discarded and introduces no additional behavior.

## 9. Implementation boundary

This document specifies production behavior. Software delivery and physical validation are tracked separately in the [implementation plan](../plans/firmware-implementation-plan.md); completed software checks do not establish hardware validation. The original two-channel diagnostic sequence is outside the production scenes. No persistent settings, runtime configuration protocol, audio, sensors, or individual LED addressing is required by this draft.
