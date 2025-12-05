#pragma once

#include "glm/ext/matrix_float4x4.hpp"

namespace ck {
class Camera {
public:
  Camera() = default;

  explicit Camera(const glm::mat4& projection) : projection_(projection) {}

  virtual ~Camera() = default;

  const glm::mat4& GetProjection() const { return projection_; }

protected:
  glm::mat4 projection_;
};
}  // namespace ck
