#pragma once

#include "glm/fwd.hpp"
#include "renderer/vertex_array.h"
namespace ck {

class RendererAPI {
public:
  enum class ApiType { kNone = 0, kOpenGL = 1 };

  virtual void Clear() = 0;
  virtual void SetClearColor(const glm::vec4&) = 0;
  virtual void DrawIndexed(const VertexArray*) = 0;

  static ApiType API() { return api_; }

private:
  static ApiType api_;
};
}  // namespace ck
