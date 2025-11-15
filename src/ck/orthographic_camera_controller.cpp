#include "orthographic_camera_controller.h"

#include "events/application_event.h"
#include "events/event.h"
#include "events/mouse_event.h"
#include "input.h"
#include "key_code.h"
#include "log.h"
#include "renderer/orthographic_camera.h"

namespace ck {

OrthographicCameraController::OrthographicCameraController(float aspect_ratio, bool rotation)
    : aspect_ratio_(aspect_ratio),
      camera_(-aspect_ratio_ * zoom_level_, aspect_ratio_ * zoom_level_, -zoom_level_, zoom_level_),
      rotation_(rotation) {}

void OrthographicCameraController::OnUpdate(DeltaTime dt) {
  if (Input::IsKeyPressed(CK_KEY_A)) {
    camera_position_.x -= camera_translation_speed_ * dt;
  } else if (Input::IsKeyPressed(CK_KEY_D)) {
    camera_position_.x += camera_translation_speed_ * dt;
  }

  if (Input::IsKeyPressed(CK_KEY_W)) {
    camera_position_.y += camera_translation_speed_ * dt;
  } else if (Input::IsKeyPressed(CK_KEY_S)) {
    camera_position_.y -= camera_translation_speed_ * dt;
  }

  if (Input::IsKeyPressed(CK_KEY_Q)) {
    camera_rotation_ += camera_rotation_speed_ * dt;
  } else if (Input::IsKeyPressed(CK_KEY_E)) {
    camera_rotation_ -= camera_rotation_speed_ * dt;
  }

  camera_.SetPosition(camera_position_);
  camera_.SetRotation(camera_rotation_);

  camera_translation_speed_ = zoom_level_;
}

void OrthographicCameraController::OnEvent(Event& e) {
  auto dispatcher = EventDispatcher(e);
  dispatcher.DispatchEvent<MouseScrollEvent>(
      CK_BIND_EVENT(OrthographicCameraController::OnMouseScrolled));
  dispatcher.DispatchEvent<WindowResizeEvent>(
      CK_BIND_EVENT(OrthographicCameraController::OnWindowResized));
}

bool OrthographicCameraController::OnMouseScrolled(MouseScrollEvent& e) {
  zoom_level_ -= e.GetMouseYOffset() * 0.25f;
  zoom_level_ = std::max(zoom_level_, 0.25f);
  camera_.SetProjection(-aspect_ratio_ * zoom_level_, aspect_ratio_ * zoom_level_, -zoom_level_,
                        zoom_level_);
  return false;
}

bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e) {
  aspect_ratio_ = float(e.GetWindowWidth() / e.GetWindowHeight());
  camera_.SetProjection(-aspect_ratio_ * zoom_level_, aspect_ratio_ * zoom_level_, -zoom_level_,
                        zoom_level_);
  return false;
}

}  // namespace ck
