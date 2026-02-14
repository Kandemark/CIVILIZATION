cmake_minimum_required(VERSION 3.10)
project(Civilization VERSION 1.0.0 LANGUAGES C)

# C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# SDL3 paths
set(SDL3_DIR "C:/SDL/SDL3-3.4.0/x86_64-w64-mingw32")
set(SDL3_TTF_DIR "C:/SDL/SDL3_ttf-3.2.2/x86_64-w64-mingw32")

set(SDL3_INCLUDE_DIR "${SDL3_DIR}/include/SDL3")
set(SDL3_TTF_INCLUDE_DIR "${SDL3_TTF_DIR}/include/SDL3_ttf")
set(SDL3_LIB_DIR "${SDL3_DIR}/lib")
set(SDL3_TTF_LIB_DIR "${SDL3_TTF_DIR}/lib")

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${SDL3_INCLUDE_DIR}
    ${SDL3_TTF_INCLUDE_DIR}
)

# Link directories
link_directories(
    ${SDL3_LIB_DIR}
    ${SDL3_TTF_LIB_DIR}
)

# Engine sources
set(ENGINE_SOURCES
    src/engine/window.c
    src/engine/renderer.c
    src/engine/input.c
    src/engine/font.c
)

# UI sources
set(UI_SOURCES
    src/ui/sdl3_main.c
    src/ui/button.c
    src/ui/ui_common.c
)

# Core game sources (preserved from original)
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
    src/core/governance/interaction.c
    src/core/governance/conversation.c
    src/core/governance/ministry.c
    src/core/governance/notebook.c
    src/core/governance/corruption.c
    src/core/governance/societal_metrics.c
    src/core/culture/religion_system.c
    src/core/governance/legal_status.c
    src/core/population/race_system.c
    src/core/population/population_vitality.c
    src/core/diplomacy/international_organizations.c
    src/core/diplomacy/unification_engine.c
    src/core/economy/currency_system.c
    src/core/economy/trade_system.c
    src/core/economy/resource_market.c
    src/core/environment/disaster_system.c
    src/core/events/story_events.c
    src/core/events/game_events.c
    src/core/politics/political_rivalry.c
    src/core/governance/rule_system.c
    src/core/governance/governance_evolution.c
    src/core/knowledge_system.c
)

# Utils sources
set(UTILS_SOURCES
    src/utils/common.c
    src/utils/types.c
    src/utils/memory_pool.c
    src/utils/config.c
    src/utils/cache.c
    src/utils/utils.c
    src/core/visuals/vexillology.c
    src/utils/noise.c
)

# Systems sources
set(SYSTEMS_SOURCES
    src/systems/climate.c
    src/systems/biomes.c
    src/systems/geography.c
    src/systems/events.c
    src/systems/politics.c
)

# Create main executable
add_executable(civilization
    ${ENGINE_SOURCES}
    ${UI_SOURCES}
    ${CORE_SOURCES}
    ${UTILS_SOURCES}
    ${SYSTEMS_SOURCES}
)

# Link SDL3 libraries
target_link_libraries(civilization PRIVATE
    SDL3
    SDL3_ttf
    m  # Math library
)

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
        target_compile_options(civilization PRIVATE 
            /O2 /GL /Oi /Ot /Oy /fp:fast
        )
        target_link_options(civilization PRIVATE /LTCG)
    else()
        target_compile_options(civilization PRIVATE 
            -O3 -DNDEBUG -flto -march=native
            -mtune=native -fomit-frame-pointer
            -ffast-math -funroll-loops
        )
        target_link_options(civilization PRIVATE -flto)
    endif()
endif()

# Copy SDL3 DLLs to build directory on Windows
if(WIN32)
    add_custom_command(TARGET civilization POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDL3_DIR}/bin/SDL3.dll"
            $<TARGET_FILE_DIR:civilization>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDL3_TTF_DIR}/bin/SDL3_ttf.dll"
            $<TARGET_FILE_DIR:civilization>
        COMMENT "Copying SDL3 DLLs to build directory"
    )
endif()

# Print build information
message(STATUS "========================================")
message(STATUS "CIVILIZATION - SDL3 Build Configuration")
message(STATUS "========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "SDL3 directory: ${SDL3_DIR}")
message(STATUS "SDL3_ttf directory: ${SDL3_TTF_DIR}")
message(STATUS "========================================")
