#include "vertex_array.h"

#include <memory>

#include "platform/opengl/opengl_vertex_array.h"
#include "renderer/renderer_api.h"

namespace ck {

Scope<VertexArray> VertexArray::Create() {
  switch (RendererAPI::GetAPI()) {
    case RendererAPI::Type::kNone:
      return nullptr;

    case RendererAPI::Type::kOpenGL:
      return std::make_unique<OpenglVertexArray>();
  }

  CK_ENGINE_ASSERT(false, "unknown RendererAPI");
  return nullptr;
}

}  // namespace ck
