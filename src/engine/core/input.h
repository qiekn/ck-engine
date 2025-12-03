#pragma once

#include "events/key_codes.h"
#include "events/mouse_codes.h"

namespace ck {
class Input {
public:
  static bool IsKeyPressed(KeyCode key);
  static bool IsMouseButtonPressed(MouseCode button);
  static std::pair<float, float> GetMousePos();
  static float GetMouseX();
  static float GetMouseY();
};
}  // namespace ck
