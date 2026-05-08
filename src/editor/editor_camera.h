#pragma once

// Same `import ck;`-in-header pattern used by the panels: needed so we can
// hold ck:: types by value in the public surface (Renderer/Camera handles).
import ck;

namespace ck_editor {

// 2D pannable + zoomable orthographic camera driver. Tracks position and
// zoom in editor-local state; PushTo(camera) writes them into the engine
// camera each frame. Input is gated on viewport hover/focus by the caller
// so dock-area drags / scrolls outside the viewport don't move the world.
//
// Pan: middle-mouse drag while viewport hovered. Zoom: mouse wheel while
// viewport hovered (multiplicative — feels constant in log space).
class EditorCamera {
public:
  void OnUpdate(ck::DeltaTime ts, bool viewport_hovered);
  void OnEvent(ck::Event& e, bool viewport_hovered);

  // Push the editor camera's position + zoom into the engine camera.
  // Renderer keeps owning SetViewport so width/height stay tied to the
  // color target's extent.
  void PushTo(ck::Camera& camera) const;

  const glm::vec3& position() const { return position_; }
  float            zoom()     const { return zoom_; }

private:
  bool OnMouseScroll(ck::MouseScrollEvent& e);

  glm::vec3 position_{0.0f, 0.0f, 0.0f};
  float     zoom_ = 1.0f;       // world half-height in units
  glm::vec2 last_mouse_{0.0f};  // for pan delta
  bool      panning_ = false;
};

}  // namespace ck_editor
