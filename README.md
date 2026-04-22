# Dominion — Grand Strategy Simulation

A high-fidelity grand strategy simulation built with C11 and SDL3. Dominion simulates ~235 real-world nations on a 2048x1024 Earth map with dynamic borders, 22-sector economies, index-based technology progression, and fully dynamic governance systems. No victory screens, no scripted end-states.

## License

**GNU Affero General Public License v3.0 (AGPLv3)** — see [LICENSE](LICENSE)

## Quick Start

```bash
# Install dependencies (Arch)
sudo pacman -S sdl3 sdl3_ttf gcc pkg-config

# Generate Earth data
python3 tools/generate_earth_map.py
python3 tools/generate_nations.py
python3 tools/generate_borders.py
python3 tools/generate_resources.py
python3 tools/generate_cities.py
python3 tools/generate_flags.py

# Build and run
make clean && make release
./build/dominion
```

## Design Philosophy

- **Index-driven, never capped**: Technology, economy, military power are continuous indices, not binary unlocks
- **Dynamic governance**: No predefined government types — political structure is defined per-nation by constitutional positions
- **Relative metrics**: Stats show position relative to global averages, not absolute scores
- **Sector economy**: 18 production and market modules simulating agriculture through war economy
- **Emergent outcomes**: A one-party state can have 89% citizen happiness if its institutions function well

## Systems

| System | Description |
|---|---|
| Map Engine | 2048x1024 equirectangular Earth, 5 switchable views, multi-scale LOD |
| Nations | ~235 real-world countries with ISO codes, flags, capitals |
| Economy | 18 modules: agriculture, banking, budget, energy, extraction, financial markets, housing, infrastructure, innovation, labor, land use, macro, manufacturing, policy, taxation, trade (domestic + international), war economy, black market |
| Governance | Dynamic branches (executive, legislative, judicial, council, religious), constitutional positions |
| Military | Units, combat resolution, conquest with territory transfer |
| Diplomacy | Relations, treaties, international organizations |
| Culture | Identity, language evolution, religion, ideology, cultural diffusion |
| Resources | 20 resource types (oil, gas, coal, iron, copper, gold, rare earths, lithium, etc.) |
| UI | 13 retained-mode widgets, theme system, popup windows, toast notifications |

## Dependencies

| Dependency | Arch Linux | Ubuntu/Debian |
|---|---|---|
| SDL3 | `sdl3` | Build from [source](https://github.com/libsdl-org/SDL) |
| SDL3_ttf | `sdl3_ttf` | Build from [source](https://github.com/libsdl-org/SDL_ttf) |
| GCC | `gcc` | `build-essential` |
| pkg-config | `pkg-config` | `pkg-config` |
| Python 3 | `python` | `python3` |
| numpy (data tools) | `python-numpy` | `python3-numpy` |

## Build

```bash
make release          # Optimized build
make debug            # Debug build with symbols
make clean && make release  # Full rebuild (required after header changes)
```

The Makefile does not track `.h` dependencies. Always `make clean` before `make release` after changing headers.

## Data Pipeline

Generated data is **not tracked in git**. Run these in order:

```bash
python3 tools/generate_earth_map.py       # data/earth_2048x1024.earth
python3 tools/generate_nations.py         # data/nations.bin (~235 countries)
python3 tools/generate_borders.py         # data/earth_borders.bin
python3 tools/generate_resources.py       # data/resources.bin (20 types, 200+ deposits)
python3 tools/generate_cities.py          # data/cities.bin (3000+ cities)
python3 tools/generate_flags.py           # data/flags/ (235 PNG flags)
```

## Architecture

```
src/
├── engine/          # SDL3 window, rendering, input, fonts
├── display/         # Camera, theme, animation, layer stack, draw list
├── ui/
│   ├── scenes/      # Scene screens (splash → menus → game)
│   ├── screens/     # Data screens (economy, dashboard, etc.)
│   ├── widget/      # Retained-mode widgets (13 types)
│   ├── panel/       # Gameplay panels
│   ├── graph/       # Charts (bar, line, pie, sparkline)
│   └── layout/      # Flex + grid layout engine
├── core/
│   ├── world/       # Map, nations, borders, territory, settlements
│   ├── economy/     # 18-module production/market/fiscal simulation
│   ├── governance/  # Branches, institutions, legal
│   ├── military/    # Units, combat, conquest
│   ├── diplomacy/   # Relations, treaties, organizations
│   ├── culture/     # Language, religion, ideology
│   ├── population/  # Demographics, vitality
│   ├── technology/  # Index-based innovation
│   ├── ai/          # Strategic + tactical AI
│   └── simulation_engine/  # Time, orchestration, persistence
├── utils/           # Memory pools, config, cache, noise
└── tools/           # Python data generation tools
```

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md). All contributions under AGPLv3.
