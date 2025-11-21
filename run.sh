#!/bin/bash

DIR="build"

if [ "$1" = "debug" ]; then
  lldb ./build/sandbox
else
  cmake --build ${DIR} -j$(nproc) && ./${DIR}/sandbox
fi

# vim: ft=sh ts=2 sw=2 et
