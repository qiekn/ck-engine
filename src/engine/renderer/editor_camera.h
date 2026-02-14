#pragma once

#include <utility>
#include "core/deltatime.h"
#include "events/event.h"
#include "events/mouse_event.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/quaternion_float.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "renderer/camera.h"

namespace ck {
class EditorCamera : public Camera {
  // ----------------------------------------------------------------------------: methods
public:
  EditorCamera() = default;

  EditorCamera(float fov, float aspect_ratio, float near_clip, float far_clip);

  void OnUpdate(DeltaTime dt);

  void OnEvent(Event& e);

  inline void SetDistance(float distance) { distance_ = distance; }

  inline float GetDistance() const { return distance_; }

  inline void SetViewportSize(float w, float h) {
    viewport_width_ = w, viewport_height_ = h;
    UpdateProjection();
  }

  const glm::mat4& GetViewMatrx() const { return view_matrix_; }

  glm::mat4 GetViewProjection() const { return projection_ * view_matrix_; }

  glm::vec3 GetUpDirection() const;

  glm::vec3 GetRightDirection() const;

  glm::vec3 GetForwardDirection() const;

  const glm::quat GetOrientation() const;

  float GetPitch() const { return pitch_; }

  float GetYaw() const { return yaw_; }

private:
  void UpdateProjection();
  void UpdateView();
  bool OnMouseScroll(MouseScrollEvent& e);

  void MousePan(const glm::vec2& delta);
  void MouseRotate(const glm::vec2& delta);
  void MouseZoom(float delta);

  glm::vec3 CalculatePosition() const;

  std::pair<float, float> GetPanSpeed() const;
  float GetRotationSpeed() const;
  float GetZoomSpeed() const;

private:
  // ----------------------------------------------------------------------------: fields
  float fov_ = 45.0f, aspect_ratio_ = 1.778f, near_clip_ = 0.1f, far_clip_ = 1000.0f;

  glm::mat4 view_matrix_;
  glm::vec3 position_{0.0f, 0.0f, 0.0f};
  glm::vec3 focal_point_ = {0.0f, 0.0f, 0.0f};
  glm::vec2 initial_mouse_pos_{0.0f, 0.0f};

  // Core orbital camera  paramters
  float distance_{10.0f};  // distance from the camera's position to the focal point
  float pitch_{0.0f};     // controls the vertical angle (looking up/down)
  float yaw_{0.0f};       // controls the horizontal angle (looking left/right)

  float viewport_width_ = 1280, viewport_height_ = 720;
};
}  // namespace ck
