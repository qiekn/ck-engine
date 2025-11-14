#include "shader.h"

#include <memory>

#include "log.h"
#include "platform/opengl/opengl_shader.h"
#include "renderer/renderer.h"
#include "renderer/renderer_api.h"

namespace ck {

Scope<Shader> Shader::Create(const std::string& vertex_source, const std::string& fragment_source) {
  switch (Renderer::API()) {
    case RendererAPI::Type::kOpenGL:
      return std::make_unique<OpenGLShader>(vertex_source, fragment_source);

    default:
      CK_ENGINE_ASSERT(false, "Unknown RendererAPI!");
      return nullptr;
  }
}

Scope<Shader> Shader::Create(const std::string& filepath) {
  switch (RendererAPI::GetAPI()) {
    case RendererAPI::Type::kOpenGL:
      return std::make_unique<OpenGLShader>(filepath);

    default:
      CK_ENGINE_ASSERT(false, "unknown RendererAPI");
      return nullptr;
  }
}
}  // namespace ck
