#pragma once

#include <cstdint>

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

namespace std {
template <typename T>
struct hash;

template <>
struct hash<ck::UUID> {
  std::size_t operator()(const ck::UUID& uuid) const {
    return (uint64_t)uuid;
  }
};
}  // namespace std
