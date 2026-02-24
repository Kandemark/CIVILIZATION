#!/usr/bin/env bash
set -euo pipefail
cmake -S . -B build-smoke -DCMAKE_BUILD_TYPE=Debug
cmake --build build-smoke -j
