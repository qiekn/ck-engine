#include "texture.h"

#include <memory>

#include "platform/opengl/opengl_texture.h"
#include "renderer/renderer_api.h"

namespace ck {

Scope<Texture2D> Texture2D::Create(const std::string& path) {
  switch (RendererAPI::GetAPI()) {
    case RendererAPI::Type::kOpenGL:
      return std::make_unique<OpenGLTexture2D>(path);
    default:
      CK_ENGINE_ASSERT(false, "Unknown RendererAPI!");
      return nullptr;
  }
}

Scope<Texture2D> Texture2D::Create(uint32_t width, uint32_t height) {
  switch (RendererAPI::GetAPI()) {
    case RendererAPI::Type::kOpenGL:
      return std::make_unique<OpenGLTexture2D>(width, height);
    default:
      CK_ENGINE_ASSERT(false, "Unknown RendererAPI!");
      return nullptr;
  }
}

}  // namespace ck
