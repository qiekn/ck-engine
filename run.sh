#!/bin/bash

PRESET="${PRESET:-debug}"
DEBUGGER="gdb"
TARGET="editor"
BIN_DIR="build/$PRESET"

# Auto-configure if build directory doesn't exist
if [ ! -d "$BIN_DIR" ]; then
  echo "Build directory not found, running cmake configure with preset $PRESET..."
  cmake --preset "$PRESET"
fi

if [ "$1" = "debug" ]; then
  cmake --build --preset "$PRESET" && $DEBUGGER ./"$BIN_DIR"/$TARGET
else
  cmake --build --preset "$PRESET" && ./"$BIN_DIR"/"$TARGET"
fi

# vim: ft=sh ts=2 sw=2 et