#pragma once

#include "renderer/renderer_api.h"

namespace ck {
class OpenGLRendererAPI : public RendererAPI {
public:
  void Init() override;
  void Clear() override;
  void SetClearColor(const glm::vec4&) override;
  void DrawIndexed(const VertexArray*) override;
};
}  // namespace ck
