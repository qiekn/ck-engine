#include "buffer.h"

#include "core/log.h"
#include "platform/opengl/opengl_buffer.h"
#include "renderer/renderer_api.h"

namespace ck {

/*─────────────────────────────────────┐
│             Index Buffer             │
└──────────────────────────────────────*/

Scope<IndexBuffer> IndexBuffer::Create(uint32_t* vertices, uint32_t count) {
  switch (RendererAPI::GetAPI()) {
    case RendererAPI::Type::kNone:
      CK_ENGINE_ERROR("RendererAPI::kNone is currently not supported!");
      return nullptr;

    case RendererAPI::Type::kOpenGL:
      return std::make_unique<OpenGLIndexBuffer>(vertices, count);
  }

  CK_ENGINE_ASSERT(false, "unknown RendererAPI");
  return nullptr;
}

/*─────────────────────────────────────┐
│            Vertex Buffer             │
└──────────────────────────────────────*/

Scope<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size) {
  switch (RendererAPI::GetAPI()) {
    case RendererAPI::Type::kNone:
      CK_ENGINE_ERROR("RendererAPI::kNone is currently not supported!");
      return nullptr;

    case RendererAPI::Type::kOpenGL:
      return std::make_unique<OpenGLVertexBuffer>(vertices, size);
  }

  CK_ENGINE_ASSERT(false, "unknown RendererAPI");
  return nullptr;
}
}  // namespace ck
