#pragma once

#include "glm/ext/matrix_float4x4.hpp"

namespace ck {
class Camera {
public:
  explicit Camera(const glm::mat4& projection) : projection_(projection) {}

  const glm::mat4& GetProjection() const { return projection_; }

private:
  glm::mat4 projection_;
};
}  // namespace ck
