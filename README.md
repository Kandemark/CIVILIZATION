# CIVILIZATION: Total Simulation

A high-fidelity grand strategy simulation built with SDL3. Unlike traditional 4X games, this is an **Agnostic Total Simulation** where success is defined purely by evolutionary growth and functional institutional control.

## 核心 (Core Simulation Philosophy)

*   **Agnostic Governance**: No "best" political system. A nation's stability depends on how well its branches (Executive, Legislative, Judicial) function within their own evolving rules and institutions.
*   **Total Social Simulation**: Stability and identity are emergent properties of **Language, Religion, Ideology, and Culture**. 
*   **Administrative Hierarchy**: Design your nation from the bottom up. Create States, Colonies, and Provinces with varying levels of autonomy.
*   **Infinite Evolution**: No victory screens. National power is measured by a **7-Tier Global Stature Ranking** (Hegemon to Failed State). Progress is continuous and requires exponential investment.
*   **Dynamic Evolution (No Scripted End-State)**: The simulation is intentionally unscripted—player and AI decisions continuously reshape institutions, borders, and social systems without a fixed scenario endpoint.
*   **Index-Based Progression**: Systems such as technology are tracked as progression indices that represent capability/efficiency growth, not a hard-capped predefined ladder.

## Systems Overview

### 1. Sovereign Governance
*   **Modular Branches**: Define custom legislative thresholds, voting methods, and judicial review powers.
*   **Autonomous Legislation**: Branches independently propose and vote on bills, updating the national Constitution in real-time.
*   **The "Hard Path" of Reform**: Systemic changes like a coup or reformation require long-term institutional manipulation.

### 2. Administrative Geography
*   **Subdivisions**: Draw your own borders to create regional governments.
*   **Custom Autonomy**: Manage colonial friction vs. incorporated state stability.
*   **Tactical Laws**: Pass "Official Language Acts" or "Religious Standardizations" to drive assimilation, at the risk of local unrest.

### 3. Integrated Systems
*   **Global Atlas Basemap**: The world uses one deterministic atlas-style baseline (non-Earth), focused on land/water/polar layout so political shifts remain the primary visual signal.
*   **Dynamic Borders**: Territorial expansion driven by both military conquest and cultural diffusion.
*   **Strategic AI**: AI nations evaluate global threats, form stances (Wary, Hostile, Friendly), and respond to proximity-based "Border Friction."
*   **High-Fidelity Rendering**: Hardware-accelerated SDL3 engine with support for large-scale world maps and detailed HUDs.

## Quick Start

### Build & Run
```powershell
mingw32-make release
cd build
.\civilization.exe
```

## Architecture

- **Engine Layer** (`src/engine/`) - SDL3 window, rendering, input
- **Governance** (`src/core/governance/`) - Legislative, Institutional, and Administrative logic
- **World Context** (`src/core/world/`) - Settlements, Maps, Subdivisions
- **Simulation** (`src/core/game.c`) - The main infinite loop

## License
See LICENSE file.
