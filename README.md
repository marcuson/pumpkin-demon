# Pumpkin Demon

Autonomous lighting firmware for an Arduino Nano Every inside a real carved pumpkin. The production design controls six electrical groups through an ULN2003A: two eyes, two mouth corners, yellow embers, and white flashes. Each group contains three LEDs; the yellow and white groups share a physical board but remain separate circuits.

## Hardware

| Nano Every | ULN2003A channel | LED group |
|---|---|---|
| D3 | 1 | Left eye, red |
| D5 | 2 | Right eye, red |
| D6 | 3 | Left mouth corner, red |
| D9 | 4 | Right mouth corner, red |
| D10 | 5 | Embers, yellow |
| D11 | 6 | Flashes, white |

The first five channels use hardware PWM for intermediate brightness. D11 is digital only: off, fully on, or flashes. Every LED needs its own resistor. A0 is reserved as an unconnected input for startup random-seed sampling.

Follow the [electronics specification](docs/specs/electronical-specs.md) for the complete power distribution, connector mapping, component limits, and assembly checks. Power the Nano through Micro-USB; branch the LED supply before the board and share ground with the ULN2003A. Leave ULN2003A COM unconnected for these LED loads. Follow the [mechanical specification](docs/specs/mechanical-specs.md) for support, insulation, and protection against pumpkin liquids.

## Autonomous behavior

| Mode | Eyes and mouth | Yellow embers | White LEDs |
|---|---|---|---|
| Sleep | Steady at 20% | Linear breathing, 10–60%, 4-second cycle | Off |
| Awake | Fully on | Fully on | 100 ms on / 400 ms off |
| Chaos | Flashes | Flashes | Flashes |

The firmware starts in Sleep. Each Sleep interval lasts a random **3–10 minutes**. The next episode is Awake with 90% probability, lasting **5–10 seconds**, or Chaos with 10% probability, lasting **2–5 seconds**. Either episode returns immediately to a newly timed Sleep interval. Repeated Chaos selections are allowed, with Sleep between episodes.

In Chaos, flashes are **100 ms on / 200 ms off**. Eyes flash as a synchronized pair, as do mouth corners. Eyes, mouth, embers, and whites each receive an independent random initial delay of **0–1500 ms**, selected once on mode entry. Outputs remain off during that delay. Delays are included in the episode duration; coincident delays are allowed.

Effects restart on every mode entry, with no transition fade. No computer, sensor, or operator input is required. Sleep is a lighting scene rather than microcontroller power-saving sleep. Random choices are decorative pseudo-randomness; a unique sequence after every reset is not guaranteed.

## Build, upload, and serial monitor

Open this folder in PlatformIO, select the `nano_every` environment, and use Build. From a terminal with PlatformIO on the path:

```sh
pio run -e nano_every
```

To install the firmware on the connected Nano Every, close any serial monitor first, disconnect the power bank, and connect the PC to the Nano's Micro-USB socket:

```sh
pio run -e nano_every -t upload
pio device monitor -b 115200
```

Serial diagnostics are in English and report startup and mode changes, including the selected duration and Chaos delays. They do not report every brightness update. Opening a monitor after startup still shows subsequent transitions; reset the board with the monitor open to inspect startup. The lighting sequence runs without the monitor connected. Diagnostics retain one pending event; if transmission remains stalled across mode changes, later events can be dropped rather than blocking the lights or corrupting a partial line.

Logs describe commanded behavior, not measured current or confirmed illumination. Check the selected episode and its duration before interpreting long periods in Sleep as a fault.

## Software checks and prototype validation

The native assertion checks exercise production lighting logic with controlled time and random choices, plus the board adapter using a small Arduino stub. Run all three native test executables (core, isolated adapter, and integrated adapter with the real core) with a C++11 compiler installed:

```sh
bash scripts/test-firmware.sh
```

Set `CXX` to a different compiler executable if needed. See the [implementation plan](docs/plans/firmware-implementation-plan.md) for recorded results.

A successful build and passing host checks do not validate physical wiring, LED brightness, power-bank behavior, temperature, or the **5 ms timing target**. These require measurements on the assembled prototype. In particular, test white brightness with the actual LEDs and ULN2003A voltage drop, verify reset and all six outputs, and perform the current/voltage/thermal and low-load tests described in the electronics specification. Hardware validation status is tracked separately in the plan.

## Specifications

- [Firmware functional specification](docs/specs/firmware-functional-specs.md)
- [Firmware technical specification](docs/specs/firmware-technical-specs.md)
- [Firmware implementation action plan](docs/plans/firmware-implementation-plan.md)
