#pragma once

// #include "pch.h"
#include "core.h"

namespace ck {
class Input {
public:
  virtual ~Input() = default;

  static bool IsKeyPressed(int key_code) { return instance_->IsKeyPressedImpl(key_code); }
  static bool IsMouseButtonPressed(int button) {
    return instance_->IsMouseButtonPressedImpl(button);
  }

  static std::pair<float, float> GetMousePos() { return instance_->GetMousePosImpl(); }
  static float GetMouseX() { return instance_->GetMouseXImpl(); }
  static float GetMouseY() { return instance_->GetMouseYImpl(); }

protected:
  virtual bool IsKeyPressedImpl(int key_code) const = 0;
  virtual bool IsMouseButtonPressedImpl(int button) const = 0;
  virtual std::pair<float, float> GetMousePosImpl() const = 0;
  virtual float GetMouseXImpl() const = 0;
  virtual float GetMouseYImpl() const = 0;

private:
  static Scope<Input> instance_;
};
}  // namespace ck
