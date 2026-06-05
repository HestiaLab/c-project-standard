#!/usr/bin/env sh
set -eu

PRESET="${1:-debug}"

if command -v clang-format >/dev/null 2>&1; then
  find include src tests -type f \( -name '*.c' -o -name '*.h' \) -print0 \
    | xargs -0 clang-format --dry-run --Werror
else
  echo "Skipping clang-format: command not found."
fi

cmake --preset "$PRESET"
cmake --build --preset "$PRESET"
ctest --preset "$PRESET" --output-on-failure
