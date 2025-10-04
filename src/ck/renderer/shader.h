#pragma once

#include <string>

namespace ck {
class Shader {
public:
  Shader(const std::string& vertex_source, const std::string& fragment_source);
  virtual ~Shader();

  void Bind() const;
  void Unbind() const;

private:
  uint32_t renderer_id_;
};
}  // namespace ck
