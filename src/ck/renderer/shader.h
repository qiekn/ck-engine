#pragma once

#include <string>

#include "core.h"

namespace ck {
class Shader {
public:
  virtual ~Shader() = default;

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  static Scope<Shader> Create(const std::string& vertex_source, const std::string& fragment_source);
  static Scope<Shader> Create(const std::string& filepath);

private:
  uint32_t renderer_id_;
};
}  // namespace ck
