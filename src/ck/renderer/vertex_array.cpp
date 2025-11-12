#include "vertex_array.h"

#include <memory>

#include "platform/opengl/opengl_vertex_array.h"
#include "renderer/renderer_api.h"

namespace ck {

std::unique_ptr<VertexArray> VertexArray::Create() {
  switch (RendererAPI::API()) {
    case RendererAPI::ApiType::kNone:
      return nullptr;

    case RendererAPI::ApiType::kOpenGL:
      return std::make_unique<OpenglVertexArray>();
  }

  CK_ENGINE_ASSERT(false, "unknown RendererAPI");
  return nullptr;
}

}  // namespace ck
