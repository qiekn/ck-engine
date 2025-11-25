#pragma once

#include "renderer/renderer_api.h"

namespace ck {
class OpenGLRendererAPI : public RendererAPI {
public:
  void Init() override;
  void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
  void Clear() override;
  void SetClearColor(const glm::vec4&) override;
  void DrawIndexed(const VertexArray*, uint32_t index_count) override;
};
}  // namespace ck
