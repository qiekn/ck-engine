#include "uniform_buffer.h"

#include "platform/opengl/opengl_uniform_buffer.h"
#include "renderer/renderer_api.h"

namespace ck {

Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding) {
  switch (RendererAPI::GetAPI()) {
    case RendererAPI::Type::kNone:
      CK_ENGINE_ASSERT(false, "RendererAPI::kNone is currently not supported!");
      return nullptr;
    case RendererAPI::Type::kOpenGL:
      return CreateRef<OpenglUniformBuffer>(size, binding);
    default:
      CK_ENGINE_ASSERT(false, "Unknown RendererAPI!");
      return nullptr;
  }
}

}  // namespace ck
