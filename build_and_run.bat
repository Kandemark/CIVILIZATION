@echo off
setlocal enabledelayedexpansion

echo ==========================================
echo Civilization Simulation Build Script
echo ==========================================

REM Check for GCC
where gcc >nul 2>nul
if %errorlevel% equ 0 (
    echo [INFO] GCC detected. Building with MinGW...
    if not exist build mkdir build
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
        src/utils/cache.c ^
        src/utils/config.c ^
        src/utils/memory_pool.c ^
        src/utils/common.c ^
        src/utils/types.c ^
        src/utils/noise.c ^
        -o build/civilization_gui.exe ^
        -I include ^
        -lgdi32 -mwindows
        
    if !errorlevel! neq 0 goto :fail
    goto :success
)

REM Check for CL (MSVC) already in path
where cl >nul 2>nul
if %errorlevel% equ 0 goto :build_msvc

REM Try to find MSVC
echo [INFO] GCC not found. Searching for Visual Studio...
set "vcvars="
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "vs_path=%%i"
    if exist "!vs_path!\VC\Auxiliary\Build\vcvars64.bat" (
        set "vcvars=!vs_path!\VC\Auxiliary\Build\vcvars64.bat"
    )
)

if defined vcvars (
    echo [INFO] Found Visual Studio. Setting up environment...
    call "!vcvars!"
    goto :build_msvc
)

echo [ERROR] No C compiler found!
echo Please install one of the following:
echo   1. MinGW-w64 (GCC) - https://www.mingw-w64.org/
echo   2. Visual Studio Community - https://visualstudio.microsoft.com/vs/community/
echo.
pause
exit /b 1

:build_msvc
echo [INFO] Building with MSVC...
if not exist build mkdir build
if not exist obj mkdir obj
cl /nologo /W3 /D_CRT_SECURE_NO_WARNINGS /Fo:obj\ ^
    src/ui/win32_app.c ^
    src/core/game.c ^
    src/core/abstracts/soft_metrics.c ^
    src/core/ai/ai_system.c ^
    src/core/ai/base_ai.c ^
    src/core/ai/strategic_ai.c ^
    src/core/ai/tactical_ai.c ^
    src/core/culture/cultural_assimilation.c ^
    src/core/culture/cultural_diffusion.c ^
    src/core/culture/cultural_identity.c ^
    src/core/culture/culture.c ^
    src/core/culture/ideology_system.c ^
    src/core/culture/language_evolution.c ^
    src/core/culture/writing_system.c ^
    src/core/diplomacy/international_organizations.c ^
    src/core/diplomacy/relations.c ^
    src/core/economy/currency_system.c ^
    src/core/economy/market.c ^
    src/core/economy/trade_system.c ^
    src/core/environment/disaster_system.c ^
    src/core/environment/geography.c ^
    src/core/events/event_manager.c ^
    src/core/events/game_events.c ^
    src/core/governance/custom_governance.c ^
    src/core/governance/government.c ^
    src/core/governance/legislative_system.c ^
    src/core/governance/rule_system.c ^
    src/core/military/combat.c ^
    src/core/military/conquest.c ^
    src/core/military/units.c ^
    src/core/politics/faction_system.c ^
    src/core/politics/politics.c ^
    src/core/population/demographics.c ^
    src/core/population/population_manager.c ^
    src/core/simulation_engine/event_dispatcher.c ^
    src/core/simulation_engine/performance_optimizer.c ^
    src/core/simulation_engine/state_persistence.c ^
    src/core/simulation_engine/system_orchestrator.c ^
    src/core/simulation_engine/time_manager.c ^
    src/core/subunits/subunit.c ^
    src/core/technology/innovation_system.c ^
    src/core/visualization/cultural_display.c ^
    src/core/world/dynamic_borders.c ^
    src/core/world/map_generator.c ^
    src/core/world/map_view.c ^
    src/core/world/settlement_manager.c ^
    src/core/world/territory.c ^
    src/utils/cache.c ^
    src/utils/config.c ^
    src/utils/memory_pool.c ^
    src/utils/common.c ^
    src/utils/types.c ^
    src/utils/noise.c ^
    /Fe:build\civilization_gui.exe ^
    /I include ^
    user32.lib gdi32.lib

if %errorlevel% neq 0 goto :fail
goto :success

:fail
echo [ERROR] Build failed!
pause
exit /b 1

:success
echo [SUCCESS] Build complete! Launching game...
start build\civilization_gui.exe
