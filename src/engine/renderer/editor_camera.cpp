#include "editor_camera.h"
#include <algorithm>
#include "core/input.h"
#include "events/event.h"
#include "events/key_codes.h"
#include "events/mouse_codes.h"
#include "events/mouse_event.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_float.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "glm/trigonometric.hpp"
#include "renderer/camera.h"
#include "scene/components.h"

namespace ck {
// ----------------------------------------------------------------------------: public

EditorCamera::EditorCamera(float fov, float aspect_ratio, float near_clip, float far_clip)
    : fov_(fov),
      aspect_ratio_(aspect_ratio),
      near_clip_(near_clip),
      far_clip_(far_clip),
      Camera(glm::perspective(glm::radians(fov), aspect_ratio, near_clip, far_clip)) {
  UpdateView();
}

void EditorCamera::OnUpdate(DeltaTime dt) {
  // Shortcuts
  if (Input::IsKeyPressed(Key::LeftAlt)) {
    const glm::vec2& mouse_pos{Input::GetMousePos()};
    glm::vec2 delta = (mouse_pos - initial_mouse_pos_) * 0.003f;
    initial_mouse_pos_ = mouse_pos;

    if (Input::IsMouseButtonPressed(Mouse::ButtonMiddle)) {
      MousePan(delta);
    } else if (Input::IsMouseButtonPressed(Mouse::ButtonLeft)) {
      MouseRotate(delta);
    } else if (Input::IsMouseButtonPressed(Mouse::ButtonRight)) {
      MouseZoom(delta.y);
    }
  }

  UpdateView();
}

void EditorCamera::OnEvent(Event& e) {
  EventDispatcher dispatcher(e);
  dispatcher.DispatchEvent<MouseScrollEvent>(CK_BIND_EVENT(EditorCamera::OnMouseScroll));
}

glm::vec3 EditorCamera::GetUpDirection() const {
  return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));  // y axis
}

glm::vec3 EditorCamera::GetRightDirection() const {
  return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));  // x axis
}

glm::vec3 EditorCamera::GetForwardDirection() const {
  return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
}

const glm::quat EditorCamera::GetOrientation() const {
  return glm::quat(glm::vec3(-pitch_, -yaw_, 0.0f));
}

// ----------------------------------------------------------------------------: private

void EditorCamera::UpdateProjection() {
  aspect_ratio_ = viewport_width_ / viewport_height_;
  projection_ = glm::perspective(glm::radians(fov_), aspect_ratio_, near_clip_, far_clip_);
}

void EditorCamera::UpdateView() {
  // yaw_ = pitch_ = 0.0f; // Lock the camera's rotation
  position_ = CalculatePosition();
  glm::quat orientation = GetOrientation();
  view_matrix_ = glm::translate(glm::mat4(1.0f), position_) * glm::toMat4(orientation);
  view_matrix_ = glm::inverse(view_matrix_);
}

bool EditorCamera::OnMouseScroll(MouseScrollEvent& e) {
  float delta = e.GetMouseYOffset() * 0.1f;
  MouseZoom(delta);
  UpdateView();
  return false;
}

void EditorCamera::MousePan(const glm::vec2& delta) {
  auto [x_speed, y_speed] = GetPanSpeed();
  focal_point_ += -GetRightDirection() * delta.x * x_speed * distance_;
  focal_point_ += GetUpDirection() * delta.y * y_speed * distance_;
}

void EditorCamera::MouseRotate(const glm::vec2& delta) {
  float yaw_sign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
  yaw_ += yaw_sign * delta.x * GetRotationSpeed();
  pitch_ += delta.y * GetRotationSpeed();
}

void EditorCamera::MouseZoom(float delta) {
  distance_ -= delta * GetZoomSpeed();
  if (distance_ < 1.0f) {
    focal_point_ += GetForwardDirection();
    distance_ = 1.0f;
  }
}

glm::vec3 EditorCamera::CalculatePosition() const {
  return focal_point_ - GetForwardDirection() * distance_;
}

std::pair<float, float> EditorCamera::GetPanSpeed() const {
  // Yes this is full of magic
  float x = std::min(viewport_width_ / 1000.0f, 2.4f);  // max = 2.4f;
  float x_factor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

  float y = std::min(viewport_height_ / 1000.0f, 2.4f);
  float y_factor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

  return {x_factor, y_factor};
}

float EditorCamera::GetRotationSpeed() const {
  return 0.8f;
}

float EditorCamera::GetZoomSpeed() const {
  float distance = std::max(distance_ * 0.2f, 0.0f);
  float speed = std::min(distance * distance, 100.0f);
  return speed;
}

}  // namespace ck
