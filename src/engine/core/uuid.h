#pragma once

#include <cstdint>
#include <functional>

namespace ck {
class UUID {
public:
  UUID();
  UUID(uint64_t uuid);
  UUID(const UUID&) = default;

  operator uint64_t() const { return uuid_; }

private:
  uint64_t uuid_;
};
}  // namespace ck

template <>
struct std::hash<ck::UUID> {
  std::size_t operator()(const ck::UUID& uuid) const {
    return (uint64_t)uuid;
  }
};