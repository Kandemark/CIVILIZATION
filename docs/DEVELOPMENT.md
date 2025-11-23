# Development Guide

## Current Status

### ✅ Implemented Systems (18 Systems)
1. **Simulation Engine** - Time, orchestration, performance, events, persistence
2. **Population** - Demographics, population management
3. **Economy** - Market dynamics
4. **Technology** - Innovation and research
5. **Military** - Units and combat
6. **Diplomacy** - Relations and treaties
7. **Events** - Event management
8. **World** - Dynamic borders
9. **Governance** - Government types
10. **Environment** - Geography
11. **Abstracts** - Soft metrics
12. **Performance** - Optimizer, orchestrator, memory pool, cache, config
13. **Interfaces** - Updatable, serializable, AI entity

### ⏳ Next Features to Implement

#### High Priority
1. **Culture System** - Cultural identity, diffusion, language evolution
2. **AI System** - Strategic AI, tactical AI, behavior trees
3. **Politics System** - Factions, legitimacy, power struggles
4. **Subunits System** - Subunit management

#### Medium Priority
5. **Advanced Population** - Health system, education system, detailed migration
6. **Advanced Economy** - Trade networks, production chains, infrastructure
7. **Advanced Military** - Logistics, doctrine, morale, war exhaustion
8. **Advanced Diplomacy** - Espionage, sanctions, international organizations

#### Low Priority
9. **Advanced World** - Territory manager, colonial system, cartography
10. **Advanced Environment** - Climate system, natural disasters, ecology
11. **Advanced Events** - Ripple effects, emergent behavior
12. **Advanced Abstracts** - Fuzzy logic, probabilistic models, influence networks

## Development Guidelines

### Code Style
- Use `snake_case` for functions and variables
- Use `civ_` prefix for all public functions
- One header file per module
- One source file per header
- Document all public functions in headers

### Adding New Systems
1. Create header in `include/core/[system_name]/`
2. Create source in `src/core/[system_name]/`
3. Update `CMakeLists.txt` and `Makefile`
4. Implement `civ_updatable_t` interface
5. Register with system orchestrator
6. Add to main game controller

### Testing
- Add unit tests in `tests/`
- Test on multiple platforms
- Use performance profiler
- Verify memory management

### Performance
- Use memory pools for frequent allocations
- Cache expensive computations
- Profile before optimizing
- Use system orchestrator for coordination

## File Organization

```
include/core/[system]/
  ├── [system].h          # Main header
  └── [subsystem].h       # Subsystem headers (if needed)

src/core/[system]/
  ├── [system].c          # Main implementation
  └── [subsystem].c       # Subsystem implementations (if needed)
```

## Build System

Always update both:
- `CMakeLists.txt` - Add new sources to `CORE_SOURCES`
- `Makefile` - Add new sources to `CORE_SOURCES`

## Next Steps

1. Implement Culture System
2. Implement AI System
3. Implement Politics System
4. Enhance existing systems with advanced features

