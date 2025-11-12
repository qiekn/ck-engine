#include "vertex_array.h"

#include <memory>

#include "platform/opengl/opengl_vertex_array.h"
#include "renderer/renderer.h"

namespace ck {

std::unique_ptr<VertexArray> VertexArray::Create() {
  switch (Renderer::API()) {
    case RendererAPI::kNone:
      return nullptr;

    case RendererAPI::kOpenGL:
      return std::make_unique<OpenglVertexArray>();
  }

  CK_ENGINE_ASSERT(false, "unknown RendererAPI");
  return nullptr;
}

}  // namespace ck
