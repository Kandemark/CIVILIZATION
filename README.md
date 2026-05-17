# DOMINION: An Agnostic Total Grand Strategy Simulation

A high-fidelity grand strategy simulation built with SDL3. Unlike traditional 4X games, Dominion is an **Agnostic Total Simulation** where success is defined purely by evolutionary growth and functional institutional control — no victory screens, no scripted end-state.

## License

**GNU Affero General Public License v3.0 (AGPLv3)**

Dominion is free and open source software. You may use, modify, and distribute it under the terms of the AGPLv3. This ensures all modifications remain open — even when run as a network service. Commercial sublicensing, proprietary forks, and closed-source redistribution are prohibited by the copyleft terms.

To support development, visit: *[fundraising link coming soon]*

See [LICENSE](LICENSE) for the full text.

---

## Core Simulation Philosophy

- **Agnostic Governance**: No "best" political system. A nation's stability depends on how well its branches (Executive, Legislative, Judicial) function within their own evolving rules and institutions.
- **Total Social Simulation**: Stability and identity are emergent properties of **Language, Religion, Ideology, and Culture**.
- **Administrative Hierarchy**: Design your nation from the bottom up. Create States, Colonies, and Provinces with varying levels of autonomy.
- **Infinite Evolution**: No victory screens. National power is measured by a **7-Tier Global Stature Ranking** (Hegemon to Failed State). Progress is continuous and requires exponential investment.
- **Dynamic Evolution (No Scripted End-State)**: The simulation is intentionally unscripted — player and AI decisions continuously reshape institutions, borders, and social systems without a fixed scenario endpoint.
- **Index-Based Progression**: Technology is tracked as progression indices that represent capability/efficiency growth, not a hard-capped predefined tech ladder.

## Systems Overview

### Sovereign Governance
- **Modular Branches**: Define custom legislative thresholds, voting methods, and judicial review powers.
- **Autonomous Legislation**: Branches independently propose and vote on bills, updating the national Constitution in real-time.
- **The "Hard Path" of Reform**: Systemic changes like a coup or reformation require long-term institutional manipulation.

### Administrative Geography
- **Subdivisions**: Draw your own borders to create regional governments.
- **Custom Autonomy**: Manage colonial friction vs. incorporated state stability.
- **Tactical Laws**: Pass "Official Language Acts" or "Religious Standardizations" to drive assimilation, at the risk of local unrest.

### Integrated Systems
- **Global Atlas Basemap**: The world uses one deterministic atlas-style baseline (non-Earth), focused on land/water/polar layout so political shifts remain the primary visual signal.
- **Dynamic Borders**: Territorial expansion driven by both military conquest and cultural diffusion.
- **Strategic AI**: AI nations evaluate global threats, form stances (Wary, Hostile, Friendly), and respond to proximity-based "Border Friction."
- **High-Fidelity Rendering**: Hardware-accelerated SDL3 engine with support for large-scale world maps and detailed HUDs.

---

## Dependencies

| Dependency | Arch Linux | Ubuntu/Debian |
|---|---|---|
| SDL3 | `sdl3` | build from source or PPA |
| SDL3_ttf | `sdl3_ttf` | build from source |
| GCC / Clang | `gcc` | `build-essential` |
| CMake (optional) | `cmake` | `cmake` |
| pkg-config | `pkg-config` | `pkg-config` |

### Install on Arch

```bash
sudo pacman -S sdl3 sdl3_ttf gcc cmake pkg-config
```

### Install on Ubuntu/Debian

SDL3 is not yet packaged in Debian/Ubuntu. Build from source:
- [SDL3](https://github.com/libsdl-org/SDL)
- [SDL3_ttf](https://github.com/libsdl-org/SDL_ttf)

---

## Build & Run

### CMake (recommended)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/dominion
```

For debug builds:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/dominion
```

### Make

```bash
make release   # Optimized build
make debug     # Debug build
./build/dominion
```

---

## Architecture

```
src/
├── engine/          # SDL3 window, rendering, input, fonts
├── ui/              # Scene-based UI (splash, menus, game view)
│   └── scenes/      # Individual UI screens
├── core/            # All simulation systems
│   ├── governance/  # Government, legislation, institutions, subdivisions
│   ├── culture/     # Identity, language, religion, diffusion, assimilation
│   ├── diplomacy/   # Relations, treaties, international organizations
│   ├── economy/     # Markets, trade, currency, resources
│   ├── military/    # Units, combat, conquest
│   ├── ai/          # Strategic + tactical AI with personality types
│   ├── world/       # Map generation, territory, dynamic borders, wonders
│   ├── population/  # Demographics, vitality, race system
│   ├── politics/    # Factions, rivalry
│   ├── technology/  # Innovation system (index-based progression)
│   ├── events/      # Story events, game events, disasters
│   ├── simulation_engine/  # Time, orchestration, persistence, performance
│   └── abstracts/   # Soft metrics, shared interfaces
├── systems/         # Climate, biomes, geography, events, politics
└── utils/           # Memory pools, config, cache, noise, types
```

---

## Contributing

Dominion welcomes contributors. All contributions fall under the AGPLv3 license. Before submitting PRs:

1. Ensure the project builds with `make release`
2. Follow the existing code style (C11, snake_case for functions, `civ_` prefix for public API)
3. Keep simulation logic data-driven — avoid hardcoded scenarios

---

## License

GNU Affero General Public License v3.0 — see [LICENSE](LICENSE)
