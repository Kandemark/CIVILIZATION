@echo off
REM Cleanup script to remove bloat and unnecessary files

echo Cleaning up CIVILIZATION project...

REM Remove build artifacts
echo Removing build artifacts...
if exist build rmdir /s /q build
if exist obj rmdir /s /q obj

REM Remove generated images
echo Removing generated PPM images...
del /q *.ppm 2>nul

REM Remove old executables
echo Removing old executables...
del /q *.exe 2>nul

REM Remove log files
echo Removing log files...
del /q *_log.txt 2>nul
del /q build_*.txt 2>nul
del /q full_log.txt 2>nul
del /q launcher_log.txt 2>nul

REM Remove redundant optimization docs (keeping only the main one)
echo Removing redundant documentation...
del /q OPTIMIZATION_SUMMARY.md 2>nul
del /q README_OPTIMIZATIONS.md 2>nul

REM Remove old batch scripts (if not needed)
echo Removing old batch scripts...
del /q build_sim.bat 2>nul
del /q debug.bat 2>nul
del /q game_launcher.bat 2>nul

echo.
echo Cleanup complete!
echo.
echo Removed:
echo - Build artifacts (build/, obj/)
echo - Generated images (*.ppm)
echo - Old executables (*.exe)
echo - Log files (*_log.txt)
echo - Redundant documentation
echo - Old batch scripts
echo.
echo Kept:
echo - Source code (src/, include/)
echo - Main documentation (docs/)
echo - Build configuration (CMakeLists.txt)
echo - Build script (build_and_run.bat)
echo.
pause
