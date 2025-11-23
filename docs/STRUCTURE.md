# Project Structure - Pure C Implementation

## Complete Directory Structure

```
CIVILIZATION/
├── include/                          # All header files
│   ├── common.h                      # Common definitions, macros, error codes
│   ├── types.h                       # Core type definitions (CivString, CivVector, etc.)
│   │
│   ├── core/                         # Core game systems
│   │   ├── game.h                    # Main game controller
│   │   │
│   │   ├── interfaces/               # System interfaces
│   │   │   ├── iupdatable.h         # Updatable interface
│   │   │   ├── iserializable.h      # Serializable interface
│   │   │   └── iai_entity.h         # AI entity interface
│   │   │
│   │   ├── simulation_engine/       # Simulation infrastructure
│   │   │   ├── time_manager.h       # Time management
│   │   │   ├── system_orchestrator.h # System coordination
│   │   │   ├── performance_optimizer.h # Performance profiling
│   │   │   ├── event_dispatcher.h   # Event dispatching
│   │   │   ├── state_persistence.h  # Save/load
│   │   │   └── deterministic_mode.h  # Deterministic simulation
│   │   │
│   │   ├── population/              # Population systems
│   │   │   ├── demographics.h      # Demographics tracking
│   │   │   ├── population_manager.h # Population management
│   │   │   ├── migration.h          # Migration system
│   │   │   ├── health_system.h     # Health and disease
│   │   │   ├── education_system.h  # Education system
│   │   │   ├── labor_mobility.h    # Labor mobility
│   │   │   ├── satisfaction.h      # Population satisfaction
│   │   │   └── social_stratification.h # Social classes
│   │   │
│   │   ├── economy/                 # Economic systems
│   │   │   ├── market.h            # Market dynamics
│   │   │   ├── trade.h             # Trade system
│   │   │   ├── trade_networks.h    # Trade networks
│   │   │   ├── production.h        # Production system
│   │   │   ├── production_chains.h # Production chains
│   │   │   ├── resource_extraction.h # Resource extraction
│   │   │   ├── infrastructure.h    # Infrastructure
│   │   │   ├── distribution.h      # Wealth distribution
│   │   │   ├── urbanization.h      # Urbanization
│   │   │   └── economic_crises.h   # Economic crises
│   │   │
│   │   ├── technology/             # Technology systems
│   │   │   ├── innovation_system.h # Innovation and research
│   │   │   ├── research_areas.h    # Research areas
│   │   │   ├── technological_eras.h # Tech eras
│   │   │   ├── knowledge_diffusion.h # Knowledge spread
│   │   │   └── specialization.h    # Tech specialization
│   │   │
│   │   ├── military/               # Military systems
│   │   │   ├── units.h             # Military units
│   │   │   ├── combat.h            # Combat system
│   │   │   ├── formations.h        # Unit formations
│   │   │   ├── logistics.h         # Military logistics
│   │   │   ├── morale.h            # Unit morale
│   │   │   ├── recruitment.h      # Unit recruitment
│   │   │   ├── doctrine.h          # Military doctrine
│   │   │   ├── war_exhaustion.h    # War exhaustion
│   │   │   └── occupation_system.h # Occupation
│   │   │
│   │   ├── diplomacy/               # Diplomacy systems
│   │   │   ├── relations.h         # Diplomatic relations
│   │   │   ├── agreements.h        # Diplomatic agreements
│   │   │   ├── alliances.h         # Alliances
│   │   │   ├── espionage_system.h  # Espionage
│   │   │   ├── sanctions_system.h  # Sanctions
│   │   │   ├── cultural_diplomacy.h # Cultural diplomacy
│   │   │   └── international_organizations.h # International orgs
│   │   │
│   │   ├── culture/                 # Culture systems
│   │   │   ├── cultural_identity.h  # Cultural identity
│   │   │   ├── cultural_diffusion.h # Cultural diffusion
│   │   │   ├── language_evolution.h # Language evolution
│   │   │   ├── assimilation.h      # Cultural assimilation
│   │   │   ├── ideology_system.h   # Ideology system
│   │   │   └── heritage_preservation.h # Heritage preservation
│   │   │
│   │   ├── governance/              # Governance systems
│   │   │   ├── government.h        # Government types
│   │   │   ├── policies.h          # Policy system
│   │   │   ├── reform_system.h     # Government reforms
│   │   │   └── coup_system.h      # Coups and revolutions
│   │   │
│   │   ├── politics/                # Political systems
│   │   │   ├── faction_system.h    # Political factions
│   │   │   ├── legitimacy.h        # Legitimacy system
│   │   │   ├── power_struggles.h   # Power struggles
│   │   │   ├── political_crises.h # Political crises
│   │   │   └── succession_systems.h # Succession systems
│   │   │
│   │   ├── world/                   # World systems
│   │   │   ├── dynamic_borders.h   # Dynamic borders
│   │   │   ├── territory_manager.h  # Territory management
│   │   │   ├── geographic_features.h # Geographic features
│   │   │   ├── colonial_system.h   # Colonization
│   │   │   └── cartography_system.h # Map system
│   │   │
│   │   ├── environment/              # Environment systems
│   │   │   ├── geography.h         # Geography system
│   │   │   ├── climate_system.h    # Climate system
│   │   │   ├── climate_change.h    # Climate change
│   │   │   ├── ecology.h           # Ecology
│   │   │   ├── natural_disasters.h # Natural disasters
│   │   │   ├── resource_depletion.h # Resource depletion
│   │   │   └── environmental_degradation.h # Environmental degradation
│   │   │
│   │   ├── events/                   # Event systems
│   │   │   ├── event_manager.h     # Event manager
│   │   │   ├── event_bus.h         # Event bus
│   │   │   ├── event_handlers.h    # Event handlers
│   │   │   ├── ripple_system.h     # Event ripple effects
│   │   │   └── emergent_behavior.h # Emergent behavior
│   │   │
│   │   ├── abstracts/                # Abstract systems
│   │   │   ├── soft_metrics.h      # Soft metrics (happiness, prestige)
│   │   │   ├── fuzzy_logic.h       # Fuzzy logic
│   │   │   ├── probabilistic_models.h # Probabilistic models
│   │   │   ├── influence_networks.h # Influence networks
│   │   │   └── emergent_properties.h # Emergent properties
│   │   │
│   │   ├── ai/                       # AI systems
│   │   │   ├── base_ai.h           # Base AI
│   │   │   ├── ai_director.h       # AI director
│   │   │   ├── strategic_ai.h     # Strategic AI
│   │   │   ├── tactical_ai.h      # Tactical AI
│   │   │   ├── utility_ai.h        # Utility-based AI
│   │   │   ├── decision_making.h   # Decision making
│   │   │   └── behavior_trees.h    # Behavior trees
│   │   │
│   │   └── subunits/                # Subunit systems
│   │       ├── subunit.h           # Base subunit
│   │       ├── subunit_manager.h  # Subunit manager
│   │       └── subunit_factory.h  # Subunit factory
│   │
│   ├── utils/                        # Utility systems
│   │   ├── memory_pool.h           # Memory pool allocator
│   │   ├── config.h                # Configuration system
│   │   ├── cache.h                 # Caching system
│   │   ├── math_utils.h            # Math utilities
│   │   ├── geometry.h              # Geometric operations
│   │   ├── pathfinding.h           # Pathfinding
│   │   └── validators.h            # Validation utilities
│   │
│   └── data/                         # Data structures
│       ├── serialization.h         # Serialization
│       └── persistence.h           # Data persistence
│
├── src/                              # All source files (mirrors include/)
│   ├── main.c                       # Entry point
│   │
│   ├── core/                        # Core implementations
│   │   ├── game.c
│   │   │
│   │   ├── interfaces/             # Interface implementations
│   │   │
│   │   ├── simulation_engine/      # Simulation engine
│   │   │   ├── time_manager.c
│   │   │   ├── system_orchestrator.c
│   │   │   ├── performance_optimizer.c
│   │   │   └── ...
│   │   │
│   │   ├── population/            # Population systems
│   │   │   ├── demographics.c
│   │   │   ├── population_manager.c
│   │   │   └── ...
│   │   │
│   │   └── [all other systems mirror include/ structure]
│   │
│   ├── utils/                       # Utility implementations
│   │   ├── common.c
│   │   ├── types.c
│   │   ├── memory_pool.c
│   │   ├── config.c
│   │   ├── cache.c
│   │   └── ...
│   │
│   └── data/                        # Data implementations
│       └── serialization.c
│
├── assets/                          # Game assets (future)
│   ├── configs/                    # Configuration files
│   └── data/                       # Data files
│
├── build/                           # Build artifacts
│   └── bin/                        # Compiled binaries
│
├── docs/                            # Documentation
│   ├── architecture/              # Architecture docs
│   ├── api/                       # API documentation
│   └── tutorials/                # Tutorials
│
├── tests/                          # Test suite (future)
│
├── CMakeLists.txt                  # CMake build configuration
├── Makefile                        # Make build configuration
├── README.md                       # Main README
├── ARCHITECTURE.md                 # Architecture overview
└── LICENSE                         # License file
```

## Key Principles

1. **Header/Source Separation**: All headers in `include/`, all sources in `src/`
2. **Mirror Structure**: `src/` mirrors `include/` structure exactly
3. **Modular Design**: Each system in its own directory
4. **Interface-Based**: Systems implement standard interfaces
5. **No Python Files**: Pure C implementation only

## System Organization

### Core Systems (13 major systems)
1. **Simulation Engine** - Time, orchestration, performance
2. **Population** - Demographics, migration, health, education
3. **Economy** - Market, trade, production, infrastructure
4. **Technology** - Innovation, research, knowledge diffusion
5. **Military** - Units, combat, logistics, doctrine
6. **Diplomacy** - Relations, agreements, espionage
7. **Culture** - Identity, diffusion, language, ideology
8. **Governance** - Government, policies, reforms
9. **Politics** - Factions, legitimacy, power struggles
10. **World** - Borders, territory, geography
11. **Environment** - Climate, ecology, disasters
12. **Events** - Event system, ripple effects
13. **Abstracts** - Soft metrics, fuzzy logic

### Supporting Systems
- **AI** - Strategic and tactical AI
- **Subunits** - Subunit management
- **Interfaces** - System interfaces
- **Utils** - Utility functions
- **Data** - Serialization and persistence

