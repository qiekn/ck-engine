#pragma once

#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float4.hpp"
#include "renderer/orthographic_camera.h"

namespace ck {
class Renderer2D {
public:
  static void Init();
  static void Shutdown();

  static void BeginScene(const OrthographicCamera&);
  static void EndScene();

  // Primitives (基本图元)
  static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
  static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
};
}  // namespace ck
