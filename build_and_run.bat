@echo off
echo Building Civilization (Win32 GUI)...

REM Create build directory if it doesn't exist
if not exist build mkdir build

REM Compile
gcc src/ui/win32_app.c ^
    src/core/game.c ^
    src/core/economy/currency_system.c ^
    src/core/economy/trade_system.c ^
    src/core/environment/disaster_system.c ^
    src/core/events/game_events.c ^
    src/core/world/map_generator.c ^
    src/core/world/map_view.c ^
    src/core/world/settlement_manager.c ^
    src/core/world/territory.c ^
    src/core/world/dynamic_borders.c ^
    src/core/population/population_manager.c ^
    src/core/technology/innovation_system.c ^
    src/core/military/combat.c ^
    src/core/military/units.c ^
    src/core/military/conquest.c ^
    src/core/diplomacy/relations.c ^
    src/core/diplomacy/international_organizations.c ^
    src/core/politics/politics.c ^
    src/core/culture/culture.c ^
    src/core/culture/ideology_system.c ^
    src/core/governance/government.c ^
    src/core/governance/legislative_system.c ^
    src/core/governance/custom_governance.c ^
    src/core/simulation_engine/time_manager.c ^
    src/core/simulation_engine/system_orchestrator.c ^
    src/core/simulation_engine/performance_optimizer.c ^
    src/core/ai/ai_system.c ^
    src/core/subunits/subunit.c ^
    src/core/visualization/cultural_display.c ^
    src/core/utils/cache.c ^
    src/core/utils/config.c ^
    src/core/utils/memory_pool.c ^
    -o build/civilization_gui.exe ^
    -I include ^
    -lgdi32 -mwindows

if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b %errorlevel%
)

echo Build successful! Launching game...
start build/civilization_gui.exe
