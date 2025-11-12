#include "orthographic_camera.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/trigonometric.hpp"

namespace ck {

OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
    : projection_matrix_(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)),
      view_matrix_(1.0f),
      view_proj_matrix_(1.0f) {
  view_proj_matrix_ = projection_matrix_ * view_matrix_;
}

OrthographicCamera::~OrthographicCamera() {}

void OrthographicCamera::RecalculateViewMatrix() {
  glm::mat4 translation = glm::translate(glm::mat4(1.0f), position_);
  glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(rotation_), glm::vec3(0, 0, 1));

  translation = translation * rotation;
  view_matrix_ = glm::inverse(translation);
  view_proj_matrix_ = projection_matrix_ * view_matrix_;
}

void OrthographicCamera::SetRotation(float rotation) {
  rotation_ = rotation;
  RecalculateViewMatrix();
}
float OrthographicCamera::GetRotation() const { return rotation_; }
void OrthographicCamera::SetPosition(const glm::vec3& position) {
  position_ = position;
  RecalculateViewMatrix();
}
const glm::vec3& OrthographicCamera::GetPosition() const { return position_; }
}  // namespace ck
