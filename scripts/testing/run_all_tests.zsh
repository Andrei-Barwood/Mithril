#!/usr/bin/env zsh
# Run all tests
set -euo pipefail

cd build
ctest --output-on-failure

