#pragma once

// PCH - Precompiled Header files

// IWYU -> Include What You Use
// see: https://clangd.llvm.org/guides/include-cleaner

// clang-format off
#include <iostream>      // IWYU pragma: export
#include <memory>        // IWYU pragma: export
#include <utility>       // IWYU pragma: export
#include <algorithm>     // IWYU pragma: export
#include <functional>    // IWYU pragma: export


#include <string>        // IWYU pragma: export
#include <format>        // IWYU pragma: export
#include <sstream>       // IWYU pragma: export
#include <vector>        // IWYU pragma: export
#include <unordered_map> // IWYU pragma: export
#include <unordered_set> // IWYU pragma: export

#ifdef CK_PLATFORM_WINDOWS
  #include <Windows.h>
#endif
// clang-format on
