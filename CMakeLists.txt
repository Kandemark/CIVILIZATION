cmake_minimum_required(VERSION 3.10)
project(dominion VERSION 1.0.0 LANGUAGES C)

# C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find SDL3 and SDL3_ttf via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED IMPORTED_TARGET sdl3)
pkg_check_modules(SDL3_TTF REQUIRED IMPORTED_TARGET sdl3-ttf)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

# Engine sources
set(ENGINE_SOURCES
    src/engine/window.c
    src/engine/renderer.c
    src/engine/input.c
    src/engine/font.c
)

# Display engine sources
set(DISPLAY_SOURCES
    src/display/layer.c
    src/display/camera.c
    src/display/theme.c
    src/display/animation.c
    src/display/draw_list.c
    src/display/debug_overlay.c
)

# UI sources
set(UI_SOURCES
    src/ui/sdl3_main.c
    src/ui/ui_common.c
    src/ui/app_controller.c
    src/ui/scene_manager.c
    src/ui/scenes/scene_splash.c
    src/ui/scenes/scene_profile_select.c
    src/ui/scenes/scene_profile_create.c
    src/ui/scenes/scene_main_menu.c
    src/ui/scenes/scene_setup.c
    src/ui/scenes/scene_life_origin.c
    src/ui/scenes/scene_spawn_select.c
    src/ui/scenes/scene_game.c
)

# Widget library sources
set(WIDGET_SOURCES
    src/ui/widget/widget_base.c
    src/ui/widget/button.c
    src/ui/widget/panel.c
    src/ui/widget/progress_bar.c
    src/ui/window_mgr.c
    src/ui/widget/drawer.c
    src/ui/widget/tooltip.c
    src/ui/widget/scroll_area.c
)

# Layout engine sources
set(LAYOUT_SOURCES
    src/ui/layout/layout.c
)

# Graph rendering sources
set(GRAPH_SOURCES
    src/ui/graph/graph.c
)

# Icon system sources
set(ICON_SOURCES
    src/ui/icon/icon_atlas.c
)

# Panel sources
set(PANEL_SOURCES
    src/ui/panel/diplomacy_panel.c
    src/ui/panel/research_panel.c
    src/ui/panel/governance_panel.c
    src/ui/panel/wonders_panel.c
    src/ui/panel/rulebook_panel.c
    src/ui/panel/unit_sidebar.c
    src/ui/panel/settlement_sidebar.c
)

# Core game sources
set(CORE_SOURCES
    src/core/game.c
    src/core/profile.c
    src/core/data/history_db.c
    src/core/governance/legislative_system.c
    src/core/governance/institution.c
    src/core/governance/subdivision.c
    src/core/world/settlement_manager.c
    src/core/culture/ideology_system.c
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
    src/core/world/real_world_map.c
    src/core/world/map_view.c
    src/core/world/territory.c
    src/core/world/wonders.c
    src/core/world/nation.c
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
    src/core/faction.c
    src/core/character.c
    src/core/time_engine.c
    src/core/knowledge_system.c
)

# Utils sources
set(UTILS_SOURCES
    src/utils/common.c
    src/utils/types.c
    src/utils/memory_pool.c
    src/utils/config.c
    src/utils/cache.c
    src/core/visuals/vexillology.c
    src/utils/noise.c
)

# Create main executable
add_executable(dominion
    ${ENGINE_SOURCES}
    ${DISPLAY_SOURCES}
    ${UI_SOURCES}
    ${WIDGET_SOURCES}
    ${LAYOUT_SOURCES}
    ${GRAPH_SOURCES}
    ${ICON_SOURCES}
    ${PANEL_SOURCES}
    ${CORE_SOURCES}
    ${UTILS_SOURCES}
)

# Link SDL3 libraries via pkg-config imported targets
target_link_libraries(dominion PRIVATE
    PkgConfig::SDL3
    PkgConfig::SDL3_TTF
    m
)

# Add SDL3 include directories from pkg-config
target_include_directories(dominion PRIVATE
    ${SDL3_INCLUDE_DIRS}
    ${SDL3_TTF_INCLUDE_DIRS}
)

# Compiler flags
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(dominion PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Debug flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(dominion PRIVATE DEBUG=1)
    target_compile_options(dominion PRIVATE -g -O0)
endif()

# Release flags - OPTIMIZED
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(dominion PRIVATE NDEBUG)
    target_compile_options(dominion PRIVATE
        -O3 -DNDEBUG -flto -march=native
        -mtune=native -fomit-frame-pointer
        -ffast-math -funroll-loops
    )
    target_link_options(dominion PRIVATE -flto)
endif()

message(STATUS "========================================")
message(STATUS "DOMINION - SDL3 Build Configuration")
message(STATUS "========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "SDL3 includes: ${SDL3_INCLUDE_DIRS}")
message(STATUS "SDL3 libraries: ${SDL3_LIBRARIES}")
message(STATUS "SDL3_ttf includes: ${SDL3_TTF_INCLUDE_DIRS}")
message(STATUS "SDL3_ttf libraries: ${SDL3_TTF_LIBRARIES}")
message(STATUS "========================================")
