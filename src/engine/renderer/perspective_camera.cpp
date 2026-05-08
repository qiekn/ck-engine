#include "renderer/perspective_camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace ck {

void PerspectiveCamera::SetViewport(uint32_t width, uint32_t height) {
  width_ = width == 0 ? 1u : width;
  height_ = height == 0 ? 1u : height;
  Recompute();
}

void PerspectiveCamera::SetPerspective(float fov, float near_plane, float far_plane) {
  fov_ = fov;
  near_ = near_plane;
  far_ = far_plane;
  Recompute();
}

void PerspectiveCamera::SetView(const glm::vec3& position, const glm::vec3& target,
                                const glm::vec3& up) {
  position_ = position;
  target_ = target;
  up_ = up;
  Recompute();
}

void PerspectiveCamera::Recompute() {
  float aspect = static_cast<float>(width_) / static_cast<float>(height_);
  glm::mat4 projection = glm::perspective(fov_, aspect, near_, far_);
  // Vulkan Y-down NDC: flip projection's Y to keep "+Y up" world space.
  projection[1][1] *= -1.0f;
  glm::mat4 view = glm::lookAt(position_, target_, up_);
  view_projection_ = projection * view;
}

}  // namespace ck
