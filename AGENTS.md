# Project instructions

## Language

- Write all code and documentation in **English**, regardless of the language used in prompts. This includes identifiers, comments, serial logs, error messages, README files, specifications, and agent instructions.
- Conversation replies may use the user's language. When editing existing non-English code or documentation, write new or modified text in English; avoid unrelated translation changes unless requested.

## Scope and source of truth

- This is a PlatformIO / Arduino Nano Every (ATmega4809) project for reusable lighting inside a real carved pumpkin. The first version covers automatic lighting only; audio and external sensors are deferred.
- Read [electronics specifications](docs/specs/electronical-specs.md) and [mechanical specifications](docs/specs/mechanical-specs.md) before changing related behavior, wiring, or assembly instructions. These documents contain the detailed design; keep affected documentation consistent with implementation changes.
- Distinguish the current two-channel diagnostic firmware in `src/main.cpp` from the planned six-channel design. Do not expand the diagnostic firmware into the full effects system unless requested.

## Hardware constraints

- Preserve the planned channel mapping: D3 = left eye, D5 = right eye, D6 = left mouth corner, D9 = right mouth corner, D10 = yellow embers (all PWM); D11 = white flashes (digital on/off). Do not assume D11 supports hardware PWM.
- Drive the LED groups through the ULN2003A. Initialize control outputs LOW before starting effects; keep a common ground and leave COM unconnected for these non-inductive loads.
- Power the Nano through Micro-USB. Branch the LED supply before the board; do not route LED current through the Nano or use its `+5V` pin as a supply input. Preserve the specified fuse and switch.
- Each LED requires its own current-limiting resistor. Resistor values and white-LED brightness must be checked with the actual LEDs, supply voltage, and ULN2003A voltage drop; example calculations are not measured results.
- The initial design uses 18 LEDs in six independent electrical groups on five perfboard assemblies: two red eye boards, two red mouth-corner boards, and one shared lower board with separate yellow and white groups facing upward. Preserve separate group returns even on the shared board.
- LED positions are approximate and follow the existing perfboard grid. Refer to the mechanical specification for layouts rather than requiring exact spacing or extra layout holes.
- The mast and platform carry mechanical loads; electrical connectors do not. Keep electronics and solder joints insulated and clear of pumpkin flesh and liquids, with cable strain relief and slack for telescopic travel.

## Implementation tracking

- Follow the [firmware implementation action plan](docs/plans/firmware-implementation-plan.md). Update task status, verification evidence, blockers, and next actions during implementation and before handing work back to the user.
- Keep software completion separate from physical prototype validation; never mark unperformed hardware checks as passed.

## Validation

- For firmware changes, build the `nano_every` environment with `pio run -e nano_every`. Serial monitor speed is configured in `platformio.ini` (currently 115200 baud); keep it consistent with firmware logging.
- Keep changes small and reuse existing project patterns. Do not add dependencies or abstractions without a concrete need.
- Compilation and serial logs do not establish that the physical LEDs work. Report build results separately from hardware checks, and do not claim a hardware fault or a confirmed fix without evidence.
- For documentation-only changes, check affected links, tables, quantities, and consistency with the specifications; no firmware build is required.

<!-- gitnexus:start -->
# GitNexus — Code Intelligence

This project is indexed by GitNexus as **pumpkin-demon** (46 symbols, 43 relationships, 0 execution flows).

> Index stale? Run `node .gitnexus/run.cjs analyze --index-only` from the project root — it auto-selects an available runner. No `.gitnexus/run.cjs` yet? Bootstrap with `npx`, `bunx`, or `pnpm dlx` — e.g. `bunx gitnexus@latest analyze` (npm 11 npx crash; #1939).

## Always Do

- **MUST run impact before editing.** Use `impact({target: "symbolName", direction: "upstream"})` or `node .gitnexus/run.cjs impact "symbolName" --direction upstream --repo .`; report callers, processes, and risk. Never substitute grep for graph analysis.
- **MUST analyze graph changes before committing.** Use `detect_changes({scope: "all"})` (MCP) or `node .gitnexus/run.cjs detect-changes --scope all --repo .` (CLI fallback). `partial: true` or `truncated: true` is not a clean check — a zero means unseen, not unaffected; re-run it. For regression review: `detect_changes({scope: "compare", base_ref: "main"})` or `node .gitnexus/run.cjs detect-changes --scope compare --base-ref "main" --repo .`.
- MUST warn on HIGH/CRITICAL `risk` pre-edit; never use `riskSharedAxes` to waive a HIGH/CRITICAL `risk` warning. Compare File/symbol: MCP File omits axes; Graph-RAG expands File.
- **MUST treat `risk: UNKNOWN` as unresolved, not as low.** An empty caller set is not evidence the symbol is unused — it can also mean the callers are not resolvable by the index (plain-object property access, dynamic dispatch, cross-language calls). `impact` pairs `UNKNOWN` with a `riskNote` saying so. Confirm with a text search before treating the symbol as safe to change or delete; do not proceed on the strength of a zero.
- **MUST use `query({search_query: "concept"})` for concepts/flows, `context({name: "symbolName"})` for a named symbol, or `impact` for blast radius, on read-only callers, dependencies, imports, or execution flow.** Graph first; text search only for empty/`UNKNOWN`/literals.
- For security review, `explain({target: "fileOrSymbol"})` lists taint findings (source→sink flows; needs `analyze --pdg`).

## Never Do

- NEVER edit a function, class, or method before MCP/CLI impact analysis.
- NEVER ignore HIGH or CRITICAL risk warnings from impact analysis, and never read `UNKNOWN` as an all-clear — it means the walk could not answer, which is the one verdict that requires confirming by other means.
- NEVER rename symbols with find-and-replace — use `rename` which understands the call graph.
- NEVER commit before MCP/CLI graph change analysis.

## Resources

| Resource | Use for |
| --- | --- |
| `gitnexus://repo/pumpkin-demon/context` | Codebase overview, check index freshness |
| `gitnexus://repo/pumpkin-demon/clusters` | All functional areas |
| `gitnexus://repo/pumpkin-demon/processes` | All execution flows |
| `gitnexus://repo/pumpkin-demon/process/{name}` | Step-by-step execution trace |

## CLI

| Task | Read this skill file |
| --- | --- |
| Understand architecture / "How does X work?" | `.claude/skills/gitnexus-exploring/SKILL.md` |
| Blast radius / "What breaks if I change X?" | `.claude/skills/gitnexus-impact-analysis/SKILL.md` |
| Trace bugs / "Why is X failing?" | `.claude/skills/gitnexus-debugging/SKILL.md` |
| Rename / extract / split / refactor | `.claude/skills/gitnexus-refactoring/SKILL.md` |
| Tools, resources, schema reference | `.claude/skills/gitnexus-guide/SKILL.md` |
| Index, status, clean, wiki CLI commands | `.claude/skills/gitnexus-cli/SKILL.md` |

<!-- gitnexus:end -->
