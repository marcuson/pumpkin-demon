# Pumpkin Demon — mechanical design

**Status:** agreed preliminary design  
**Date:** October 18, 2026  
**Scope:** structure, fasteners, physical wiring, and assembly. The electrical schematic and firmware will be documented separately.

## 1. Goals and constraints

The decoration uses a **real carved pumpkin** and an **Arduino Nano Every** controlling lighting effects; audio is deferred to a later stage. The pumpkin is consumable; the support structure, LEDs, and electronics must be recoverable and reusable in subsequent years.

Mechanical requirements:

- No electronic board in contact with pumpkin flesh or liquids.
- **Wooden support platform** with a central hole for the lighting mast.
- **Separate pedestal/electronics compartment**, located below the wooden platform; the pumpkin's weight must rest on the platform and its supports, rather than on the electronics enclosure or mast.
- **Telescopic lighting mast**, adjustable in height and reusable with pumpkins of different sizes.
- Common single-color LEDs (mainly **3 mm**, some **5 mm**) mounted on adjustable arms.
- Mast complete with LEDs and wiring **removable as a module**, with **two JST-XH connectors** near its base.
- Construction **without relying on 3D printing**, using hardware-store materials and simple fabrication methods.
- Operation from a **USB power bank**; automatic sequences, with no external sensors in the first version.

## 2. Physical architecture

```text
                 REAL CARVED PUMPKIN
               (lit eyes and mouth)
                         │
                  adjustable arms
                and LED perfboard modules
                         │
                  TELESCOPIC MAST
                 (attached wiring)
                         │
               protected central hole
     ┌─────────────────────────────────┐
     │ WOODEN PLATFORM + MOISTURE       │
     │ BARRIER / TRAY                   │
     └─────────────────────────────────┘
       │                 │           │
      feet           mechanical     feet
                       socket
                         │
                6 + 2-pin connectors
                         │
          ┌───────────────────────────┐
          │ ELECTRONICS ENCLOSURE     │
          │ Nano Every + ULN2003A     │
          │ power bank                │
          └───────────────────────────┘
```

**Essential separation:** the mast socket mechanically supports the mast; **the electrical connectors do not bear loads**.

## 3. Wooden base

### Agreed choice

- Rigid platform, initially **square**, with a **central hole** through which the mast passes.
- Four feet or supports leaving room underneath for the electronics enclosure.
- Electronics enclosure attached or placed below the platform, in an area protected from pumpkin leaks.
- Washable/waterproof top layer (e.g. a tray or rigid plastic sheet), with a **raised edge and protected central opening** to prevent liquid ingress.
- The pumpkin rests on the platform and must not rest on the mast or electronics enclosure.

### Preliminary dimensions, NOT final

| Item | Initial guideline | To be checked |
|---|---:|---|
| Wooden platform | about **35 × 35 cm** | diameter, weight, and stability of the selected pumpkin |
| Platform thickness | **15–18 mm** | available material and stiffness |
| Feet | **6–8 cm** | actual space required by enclosure, connector, and cables |
| Central hole | about **20–25 mm** | actual diameter of mast and socket |

Adjust the dimensions only after choosing the pumpkin and tubes. Size the hole for the system's **actual overall dimensions**, rather than solely for the nominal diameter stated by the supplier.

### Moisture protection

A real pumpkin can produce condensation and liquids. Protection must prevent them from reaching the enclosure, connectors, electronics, and power bank:

- washable tray or barrier above the wood;
- waterproofing of the wood if needed, especially around the hole and edges;
- sleeve/raised rim around the central opening;
- cable routing that does not direct drips toward the enclosure (a drip loop where useful);
- no real flame inside the pumpkin.

**To be checked during testing:** a tray with a simple hole does not, on its own, provide a liquid seal.

## 4. Telescopic lighting mast

### Principle

Two concentric tubes: the **outer tube is fixed** relative to the base, and the **inner tube slides**. A collar or screw/knob clamp locks the desired height; a stop prevents accidental removal of the inner tube.

- Prefer **rigid PVC** tubes or another lightweight insulating material.
- Use aluminum only if beneficial, taking care to insulate conductors and terminals.
- Initial example: nominal **20 mm outer tube / 16 mm inner tube**, **not yet verified**: check the *actual inside diameter of the larger tube* and *outside diameter of the smaller tube*, tolerances, and sliding fit.
- Proposed useful height range: **about 20–40 cm above the platform**, to be validated against tube length, minimum overlap, and stability.
- Markings on the mast (e.g. every 2 cm) are optional but useful for reproducing a setting.

### Telescopic locking

- Adjustable collar/clamp with a knob screw, or a commercially available telescopic fitting.
- Insert a small **plastic/rubber pad** if a screw presses against the tube, to avoid crushing it.
- Ensure sufficient tube overlap even at maximum extension.
- Add a **travel stop**; never rely on the cables to prevent the tubes from pulling apart.

### Base socket

The mast must fit into a mechanical support/collar below the platform and be removable, preferably without tools (for example, a fitting with a locking knob). The electrical connectors remain **separate** from the socket.

## 5. Adjustable arms

### Agreed choice

- About **three levels** along the mast: eyes, mouth, and internal lights/embers/flashes.
- **Sliding pipe collars** on the mast to set each level's height.
- **Bendable aluminum wire about 2–3 mm thick** for the arms, with initial trial lengths of about **10–12 cm**.
- Each arm attaches under the collar, with a tab or non-slip rubber strip if needed.
- All metal ends must be rounded or covered and kept away from LED contacts.

Available adjustments:

1. overall height through the telescopic mechanism;
2. individual group height through its collar;
3. distance and direction of each light by bending its arm;
4. orientation of the LED mounting assembly relative to the arm.

Do not attach the arms to the pumpkin flesh: **the mast must fully support them**.

## 6. LED perfboard modules

### Selected solution

- Solder the LEDs and their individual current-limiting resistors directly onto **perfboard modules**.
- Use **five perfboard assemblies for six independent electrical groups**: one board for each eye and mouth corner, plus one shared lower board for the yellow and white LEDs. Attach the boards to the mast/arms with insulating supports and, where useful, a small M3 screw with a self-locking nut for tilt adjustment. Keep mounting holes clear of copper pads and wiring.
- Support the shared lower board just above the pumpkin floor, with all six LEDs pointing upward. Do not let the board or its wiring rest on the pumpkin flesh or in liquid.
- Insulate the solder side and exposed leads; keep boards and connections away from pumpkin flesh and liquids. Provide strain relief at each module; LED leads and solder joints must not support the board.
- Available LED colors are **red, yellow, blue, green, and white**. The initial build uses red, yellow, and white only.

### Selected distribution

All distances below are **approximate, center-to-center targets**; use the nearest practical positions on the existing perfboard grid. There are **18 LEDs total: 12 red 3 mm, 3 yellow 5 mm, and 3 white 5 mm**, with one resistor per LED.

| Area | LEDs per electrical group | Layout | Adjustment |
|---|---|---|---|
| Left eye | 3 red 3 mm LEDs | approximately equilateral triangle, about 1 cm sides | upper collar, left arm |
| Right eye | 3 red 3 mm LEDs | approximately equilateral triangle, about 1 cm sides | upper collar, right arm |
| Left mouth corner | 3 red 3 mm LEDs | straight line, about 1 cm between adjacent LEDs (about 2 cm between outer centers) | middle collar, left arm |
| Right mouth corner | 3 red 3 mm LEDs | straight line, about 1 cm between adjacent LEDs (about 2 cm between outer centers) | middle collar, right arm |
| Internal embers | 3 yellow 5 mm LEDs | alternating vertices of the shared six-LED hexagon; approximately equilateral triangle, about 3 cm sides | shared lower board, facing upward |
| Demonic flashes | 3 white 5 mm LEDs | the other three vertices of the same hexagon; approximately equilateral triangle, about 3 cm sides | shared lower board, facing upward |

The eye triangles have a nominal height of about **0.87 cm**. On the lower board, arrange the six LEDs around an approximately regular hexagon in the order **yellow–white–yellow–white–yellow–white**. With each same-color triangle about **3 cm per side**, the hexagon sides are about **1.7 cm**. Keep yellow and white as separate electrical groups with separate return wires; each LED retains its own resistor. Check clearance for the lower board, LED bodies, resistors, mast, and telescopic travel before cutting it.

The [LED placement illustration](../img/pumpkin-led-layout.png) shows the intended front view, bendable side arms, and upward orientation of the lower LEDs; it is not dimensioned.

Use the existing 2.54 mm perfboard holes and accept the resulting spacing and shape approximations; no additional holes are needed to match the nominal LED layout. For reference, four grid intervals give 10.16 mm and twelve give 30.48 mm along a row. Check the actual placement before soldering, without forcing the LED leads.

Final aiming and any diffusers depend on the carving and lighting results. Ideally, the LEDs should illuminate the openings without being directly visible. Test the white flash brightness and the yellow ember effect inside the pumpkin before final assembly.

## 7. Mast wiring and removable connection

- Wiring attached to the mast and secured to the arms with small cable ties.
- **Two JST-XH connectors**, one 6-pin and one 2-pin, near the **base of the mast**, accessible from below the platform. Secure the pedestal-side headers to a rigid support; the wired housings stay with the mast.
- Planned connection: **six ULN2003A-controlled returns on the 6-pin connector; common +5 V and one spare contact on the 2-pin connector**. The schematic is in the electronics document.
- LED current-limiting resistors are part of the individual lighting branches/modules.
- The +5 V contact and its wire must be rated for the total current; protect both connectors from moisture and impacts.
- Secure the cable to the mast with **strain relief**: never allow a tug to stress solder joints or LED modules.
- At the telescopic section, provide a **flexible cable loop** that accommodates the full travel without tension or entering between the sliding surfaces.
- One practical option is to route cables outside the tubes and organize them with reusable cable ties.

**Intended disassembly:** disconnect both connectors, loosen the mechanical socket, and remove **mast + arms + LEDs + wiring** as a single module.

## 8. Planned electronics in the lower compartment (for space planning only)

- Arduino Nano Every powered through its Micro-USB socket.
- Six lighting channels through a **THT ULN2003A** in the pedestal: five PWM channels and one digital channel for flashes.
- USB power bank, 5 V, already available.
- Audio deferred: if possible, leave space for a future speaker and its opening.
- Adequate 5 V and ground distribution, with protected connections.

The electronics must remain accessible for servicing and programming without dismantling the entire decoration. The final circuit schematic will be covered separately.

## 9. Preliminary materials — mechanical parts

| Material | Initial quantity | Notes |
|---|---:|---|
| 15–18 mm plywood | 1 platform ~35 × 35 cm | adapt to the pumpkin |
| Feet/supports | 4 | initial height 6–8 cm |
| Waterproof tray/rigid sheet | 1 | protected central opening |
| Concentric PVC tubes | 2 lengths | check fit and travel |
| Platform mounting socket/fitting | 1 | separate from the electrical connector |
| Telescopic clamp with knob | 1 | with travel stop |
| Sliding collars for groups | ~4–6 | diameter selected for the actual tubes |
| 2–3 mm aluminum wire | ~2 m | for adjustable arms |
| Perfboard for LED assemblies | 5 pieces | four red boards and one shared six-LED lower board; size for resistors and mounting margins |
| Insulating supports / solder-side covers | 5 sets | attach and protect the LED assemblies |
| M3 screws + self-locking nuts | a few | for adjustable mounting assemblies |
| Cable ties, sleeving, non-slip rubber | assortment | attachment/protection |
| JST-XH 6-pin and 2-pin mating pairs | 1 of each type | already available; check matching headers, mounting, and wire gauge |
| Flexible insulated wires | several meters | lengths determined on the prototype |
| Plastic electronics enclosure | 1 | size for power bank and lighting circuit; leave room for future audio if possible |

Quantities are **estimates for preparing the prototype**, not yet a final purchasing list.

## 10. Recommended construction sequence

1. **Gather/measure materials**: actual diameters of both tubes, collars, perfboard, LEDs, and power bank dimensions.
2. **Bench-test the telescopic mechanism**: sliding, locking, travel stop, and stability at maximum extension.
3. **Install collars and arms**, checking that they are sufficiently rigid and can be aimed without drifting out of position.
4. **Prepare the five perfboard assemblies**; mark the selected LED layouts, including the alternating lower hexagon, check spacing and mounting clearance, solder each LED with its own resistor, and insulate the solder side and leads.
5. **Build the mast wiring**, including the telescopic loop, fasteners, and both connectors.
6. **Prepare the wooden platform**, feet, hole, and socket, keeping the pumpkin's load separate from the electronics enclosure.
7. **Add liquid protection**, checking it with the structure powered off and no electronics exposed.
8. **Bench-test the LEDs**, then the entire driver system.
9. **Once the pumpkin is available**, adjust height, arms, and LED-module direction; check that nothing touches the flesh.
10. **Test complete disassembly and reassembly**: two connectors, one mechanical socket, recoverable mast.

## 11. Confirmed decisions and open questions

### Confirmed

- External **wooden support platform with a central hole**.
- Electronics compartment **below the platform**, isolated from liquids.
- Two-section **telescopic mast**, with locking and adjustable LED groups.
- Sliding collars + bendable arms + adjustable perfboard modules.
- **LEDs and individual resistors soldered onto five perfboard assemblies for six independent groups**, using the layouts in section 6.
- Wiring attached to the mast; **two JST-XH connectors, 6-pin and 2-pin**, near the base.
- Mechanical mast support **independent** of the connectors.
- No 3D printing required for the first version.

### Still to be decided/checked

- Pumpkin size and weight; final platform dimensions.
- Actual fit of the two tubes, lengths, travel, and clamp type.
- Exact dimensions of the hole, waterproof sleeve, and mounting socket.
- Final perfboard outlines, mounting holes, insulation, and achievable LED placement.
- LED-module aiming, brightness, and any diffusers; the initial count is three LEDs per group.
- JST-XH contact ratings, wire gauge, header mounting, and moisture protection.
- Actual electronics enclosure dimensions and room for a possible future speaker.

---

**Design note:** this document describes the **agreed mechanical design**; for LED, ULN2003A, and Nano Every connections, follow the electrical schematic and verify it on the prototype. During assembly, the priority is to keep all powered components dry, insulated, and separate from the pumpkin.
