# PowerShell script to remove Python files and clean up structure

Write-Host "Cleaning up Python files..." -ForegroundColor Yellow

# Remove Python directories
$pythonDirs = @(
    "geo_simulation",
    "analytics",
    "game_layer",
    "modding",
    "experiments",
    "venv",
    "__pycache__"
)

foreach ($dir in $pythonDirs) {
    if (Test-Path $dir) {
        Write-Host "Removing directory: $dir" -ForegroundColor Red
        Remove-Item -Path $dir -Recurse -Force -ErrorAction SilentlyContinue
    }
}

# Remove Python files in root
Get-ChildItem -Path . -Filter "*.py" -File | Remove-Item -Force
Get-ChildItem -Path . -Filter "*.pyc" -File | Remove-Item -Force

# Remove Python config files
$pythonConfigs = @(
    "pyproject.toml",
    "setup.cfg",
    "requirements.txt"
)

foreach ($file in $pythonConfigs) {
    if (Test-Path $file) {
        Write-Host "Removing file: $file" -ForegroundColor Red
        Remove-Item -Path $file -Force -ErrorAction SilentlyContinue
    }
}

# Remove __pycache__ directories recursively
Get-ChildItem -Path . -Directory -Filter "__pycache__" -Recurse | Remove-Item -Recurse -Force -ErrorAction SilentlyContinue

# Remove .pyc files recursively
Get-ChildItem -Path . -Filter "*.pyc" -Recurse | Remove-Item -Force -ErrorAction SilentlyContinue

Write-Host "Cleanup complete!" -ForegroundColor Green
Write-Host "Remaining structure is C-only." -ForegroundColor Green

