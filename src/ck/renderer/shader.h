#pragma once

#include <string>

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float4.hpp"

namespace ck {
class Shader {
public:
  Shader(const std::string& vertex_source, const std::string& fragment_source);
  virtual ~Shader();

  void Bind() const;
  void Unbind() const;

  void UploadUniformFloat4(const std::string& name, const glm::vec4&) const;
  void UploadUniformMat4(const std::string& name, const glm::mat4&) const;

private:
  uint32_t renderer_id_;
};
}  // namespace ck
