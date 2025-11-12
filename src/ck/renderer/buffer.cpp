#include "buffer.h"

#include "log.h"
#include "platform/opengl/opengl_buffer.h"
#include "renderer/renderer.h"

namespace ck {
std::unique_ptr<IndexBuffer> IndexBuffer::Create(uint32_t* vertices, uint32_t count) {
  switch (Renderer::API()) {
    case RendererAPI::kNone:
      CK_ENGINE_ERROR("RendererAPI::kNone is currently not supported!");
      return nullptr;

    case RendererAPI::kOpenGL:
      return std::make_unique<OpenGLIndexBuffer>(vertices, count);
  }

  CK_ENGINE_ASSERT(false, "unknown RendererAPI");
  return nullptr;
}

std::unique_ptr<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size) {
  switch (Renderer::API()) {
    case RendererAPI::kNone:
      CK_ENGINE_ERROR("RendererAPI::kNone is currently not supported!");
      return nullptr;

    case RendererAPI::kOpenGL:
      return std::make_unique<OpenGLVertexBuffer>(vertices, size);
  }

  CK_ENGINE_ASSERT(false, "unknown RendererAPI");
  return nullptr;
}
}  // namespace ck
