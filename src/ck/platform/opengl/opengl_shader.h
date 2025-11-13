#pragma once

#include <string>

#include "glm/ext/matrix_float3x3.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "renderer/shader.h"

namespace ck {
class OpenGLShader : public Shader {
public:
  OpenGLShader(const std::string& vertex_source, const std::string& fragment_source);
  virtual ~OpenGLShader();

  void Bind() const override;
  void Unbind() const override;

  void UploadUniformInt(const std::string& name, int) const;

  void UploadUniformFloat(const std::string& name, float) const;
  void UploadUniformFlaot2(const std::string& name, const glm::vec2&) const;
  void UploadUniformFlaot3(const std::string& name, const glm::vec3&) const;
  void UploadUniformFlaot4(const std::string& name, const glm::vec4&) const;

  void UploadUniformMat3(const std::string& name, const glm::mat3&) const;
  void UploadUniformMat4(const std::string& name, const glm::mat4&) const;

private:
  uint32_t renderer_id_;
};
}  // namespace ck
