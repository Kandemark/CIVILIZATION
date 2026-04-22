# Contributing to Dominion

Dominion is an open source grand strategy simulation built with C11 and SDL3. All contributions are welcome under the AGPLv3 license.

## Getting Started

1. Fork the repository and clone your fork
2. Build with `make clean && make release`
3. Run `./build/dominion` to verify everything works
4. Generate data files: `python3 tools/generate_earth_map.py && python3 tools/generate_nations.py && python3 tools/generate_borders.py && python3 tools/generate_resources.py && python3 tools/generate_cities.py && python3 tools/generate_flags.py`

## Code Style

- **C11** standard (`-std=c11`)
- **snake_case** for functions and variables
- **civ_** prefix for all public API functions and types
- **PascalCase** for enum values: `CIV_MAP_VIEW_POLITICAL`
- **UPPER_SNAKE** for constants and macros
- Keep headers in `include/` mirroring the source tree under `src/`
- No comments that explain WHAT the code does — well-named identifiers handle that. Comments only for WHY (non-obvious constraints, workarounds)
- Prefer editing existing files over creating new ones

## Build Requirements

Always rebuild fully after header changes:

```bash
make clean && make release
```

The Makefile does not track `.h` dependencies — struct layout changes in headers require a clean rebuild or the old object layout gets linked.

## Pull Request Process

1. Create a feature branch from `main`
2. Make your changes, following code style
3. Ensure `make clean && make release` builds without errors
4. Test your changes by running the game
5. Push your branch and open a PR against `main`
6. Describe what changed and why in the PR description

## Data Pipeline

Generated data files (`data/*.bin`, `data/*.earth`, `data/flags/`) are **not tracked in git**. Run the Python tools to generate them locally.

## Project Structure

```
src/
├── engine/          # SDL3 window, rendering, input, fonts
├── display/         # Camera, theme, animation, layer stack, draw list
├── ui/
│   ├── scenes/      # Scene-based screens (splash, menus, game)
│   ├── screens/     # In-game data screens (economy, dashboard, etc.)
│   ├── widget/      # Retained-mode widget library (13 types)
│   ├── panel/       # Gameplay panels (diplomacy, governance, etc.)
│   ├── graph/       # Chart rendering (bar, line, pie, sparkline, etc.)
│   ├── icon/        # Icon atlas system
│   └── layout/      # Flex and grid layout engines
├── core/
│   ├── world/       # Map, nations, territory, borders, settlements
│   ├── economy/     # 18 production/market/fiscal modules
│   ├── governance/  # Government branches, institutions, legal
│   ├── military/    # Units, combat, conquest
│   ├── diplomacy/   # Relations, treaties, organizations
│   ├── culture/     # Identity, language, religion, diffusion
│   ├── population/  # Demographics, vitality
│   ├── technology/  # Index-based innovation system
│   ├── ai/          # Strategic and tactical AI
│   └── simulation_engine/  # Time, orchestration, persistence
├── utils/           # Memory pools, config, cache, noise, types
└── core/visuals/    # Vexillology
```

## Questions?

Open an issue or start a discussion.
