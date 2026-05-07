#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace ck {

void Camera::SetViewport(uint32_t width, uint32_t height) {
  width_ = width == 0 ? 1u : width;
  height_ = height == 0 ? 1u : height;
  Recompute();
}

void Camera::SetPosition(const glm::vec3& position) {
  position_ = position;
  Recompute();
}

void Camera::Recompute() {
  float aspect = static_cast<float>(width_) / static_cast<float>(height_);

  // GL-style ortho (Y up). Flip Y on the projection's Y scale to match
  // Vulkan's Y-down NDC.
  glm::mat4 projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f);
  projection[1][1] *= -1.0f;

  glm::mat4 view = glm::translate(glm::mat4(1.0f), -position_);
  view_projection_ = projection * view;
}

}  // namespace ck
