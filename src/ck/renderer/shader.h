#pragma once

#include <memory>
#include <string>

namespace ck {
class Shader {
public:
  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  static std::unique_ptr<Shader> Create(const std::string& vertex_source,
                                        const std::string& fragment_source);

private:
  uint32_t renderer_id_;
};
}  // namespace ck
