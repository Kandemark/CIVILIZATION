# Architecture Overview - C Implementation

## Project Structure

The Civilization simulation has been converted from Python to pure C with a modular, well-organized structure:

```
CIVILIZATION/
├── include/              # Header files
│   ├── common.h         # Common definitions and utilities
│   ├── types.h          # Core type definitions
│   └── core/            # Core system headers
│       ├── game.h       # Main game controller
│       └── simulation_engine/
│           └── time_manager.h
├── src/                 # Source files
│   ├── main.c          # Entry point
│   ├── core/           # Core system implementations
│   │   ├── game.c
│   │   └── simulation_engine/
│   │       └── time_manager.c
│   └── utils/          # Utility functions
│       ├── common.c
│       └── types.c
├── lib/                # External libraries (future)
├── build/              # Build artifacts
├── CMakeLists.txt      # CMake build configuration
└── Makefile           # Make build configuration
```

## Core Systems

### 1. Time Management (`time_manager.h/c`)

The time management system handles:
- Multiple time scales (Paused, Slow, Normal, Fast, Very Fast, Ultra)
- Game calendar with seasons
- Time-based event listeners
- Time delta calculation for frame-based updates

**Key Structures:**
- `civ_time_manager_t`: Main time manager
- `civ_calendar_t`: Game calendar with year/month/day tracking
- `civ_time_listener_t`: Callback system for time events

### 2. Game Controller (`game.h/c`)

The main game controller orchestrates all systems:
- Game state management (Initializing, Running, Paused, etc.)
- System initialization and updates
- Event logging
- Save/load functionality
- Performance metrics

**Key Structures:**
- `civ_game_t`: Main game instance
- `civ_game_config_t`: Game configuration
- `civ_game_event_t`: Event log entries

### 3. Common Utilities

**`common.h`**: Provides:
- Error codes and result types
- Memory management macros
- Logging system
- Common constants and macros

**`types.h`**: Defines:
- Core types (IDs, floats, ints)
- String type (`civ_string_t`)
- Vector types (2D, 3D)
- Color and timestamp types

## Design Principles

### 1. Memory Management
- All allocations use `CIV_MALLOC`, `CIV_CALLOC`, `CIV_REALLOC`
- All deallocations use `CIV_FREE` (which nullifies pointers)
- Clear ownership semantics for all resources

### 2. Error Handling
- `civ_result_t` type for operation results
- Error codes in `civ_error_t` enum
- Consistent error checking throughout

### 3. Modularity
- Each system is self-contained in its own header/source pair
- Clear interfaces between systems
- Minimal dependencies

### 4. Performance
- Efficient data structures
- Minimal allocations in hot paths
- Performance metrics tracking

## Build System

### CMake (Recommended)
```bash
mkdir build && cd build
cmake ..
make
```

### Make
```bash
make          # Debug build
make release  # Release build
make clean    # Clean build artifacts
```

## Future Systems (To Be Implemented)

1. **Economy System** (`economy/`)
   - Market dynamics
   - Resource management
   - Trade networks

2. **Population System** (`population/`)
   - Demographics
   - Migration
   - Growth dynamics

3. **Technology System** (`technology/`)
   - Research tree
   - Innovation system
   - Knowledge diffusion

4. **Military System** (`military/`)
   - Units and formations
   - Combat mechanics
   - Logistics

5. **Diplomacy System** (`diplomacy/`)
   - Relations between nations
   - Alliances and agreements
   - International organizations

6. **World System** (`world/`)
   - Dynamic borders
   - Territory management
   - Geographic features

7. **Culture System** (`culture/`)
   - Cultural identity
   - Language evolution
   - Assimilation

8. **Event System** (`events/`)
   - Event bus
   - Event handlers
   - Ripple effects

9. **AI System** (`ai/`)
   - Strategic AI
   - Tactical AI
   - Behavior trees

## Data Serialization

Currently uses simple JSON format for save/load. Future improvements:
- Full JSON library integration (cJSON or similar)
- Binary serialization option
- Compression for large saves

## Platform Support

- **Windows**: MSVC and MinGW support
- **Linux**: GCC/Clang support
- **macOS**: Clang support

## Dependencies

Currently **zero external dependencies** - pure C standard library.

Future optional dependencies:
- JSON library (for better serialization)
- Math library (already linked via `-lm`)
- Threading library (for parallel system updates)

## Code Style

- C11 standard
- Snake_case for functions and variables
- `civ_` prefix for all public symbols
- Clear documentation in header files
- Consistent error handling patterns

## Migration Notes

### From Python to C

Key differences in the C implementation:
1. **Manual memory management** instead of garbage collection
2. **Explicit error handling** with result types
3. **Struct-based** instead of class-based design
4. **Function pointers** for callbacks instead of methods
5. **Header/source separation** for compilation

### Performance Considerations

The C implementation should provide:
- Faster execution (compiled vs interpreted)
- Lower memory overhead
- Better cache locality
- Easier optimization

## Contributing

When adding new systems:
1. Create header in `include/core/[system]/`
2. Create implementation in `src/core/[system]/`
3. Add to build system (CMakeLists.txt and Makefile)
4. Document in this file
5. Follow existing code patterns
