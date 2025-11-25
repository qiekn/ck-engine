#pragma once

#include <string>
#include <unordered_map>

#include "core/core.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"

namespace ck {
class Shader {
public:
  virtual ~Shader() = default;

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  virtual const std::string& Name() const = 0;

  virtual void SetInt(const std::string& name, int) const = 0;
  virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;
  virtual void SetFloat(const std::string& name, float) const = 0;
  virtual void SetFloat3(const std::string& name, const glm::vec3&) const = 0;
  virtual void SetFloat4(const std::string& name, const glm::vec4&) const = 0;
  virtual void SetMat4(const std::string& name, const glm::mat4&) const = 0;

  static Scope<Shader> Create(const std::string& name, const std::string& vertex_source,
                              const std::string& fragment_source);
  static Scope<Shader> Create(const std::string& filepath);

private:
  uint32_t renderer_id_;
};

class ShaderLibrary {
public:
  // Add to shaders_
  void Add(Ref<Shader>);
  void Add(const std::string& name, Ref<Shader>);

  // Load form file
  Ref<Shader> Load(const std::string& filepath);
  Ref<Shader> Load(const std::string& name, const std::string& filepath);

  Ref<Shader> Get(const std::string& name);

  bool IsExist(const std::string& name) const;

private:
  std::unordered_map<std::string, Ref<Shader>> shaders_;
};
}  // namespace ck
