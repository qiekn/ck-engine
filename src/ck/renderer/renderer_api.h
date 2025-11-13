#pragma once

#include "glm/fwd.hpp"
#include "renderer/vertex_array.h"
namespace ck {

class RendererAPI {
public:
  enum class Type { kNone = 0, kOpenGL = 1 };  // API type

  virtual void Clear() = 0;
  virtual void SetClearColor(const glm::vec4&) = 0;
  virtual void DrawIndexed(const VertexArray*) = 0;

  static Type GetAPI() { return api_; }

private:
  static Type api_;
};
}  // namespace ck
