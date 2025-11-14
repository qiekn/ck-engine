#pragma once

#include "glm/fwd.hpp"
#include "renderer/renderer_api.h"
#include "renderer/vertex_array.h"

namespace ck {
class RenderCommand {
public:
  inline static void Init() { renderer_api_->Init(); }

  inline static void SetClearColor(const glm::vec4& color) { renderer_api_->SetClearColor(color); }

  inline static void Clear() { renderer_api_->Clear(); }

  inline static void DrawIndexed(const VertexArray* vertex_array) {
    renderer_api_->DrawIndexed(vertex_array);
  }

private:
  static RendererAPI* renderer_api_;
};
}  // namespace ck
