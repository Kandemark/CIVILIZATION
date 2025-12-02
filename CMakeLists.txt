cmake_minimum_required(VERSION 3.10)
project(Civilization VERSION 0.1.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Build options
option(BUILD_TESTS "Build test suite" OFF)
option(BUILD_DOCS "Build documentation" OFF)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(CORE_SOURCES
    src/core/game.c
    src/core/simulation_engine/time_manager.c
    src/core/simulation_engine/system_orchestrator.c
    src/core/simulation_engine/performance_optimizer.c
    src/core/simulation_engine/event_dispatcher.c
    src/core/simulation_engine/state_persistence.c
    src/core/population/demographics.c
    src/core/population/population_manager.c
    src/core/economy/market.c
    src/core/technology/innovation_system.c
    src/core/military/units.c
    src/core/military/combat.c
    src/core/diplomacy/relations.c
    src/core/events/event_manager.c
    src/core/world/dynamic_borders.c
    src/core/governance/government.c
    src/core/governance/custom_governance.c
    src/core/environment/geography.c
    src/core/military/conquest.c
    src/core/abstracts/soft_metrics.c
    src/core/culture/cultural_identity.c
    src/core/culture/writing_system.c
    src/core/culture/cultural_diffusion.c
    src/core/culture/cultural_assimilation.c
    src/core/culture/language_evolution.c
    src/core/culture/culture.c
    src/core/world/map_generator.c
    src/core/world/map_view.c
    src/core/world/territory.c
    src/core/visualization/cultural_display.c
    src/core/ai/base_ai.c
    src/core/ai/strategic_ai.c
    src/core/ai/tactical_ai.c
    src/core/ai/ai_system.c
    src/core/politics/faction_system.c
    src/core/politics/politics.c
    src/core/subunits/subunit.c
)

set(UTILS_SOURCES
    src/utils/common.c
    src/utils/types.c
    src/utils/memory_pool.c
    src/utils/config.c
    src/utils/cache.c
    src/utils/utils.c
)

set(SYSTEMS_SOURCES
    src/systems/climate.c
    src/systems/biomes.c
    src/systems/geography.c
    src/systems/events.c
    src/systems/politics.c
)

set(MAIN_SOURCES
    src/core/main.c
)

# Combine all sources
set(ALL_SOURCES
    ${CORE_SOURCES}
    ${UTILS_SOURCES}
    ${SYSTEMS_SOURCES}
    ${MAIN_SOURCES}
)

# Create executable
add_executable(civilization ${ALL_SOURCES})

# Compiler flags
if(MSVC)
    target_compile_options(civilization PRIVATE /W4)
else()
    target_compile_options(civilization PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Debug flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(civilization PRIVATE DEBUG=1)
    if(MSVC)
        target_compile_options(civilization PRIVATE /Od /Zi)
    else()
        target_compile_options(civilization PRIVATE -g -O0)
    endif()
endif()

# Release flags - OPTIMIZED
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        # MSVC optimizations
        target_compile_options(civilization PRIVATE 
            /O2          # Maximum optimization
            /GL          # Whole program optimization
            /Oi          # Enable intrinsic functions
            /Ot          # Favor fast code
            /Oy          # Omit frame pointers
            /fp:fast     # Fast floating point
        )
        target_link_options(civilization PRIVATE /LTCG)
    else()
        # GCC/Clang optimizations
        target_compile_options(civilization PRIVATE 
            -O3                      # Maximum optimization
            -DNDEBUG                 # Disable assertions
            -flto                    # Link-time optimization
            -march=native            # Optimize for current CPU
            -mtune=native            # Tune for current CPU
            -fomit-frame-pointer     # Remove frame pointers
            -fno-stack-protector     # Disable stack protection (performance)
            -ffast-math              # Fast math operations
            -funroll-loops           # Unroll loops
            -ftree-vectorize         # Enable auto-vectorization
            -fprefetch-loop-arrays   # Prefetch arrays in loops
            -fno-signed-zeros        # Assume no signed zeros
            -fno-trapping-math       # No trapping math
        )
        target_link_options(civilization PRIVATE -flto)
    endif()
endif()

# Installation
install(TARGETS civilization
    RUNTIME DESTINATION bin
)

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")

# Performance optimizations summary
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Performance optimizations: ENABLED")
    message(STATUS "  - Link-time optimization (LTO)")
    message(STATUS "  - Native architecture optimizations")
    message(STATUS "  - Fast math optimizations")
    message(STATUS "  - Auto-vectorization enabled")
    message(STATUS "  - Loop unrolling enabled")
    message(STATUS "  - Prefetch optimizations enabled")
else()
    message(STATUS "Performance optimizations: DISABLED (Debug build)")
endif()
