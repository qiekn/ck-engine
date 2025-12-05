#pragma once

#include "events/key_codes.h"
#include "events/mouse_codes.h"
#include "glm/ext/vector_float2.hpp"

namespace ck {
class Input {
public:
  static bool IsKeyPressed(KeyCode key);
  static bool IsMouseButtonPressed(MouseCode button);
  static glm::vec2 GetMousePos();
  static float GetMouseX();
  static float GetMouseY();
};
}  // namespace ck
