#include "frame_buffer.h"
#include "core/core.h"
#include "platform/opengl/opengl_frame_buffer.h"
#include "renderer/renderer_api.h"

namespace ck {

Ref<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecification& spec) {
  switch (RendererAPI::GetAPI()) {
    case RendererAPI::Type::kNone:
      CK_ENGINE_ASSERT(false, "RendererAPI::KNone is currently not supported!");
      return nullptr;
    case RendererAPI::Type::kOpenGL:
      return CreateRef<OpenglFrameBuffer>(spec);
    default:
      CK_ENGINE_ASSERT(false, "Unknown RendererAPI!");
      return nullptr;
  }
}

}  // namespace ck
