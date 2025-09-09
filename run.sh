#!/bin/bash

BUILD_DIR="build"

if [ "$1" = "debug" ]; then
  lldb ./build/sandbox
else
  make -C "$BUILD_DIR" && make -j$(nproc) -C "$BUILD_DIR" && ./"$BUILD_DIR"/sandbox
fi

# vim: ft=sh ts=2 sw=2 et
