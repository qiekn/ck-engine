#include "shader.h"

#include <memory>
#include <string>
#include <utility>

#include "core/core.h"
#include "core/log.h"
#include "platform/opengl/opengl_shader.h"
#include "renderer/renderer.h"
#include "renderer/renderer_api.h"

namespace ck {
/*─────────────────────────────────────┐
│                Shader                │
└──────────────────────────────────────*/

Scope<Shader> Shader::Create(const std::string& name, const std::string& vertex_source,
                             const std::string& fragment_source) {
  switch (Renderer::API()) {
    case RendererAPI::Type::kOpenGL:
      return std::make_unique<OpenGLShader>(name, vertex_source, fragment_source);

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

/*─────────────────────────────────────┐
│            ShaderLibrary             │
└──────────────────────────────────────*/

void ShaderLibrary::Add(Ref<Shader> shader) {
  auto& name = shader->Name();
  Add(name, std::move(shader));
}

void ShaderLibrary::Add(const std::string& name, Ref<Shader> shader) {
  CK_ENGINE_ASSERT(!IsExist(name), "{} shader already exists", name);
  shaders_[name] = std::move(shader);
}

Ref<Shader> ShaderLibrary::Load(const std::string& filepath) {
  Ref<Shader> shader = Shader::Create(filepath);
  Add(shader);
  return shader;
}
Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filepath) {
  Ref<Shader> shader = Shader::Create(filepath);
  Add(name, shader);
  return shader;
}

Ref<Shader> ShaderLibrary::Get(const std::string& name) {
  CK_ENGINE_ASSERT(IsExist(name), "{} shader not found", name);
  return shaders_[name];
}

bool ShaderLibrary::IsExist(const std::string& name) const {
  return shaders_.find(name) != shaders_.end();
}
}  // namespace ck
