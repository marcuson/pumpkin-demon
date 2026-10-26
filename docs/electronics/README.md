# Pumpkin Demon — KiCad wiring schematic

Open **`pumpkin-demon.kicad_pro`** in KiCad, then open the schematic. You can also open `pumpkin-demon.kicad_sch` directly in the Schematic Editor. Created and checked with **KiCad 10.0.7**.

No preliminary library installation is needed: the schematic contains embedded copies of its symbols. Keep `pumpkin-demon.kicad_sym` and `sym-lib-table` alongside the project so editing and library checks can use the same local symbols. No PCB layout is included; footprints are intentionally unassigned because this is an assembly wiring schematic.

## Contents and conventions

- USB power-bank input, 2 A fuse, power switch, protected +5 V distribution, and common ground.
- Arduino Nano Every with actual module header pin numbers and the separate power-only cable to its onboard Micro-USB socket. The symbol's MOSI pin is D11. A0 is intentionally unconnected for startup seed sampling; the +5V and VIN headers are not used as supply inputs.
- ULN2003A DIP-16 with all six used input/output channels and GND connected. COM and channel 7 are marked unconnected.
- All 18 LEDs with individual 220 Ω / 0.25 W resistors. Values remain subject to prototype current and brightness checks, particularly for white LEDs.
- Six separately named cathode returns. The yellow and white groups share a physical lower board but remain separate electrical groups.

Matching net labels are electrically connected even when no long wire is drawn between sections. Each JST-XH connector symbol represents the mated header/housing pair, rather than two additional connectors.

| Schematic reference | Meaning in the specifications |
|---|---|
| J0 | Power bank USB breakout: pin 1 = +5 V, pin 2 = GND |
| J1 | J1A supply connector pair: pin 1 = protected +5 V; pin 2 spare, insulated at both ends |
| J2 | Power-only Micro-USB lead to the Nano onboard socket: pin 1 = VBUS, pin 2 = GND; these are cable terminations, not Nano header pins |
| J3 | J1B return connector pair: pins 1–6 = left eye, right eye, left mouth, right mouth, embers, white flashes |

J1/J3 use numeric references for KiCad annotation; the original J1A/J1B names are retained in their values and notes. The two PWR_FLAG symbols are ERC declarations that power comes from the power bank; they are not physical components.

## Reversed driver-channel routing

Arduino D3, D5, D6, D9, D10, and D11 drive ULN2003A IN6, IN5, IN4, IN3, IN2, and IN1 respectively. Corresponding OUT6 through OUT1 connect to J3 pins 1 through 6. Logical group order and firmware pin assignments are unchanged. See the [routing table](../specs/electronical-specs.md#arduino-to-driver-routing) for physical DIP pin numbers.

## Validation performed

- KiCad 10.0.7 successfully loaded the schematic and exported SVG and XML netlist files.
- ERC completed with **zero errors and zero warnings**, without suppressing schematic violations.
- The exported netlist was checked against the specification: all six Nano-to-ULN mappings, all six separate cathode returns, all 18 resistor-to-anode branches, the fuse/switch supply path, common ground, and intentionally unconnected pins.
- The SVG was rendered and visually inspected for readability and overlapping elements.

These checks establish schematic connectivity and readability. They do not establish real component pin orientation, connector crimp orientation, resistor suitability for the purchased LEDs, brightness, or physical hardware safety. Follow the [electronics specification](../specs/electronical-specs.md) and validate the assembly before use. Disconnect the power bank and its Nano USB lead before connecting the PC USB cable.

## Symbol sources

The local and embedded symbols are derived from the [official KiCad symbol library](https://gitlab.com/kicad/libraries/kicad-symbols). Source symbols: `MCU_Module:Arduino_Nano_Every`, `Transistor_Array:ULN2003A` (flattened from its `ULN2003` base), `Device:R`, `Device:LED`, `Device:Fuse`, `Switch:SW_SPST`, `Connector_Generic:Conn_01x02`, `Connector_Generic:Conn_01x06`, and `power:PWR_FLAG`. They are collected under the local `Pumpkin` library name. The [upstream symbol-library license](SYMBOL-LICENSE.md) is included alongside this project. The symbols were embedded and collected locally; inheritance was flattened for the ULN2003A, and no electrical pin mappings were changed.
