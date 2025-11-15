#pragma once

namespace ck {
class DeltaTime {
public:
  DeltaTime(float time = 0.0f) : time_(time) {}

  float Seconds() const { return time_; }
  float Milliseconds() const { return time_ * 1000.0f; }

  operator float() const { return time_; }

private:
  float time_ = 0.0f;
};
}  // namespace ck
