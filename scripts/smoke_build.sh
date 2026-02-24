#!/usr/bin/env bash
set -euo pipefail

python3 scripts/verify_source_coverage.py
cmake -S . -B build-smoke -DCMAKE_BUILD_TYPE=Debug
cmake --build build-smoke -j
