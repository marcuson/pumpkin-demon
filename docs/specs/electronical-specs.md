# Pumpkin Demon — electronics design

Status: assembly schematic for the first version, **lights only**, to be verified on the prototype. The [mechanical design](mechanical-specs.md) defines the support for the pumpkin and mast: the electrical connectors do not carry the mast's weight. The compartment below the platform houses the Nano Every, ULN2003A, power bank, power distribution, and two accessible connectors. Protect the electronics and connectors from pumpkin liquids.

## Architecture and power supply

```text
USB power bank, 5 V (output rated 2 A or more)
  └─ +5 V ─ 2 A fuse ─ switch ─ +5 V bus ─┬─ Micro-USB cable → Nano Every
                                          └─ J1A (2-pin), pin 1 → LED anodes
     GND ───────────────────── GND bus ───┬─ Micro-USB cable GND
                                          └─ ULN2003A GND

Nano Every D3, D5, D6, D9, D10 (PWM) → ULN2003A inputs 1–5
Nano Every D11 (digital, ON/OFF flashes) → ULN2003A input 6
ULN2003A outputs 1–6 → J1B (6-pin), pins 1–6 → cathodes of the six LED groups
J1A pin 2: spare, insulated and unconnected at both ends

Each LED: J1A pin 1 (+5 V) → its own R_LED → LED anode → LED cathode
          → group pin on J1B → ULN2003A output → GND
```

The +5 V supply branches **before** the Nano: LED current does not flow through the Arduino board. The bus, USB cable, and ULN2003A share a common ground. The fuse and switch are upstream of both branches. Power the Nano Every through its **Micro-USB socket**; the manufacturer specifies its `+5V` pin as an **output**, so it must not be used as an input in this circuit. `VIN` is also unsuitable for the power bank's 5 V output. To program from a PC, disconnect the power bank and connect the PC's USB cable to the same socket on the Nano.

The ULN2003A is an array of seven low-side switches: six channels are used and the seventh is spare. Connect its GND pin to the common ground; leave COM unconnected, since the loads here are LEDs rather than inductive loads. **Check the datasheet pinout of the purchased component** before soldering it. ULN2003A inputs accept the Nano Every's 5 V signals; the gate resistors and pull-downs specified for the previous MOSFETs are not needed.

### Removable connectors J1A and J1B

The removable module connects through **two JST-XH connectors already available**: a 2-pin J1A for +5 V and a spare, and a 6-pin J1B for the six LED returns. These are wire-to-board connectors: mount both headers on the base side and both housings with crimped contacts on the mast wiring. Secure the headers to a rigid support and provide cable strain relief; do not rely on solder joints alone to support them. JST-XH pitch is 2.50 mm: check compatibility with the perfboard (usually 2.54 mm), without forcing the pins. If necessary, use a support drilled at 2.50 mm pitch. Keep both connectors in a dry, accessible compartment.

| Connector and pin | Pedestal side | Mast/pumpkin side |
|---|---|---|
| J1A-1 | Protected +5 V | common positive supply for all LEDs |
| J1A-2 | spare, unconnected | spare, unconnected |
| J1B-1 | ULN2003A output 1, driven by D3 PWM | left eye cathodes |
| J1B-2 | ULN2003A output 2, driven by D5 PWM | right eye cathodes |
| J1B-3 | ULN2003A output 3, driven by D6 PWM | left mouth cathodes |
| J1B-4 | ULN2003A output 4, driven by D9 PWM | right mouth cathodes |
| J1B-5 | ULN2003A output 5, driven by D10 PWM | internal ember cathodes |
| J1B-6 | ULN2003A output 6, driven by digital D11 | demonic flash cathodes |

Pin J1A-1 carries the **sum** of the LED currents; select a wire and contact rated for the measured current. Do not join the cathodes of the six groups. The two differently sized housings are not interchangeable, but label them `+5 V` and `LED returns` anyway, and use a multimeter to verify the pin numbering as viewed from the wiring side. Disconnect and reconnect both only with the switch off; the XH series uses a friction lock, so hold the connector bodies rather than pulling on the wires.

### Channels and LEDs

The six groups follow the preliminary distribution in the mechanical design: 2–3 LEDs for each eye and each half of the mouth, about 2 for the embers and 2 for the flashes, giving **12–16 LEDs initially**. Each LED has its own resistor, even when several LEDs in the same group are connected in parallel. The five PWM channels support variable brightness; the sixth on D11 provides on/off flashes. If fading is later required on the sixth channel too, software PWM will be considered in the firmware. At startup, configure all six control pins as LOW outputs before starting the effects.

## Components and limits

| Item | Quantity | Specification / note |
|---|---:|---|
| Arduino Nano Every 5 V / ATmega4809 | 1 | already available; 5 PWM outputs on D3, D5, D6, D9, D10 |
| ULN2003A | 1 | THT DIP-16 version, 6 of 7 channels used |
| Single-color 3/5 mm LEDs | 12–16 initially | electrical design maximum: 6 per channel, 36 total; verify rated current and polarity |
| LED resistors | one per LED | starting point: 220 Ω, 0.25 W, to be checked with the actual LEDs |
| JST-XH J1A and J1B | 2 pairs | one 2-pin and one 6-pin pair, already available; check matching headers, crimped contacts, wire gauge, and rigid mounting |
| Fuse and fuse holder | 1 | 2 A on +5 V immediately after the power bank; size input wires for at least 2 A |
| Switch | 1 | on +5 V, rated at least 2 A at 5 V DC |
| USB power cable / splitter | 1 | +5 V/GND distribution to the bus and Micro-USB cable to the Nano |
| Perfboard, flexible wire, heat-shrink tubing | as needed | insulated solder joints and strain relief |
| USB power bank, 5 V | 1 | already available; recommended output at least 2 A, to be checked on the label |

**LED resistors.** The ULN2003A has a voltage drop when conducting, so the LEDs will be dimmer than with a nearly ideal MOSFET. For a conservative maximum-current check, this drop can be ignored: with `Vmax=5.25 V`, a red/orange LED with `Vf_min=1.8 V`, and R=220 Ω, the result is `(5.25−1.8)/220 ≈ 15.7 mA` per LED. At 5 V, with `Vf=2.0 V` and an indicative 0.9 V drop across the ULN2003A, the current would be about `9.5 mA`. These are examples: test brightness and current with the purchased LEDs before choosing the final resistor value. A 0.25 W resistor is adequate under the stated conservative assumption (about 0.054 W dissipated).

**Total current.** The limit of 6 LEDs per channel × 6 channels × 15.7 mA gives a conservative ceiling of about **0.57 A** for the lights, plus the Nano Every's consumption. The actual value will probably be lower because of the ULN2003A voltage drop, but must be measured with all lights on. Also check the voltage at the Nano and J1A under load, and the ULN2003A temperature after 30 minutes running the most demanding cycle. The ULN2003A datasheet's 500 mA per-channel rating does not mean seven times that current can be drawn without checking chip power dissipation.

**Power bank.** Some models turn off their output at low current. Test the darkest sequence for at least 30 minutes; if the power bank turns off, use its *low current* mode if available, or a model that stays on.

## Assembly checks

1. **On the bench:** read the Nano and ULN2003A markings and the power bank rating; measure USB voltage with no load and under load. Identify the polarity and specifications of the available LEDs.
2. **Distribution:** prepare the USB splitter, fuse, switch, and +5 V/GND buses. With the power bank disconnected, check polarity and the absence of shorts. Power the Nano through its Micro-USB socket, without connecting the bus to the board's `+5V` pin.
3. **Channels:** assemble the ULN2003A and one LED with a resistor for testing. On the five PWM pins, check zero, half, and maximum brightness; on D11, check on/off operation. The LEDs must remain off during reset.
4. **J1A, J1B, and mast:** check continuity pin by pin and the absence of shorts, especially between J1A-1 and J1A-2, before connecting the Arduino. Test the full telescopic travel without pulling or pinching the wires; the connectors do not support the mast.
5. **Final testing:** with all actual LEDs on, measure current and voltages; test for 30 minutes in the most demanding cycle and 30 minutes in the quietest cycle. Switch off before disconnecting J1A or J1B.

## Technical sources

- [Arduino Nano Every, official datasheet](https://docs.arduino.cc/resources/datasheets/ABX00028-datasheet.pdf): USB power, `+5V` as an output, and pinout.
- [Arduino, supported PWM pins](https://support.arduino.cc/hc/en-us/articles/9350537961500-Use-PWM-output-with-Arduino): Nano Every D3, D5, D6, D9, D10.
- [ST, ULN2003A](https://www.st.com/resource/en/datasheet/uln2003.pdf): inputs for 5 V logic, pinout, and voltage drop.
- [JST, XH series](https://www.jst-mfg.com/product/pdf/eng/eXH.pdf): wire-to-board connector, 2.50 mm pitch, and contact specifications.
