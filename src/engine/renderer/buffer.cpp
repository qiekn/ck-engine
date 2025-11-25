#include "buffer.h"

#include <memory>

#include "core/core.h"
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

Ref<VertexBuffer> VertexBuffer::Create(uint32_t size) {
  switch (RendererAPI::GetAPI()) {
    case RendererAPI::Type::kNone:
      CK_ENGINE_ERROR("RendererAPI::kNone is currently not supported!");
      return nullptr;
    case RendererAPI::Type::kOpenGL:
      return CreateRef<OpenGLVertexBuffer>(size);

      CK_ENGINE_ASSERT(false, "unknown RendererAPI");
      return nullptr;
  }
}

Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size) {
  switch (RendererAPI::GetAPI()) {
    case RendererAPI::Type::kNone:
      CK_ENGINE_ERROR("RendererAPI::kNone is currently not supported!");
      return nullptr;

    case RendererAPI::Type::kOpenGL:
      return CreateRef<OpenGLVertexBuffer>(vertices, size);
  }

  CK_ENGINE_ASSERT(false, "unknown RendererAPI");
  return nullptr;
}
}  // namespace ck
