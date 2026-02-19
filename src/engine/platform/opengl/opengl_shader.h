#pragma once

#include <string>
#include <unordered_map>

#include "glad/gl.h"
#include "glm/ext/matrix_float3x3.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "renderer/shader.h"

namespace ck {
class OpenGLShader : public Shader {
public:
  OpenGLShader(const std::string& filepath);
  OpenGLShader(const std::string& name, const std::string& vertex_source,
               const std::string& fragment_source);
  virtual ~OpenGLShader();

  void Bind() const override;
  void Unbind() const override;

  const std::string& Name() const override;

  void SetInt(const std::string& name, int) const override;
  void SetIntArray(const std::string& name, int* values, uint32_t count) override;
  void SetFloat(const std::string& name, float value) const override;
  void SetFloat2(const std::string& name, const glm::vec2&) const override;
  void SetFloat3(const std::string& name, const glm::vec3&) const override;
  void SetFloat4(const std::string& name, const glm::vec4&) const override;
  void SetMat4(const std::string& name, const glm::mat4&) const override;

  void UploadUniformInt(const std::string& name, int) const;
  void UploadUniformIntArray(const std::string& name, int* values, uint32_t count) const;

  void UploadUniformFloat(const std::string& name, float) const;
  void UploadUniformFloat2(const std::string& name, const glm::vec2&) const;
  void UploadUniformFloat3(const std::string& name, const glm::vec3&) const;
  void UploadUniformFloat4(const std::string& name, const glm::vec4&) const;

  void UploadUniformMat3(const std::string& name, const glm::mat3&) const;
  void UploadUniformMat4(const std::string& name, const glm::mat4&) const;

private:  // ReadFile -- Parse -- Compile
  std::string ReadFile(const std::string& filepath);

  // Spile source to 2 parts --> vertex shader source & fragment shader source
  // by syntax `#type vertex` or `#type fragment`
  std::unordered_map<GLenum, std::string> Parse(const std::string& source);

  void Compile(const std::unordered_map<GLenum, std::string>& shader_sources);

private:
  uint32_t renderer_id_;
  std::string name_;
};
}  // namespace ck
