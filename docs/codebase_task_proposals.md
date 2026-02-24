# Codebase Issue Triage: Proposed Tasks

## 1) Typo fix task
- **Issue found:** A biome generation comment says "But per definition this overrides." which is awkward/incorrect phrasing in English (intended meaning appears to be "By definition").
- **Why it matters:** Small wording errors in inline comments make simulation logic harder to scan and can confuse contributors.
- **Proposed task:**
  1. Update the comment to "By definition, this overrides biome classification." (or equivalent clear wording).
  2. Do a quick pass over nearby biome comments for similar phrasing issues.
- **Acceptance criteria:** Biome override comment reads clearly and unambiguously.
- **Evidence:** `src/systems/biomes.c` line 50.

## 2) Bug fix task
- **Issue found:** The game references wonder manager symbols from `game.c`, but current build configuration does not compile/link `src/core/world/wonders.c`, causing linker failures (undefined references).
- **Why it matters:** This is a build-breaking issue that blocks producing a runnable binary.
- **Proposed task:**
  1. Add `src/core/world/wonders.c` to the build source list in `CMakeLists.txt`.
  2. Run a clean configure/build to verify linker errors are resolved.
  3. Optionally add a build-time guard/check to catch missing source units for required subsystems.
- **Acceptance criteria:** Project links successfully without undefined references to wonder manager functions.
- **Evidence:** `src/core/game.c` lines 103, 213, 414; `src/core/world/wonders.c` lines 11, 20, 114; prior linker output in `build_error.txt`.

## 3) Documentation discrepancy task
- **Issue found:** Core design intent emphasizes a dynamic, unscripted simulation with no fixed end-state, while README philosophy text is currently brief and may underspecify that intent for new contributors.
- **Why it matters:** Missing this context can lead contributors to implement rigid scenario logic that conflicts with the project's "evolutionary" simulation goal.
- **Proposed task:**
  1. Expand README philosophy language to explicitly state: no scripted win-state/end-cap, world evolution driven by in-game actor decisions, and index-based progression metrics.
  2. Add one short example that clarifies technology is tracked as an index/progression multiplier, not a fixed linear tech tree endpoint.
- **Acceptance criteria:** README explicitly communicates dynamic/no-cap simulation intent and index-based progression framing.
- **Evidence:** `README.md` sections "Core Simulation Philosophy" and "Infinite Evolution".

## 4) Test improvement task
- **Issue found:** There is no automated test/smoke check that would have caught the wonder linkage regression before manual build attempts.
- **Why it matters:** Missing CI-level build verification allows link-break regressions to slip in.
- **Proposed task:**
  1. Add a minimal CI/local smoke target (e.g., CMake configure + build in Debug) executed in automation.
  2. Treat unresolved symbols as failing checks.
  3. Optionally add a tiny subsystem compilation test that instantiates `civ_game_t` and verifies creation/destruction paths compile and link.
- **Acceptance criteria:** A single command validates end-to-end compile/link and fails fast on missing source linkage.
- **Evidence:** Linker failure captured in `build_error.txt` for wonder manager symbols.
