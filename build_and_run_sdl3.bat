@echo off
REM SDL3 Civilization Build and Run Script
REM This script builds and runs the SDL3 version of Civilization

echo ========================================
echo CIVILIZATION - SDL3 Build Script
echo ========================================
echo.

REM Check for MinGW
where gcc >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: gcc not found in PATH!
    echo Please install MinGW-w64 or add it to your PATH.
    echo Download from: https://www.mingw-w64.org/
    pause
    exit /b 1
)

echo Found GCC compiler
gcc --version | findstr "gcc"
echo.

REM Check for MinGW make
set MAKE_CMD=
where mingw32-make >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    set MAKE_CMD=mingw32-make
) else (
    where make >nul 2>&1
    if %ERRORLEVEL% EQU 0 (
        set MAKE_CMD=make
    ) else (
        echo ERROR: make not found in PATH!
        echo Please install MinGW-w64 or add it to your PATH.
        pause
        exit /b 1
    )
)

echo Found make: %MAKE_CMD%
echo.

REM Clean previous build
echo Cleaning previous build...
if exist build rmdir /s /q build
echo.

REM Build
echo Building CIVILIZATION (Release)...
%MAKE_CMD% release
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build successful!
echo ========================================
echo.
echo Executable: build\civilization.exe
echo.

REM Ask to run
set /p RUN_NOW="Run the game now? (Y/N): "
if /i "%RUN_NOW%"=="Y" (
    echo.
    echo Starting CIVILIZATION...
    echo Press ESC to quit, F11 to toggle fullscreen
    echo.
    cd build
    civilization.exe
    cd ..
)

echo.
echo Done!
pause
