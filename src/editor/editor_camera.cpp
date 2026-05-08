#include "editor_camera.h"

#include <algorithm>

namespace ck_editor {

namespace {
// Pan factor at zoom=1: 1px of mouse delta -> kPanScale world units.
// Multiplied by zoom_ at the call site so panning feels constant in
// screen space across zoom levels.
constexpr float kPanScale = 0.005f;
constexpr float kZoomMin  = 0.05f;
constexpr float kZoomMax  = 50.0f;
constexpr float kScrollStep = 0.1f;
}  // namespace

void EditorCamera::OnUpdate(ck::DeltaTime /*ts*/, bool viewport_hovered) {
  glm::vec2 mouse  = ck::Input::GetMousePos();
  bool      middle = ck::Input::IsMouseButtonPressed(ck::Mouse::ButtonMiddle);

  if (middle) {
    if (panning_) {
      glm::vec2 d = mouse - last_mouse_;
      // World axes: +X right, +Y up. Mouse Y grows down -> invert.
      position_.x -= d.x * kPanScale * zoom_;
      position_.y += d.y * kPanScale * zoom_;
    } else if (viewport_hovered) {
      // Latch on first frame the button is held over the viewport;
      // continues even if the cursor strays outside until release.
      panning_ = true;
    }
    last_mouse_ = mouse;
  } else {
    panning_ = false;
    last_mouse_ = mouse;
  }
}

void EditorCamera::OnEvent(ck::Event& e, bool viewport_hovered) {
  if (!viewport_hovered) return;
  ck::EventDispatcher dispatcher(e);
  dispatcher.DispatchEvent<ck::MouseScrollEvent>(
      [this](ck::MouseScrollEvent& evt) { return OnMouseScroll(evt); });
}

bool EditorCamera::OnMouseScroll(ck::MouseScrollEvent& e) {
  // Multiplicative scroll: each tick scales zoom by ~10%, clamped to a
  // sane range. e.GetMouseYOffset() is +1 per wheel-up, -1 per wheel-down.
  zoom_ -= e.GetMouseYOffset() * kScrollStep * zoom_;
  zoom_ = std::clamp(zoom_, kZoomMin, kZoomMax);
  return false;  // don't mark handled — let other layers see it too
}

void EditorCamera::PushTo(ck::Camera& camera) const {
  camera.SetPosition(position_);
  camera.SetZoom(zoom_);
}

}  // namespace ck_editor
