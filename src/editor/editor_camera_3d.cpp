#include "editor_camera_3d.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace ck_editor {

namespace {
constexpr float kOrbitSpeed = 0.005f;       // rad per pixel
constexpr float kPanSpeed   = 0.0025f;      // world units per pixel at distance=1
constexpr float kZoomStep   = 0.1f;
constexpr float kDistanceMin = 0.1f;
constexpr float kDistanceMax = 1000.0f;
constexpr float kPitchClamp =
    std::numbers::pi_v<float> / 2.0f - 0.01f;  // avoid gimbal flip at poles
}  // namespace

EditorCamera3D::EditorCamera3D() {
  camera_.SetPerspective(std::numbers::pi_v<float> / 3.0f,  // 60deg
                         0.1f, 1000.0f);
  Recompute();
}

void EditorCamera3D::SetViewport(unsigned int width, unsigned int height) {
  camera_.SetViewport(width, height);
}

void EditorCamera3D::OnUpdate(ck::DeltaTime /*ts*/, bool viewport_hovered) {
  glm::vec2 mouse  = ck::Input::GetMousePos();
  bool      middle = ck::Input::IsMouseButtonPressed(ck::Mouse::ButtonMiddle);
  // Shift toggles between orbit (default) and pan.
  bool shift = ck::Input::IsKeyPressed(ck::KeyCode::LeftShift) ||
               ck::Input::IsKeyPressed(ck::KeyCode::RightShift);

  if (middle) {
    if (dragging_) {
      glm::vec2 d = mouse - last_mouse_;
      if (shift) {
        // Pan in camera-aligned axes. Compose camera basis from spherical
        // angles; scale by distance so pan feels consistent at any zoom.
        float cy = std::cos(yaw_), sy = std::sin(yaw_);
        float cp = std::cos(pitch_), sp = std::sin(pitch_);
        glm::vec3 forward(-cp * sy, -sp, -cp * cy);
        glm::vec3 right  ( cy, 0.0f, -sy);
        glm::vec3 up = glm::cross(right, forward);
        focus_ -= right * (d.x * kPanSpeed * distance_);
        focus_ += up    * (d.y * kPanSpeed * distance_);
      } else {
        yaw_   -= d.x * kOrbitSpeed;
        pitch_ -= d.y * kOrbitSpeed;
        pitch_ = std::clamp(pitch_, -kPitchClamp, kPitchClamp);
      }
      Recompute();
    } else if (viewport_hovered) {
      dragging_ = true;
    }
    last_mouse_ = mouse;
  } else {
    dragging_ = false;
    last_mouse_ = mouse;
  }
}

void EditorCamera3D::OnEvent(ck::Event& e, bool viewport_hovered) {
  if (!viewport_hovered) return;
  ck::EventDispatcher dispatcher(e);
  dispatcher.DispatchEvent<ck::MouseScrollEvent>(
      [this](ck::MouseScrollEvent& evt) { return OnMouseScroll(evt); });
}

bool EditorCamera3D::OnMouseScroll(ck::MouseScrollEvent& e) {
  // Multiplicative on distance so zoom feels constant in log space.
  distance_ -= e.GetMouseYOffset() * kZoomStep * distance_;
  distance_ = std::clamp(distance_, kDistanceMin, kDistanceMax);
  Recompute();
  return false;
}

void EditorCamera3D::Recompute() {
  // position = focus + distance * (sin(yaw)*cos(pitch),
  //                                sin(pitch),
  //                                cos(yaw)*cos(pitch))
  // With yaw=0, pitch=0 the camera sits at focus + (0, 0, distance) and
  // looks toward -Z (matching the convention of editor 2D scenes drawn
  // on the z=0 plane).
  float cp = std::cos(pitch_);
  glm::vec3 offset(std::sin(yaw_) * cp, std::sin(pitch_), std::cos(yaw_) * cp);
  glm::vec3 position = focus_ + distance_ * offset;
  camera_.SetView(position, focus_);
}

}  // namespace ck_editor
