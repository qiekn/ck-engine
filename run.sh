#!/bin/bash

BUILD="build"
DEBUGGER="gdb"
TARGET="editor"

# Auto-configure if build directory doesn't exist
if [ ! -d "$BUILD" ]; then
  echo "Build directory not found, running cmake configure..."
  cmake -B "$BUILD"
fi

if [ "$1" = "debug" ]; then
  cmake --build "$BUILD" -j && $DEBUGGER ./"$BUILD"/$TARGET
else
  cmake --build "$BUILD" -j && ./"$BUILD"/"$TARGET"
fi

# vim: ft=sh ts=2 sw=2 et
