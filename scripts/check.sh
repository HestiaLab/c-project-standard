#!/usr/bin/env sh
set -eu

PRESET="${1:-debug}"
ENABLE_TIDY="${2:-}"

if command -v clang-format >/dev/null 2>&1; then
  find include src tests -type f \( -name '*.c' -o -name '*.h' -o -name '*.cpp' \) -print0 \
    | xargs -0 clang-format --dry-run --Werror
else
  echo "Skipping clang-format: command not found."
fi

cmake --preset "$PRESET"
cmake --build --preset "$PRESET"

if [ "$ENABLE_TIDY" = "tidy" ] || [ "$ENABLE_TIDY" = "--tidy" ]; then
  if command -v clang-tidy >/dev/null 2>&1; then
    BUILD_DIR="build/$PRESET"
    GENERATED_INCLUDE_DIR="$BUILD_DIR/generated/include"
    if [ -f "$BUILD_DIR/compile_commands.json" ]; then
      find src tests \( -path tests/package_smoke -o -path tests/subproject_smoke \) -prune -o -type f -name '*.c' \
        -exec clang-tidy --warnings-as-errors='*' -p "$BUILD_DIR" {} \;
    else
      find src tests \( -path tests/package_smoke -o -path tests/subproject_smoke \) -prune -o -type f -name '*.c' \
        -exec clang-tidy --warnings-as-errors='*' --extra-arg=-std=c11 {} -- -Iinclude -I"$GENERATED_INCLUDE_DIR" \;
    fi
  else
    echo "Skipping clang-tidy: command not found."
  fi
fi

ctest --preset "$PRESET" --output-on-failure
