#pragma once

#include "core/core.h"
#include "renderer/vertex_array.h"

#include "glm/fwd.hpp"

#include <cstdint>
namespace ck {

class RendererAPI {
public:
  enum class Type { kNone = 0, kOpenGL = 1 };  // API type

  virtual void Init() = 0;
  virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
  virtual void Clear() = 0;
  virtual void SetClearColor(const glm::vec4&) = 0;
  virtual void DrawIndexed(const Ref<VertexArray>& vertex_array, uint32_t index_count = 0) = 0;
  virtual void DrawLines(const Ref<VertexArray>& vertex_array, uint32_t vertex_count) = 0;

  virtual void SetLineWidth(float width) = 0;

  static Type GetAPI() { return api_; }

private:
  static Type api_;
};
}  // namespace ck
