#pragma once

#include <memory>
#define BIT(x) (1 << x)

namespace ck {
template <typename T>
using Scope = std::unique_ptr<T>;

template <typename T>
using Ref = std::shared_ptr<T>;
}  // namespace ck
