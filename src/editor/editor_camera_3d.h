#pragma once

// Same `import ck;`-in-header pattern used by the panels: needed so we can
// hold ck:: types by value in the public surface.
import ck;

namespace ck_editor {

// 3D arcball editor camera. Holds spherical orbit state (focus point,
// yaw, pitch, distance) and rebuilds a PerspectiveCamera each frame.
// The composed view_projection is pushed to the engine via
// Renderer::SetActiveCamera by EditorLayer.
//
// Pan: middle-mouse drag while viewport hovered.
// Orbit: shift + middle-mouse drag.
// Zoom: mouse wheel (multiplicative on distance).
class EditorCamera3D {
public:
  EditorCamera3D();

  // Render-target extent for aspect; called per-frame from EditorLayer.
  void SetViewport(unsigned int width, unsigned int height);

  void OnUpdate(ck::DeltaTime ts, bool viewport_hovered);
  void OnEvent(ck::Event& e, bool viewport_hovered);

  const glm::mat4& view_projection() const { return camera_.view_projection(); }

private:
  bool OnMouseScroll(ck::MouseScrollEvent& e);
  void Recompute();

  ck::PerspectiveCamera camera_;

  glm::vec3 focus_{0.0f};
  float     distance_ = 5.0f;
  float     yaw_   = 0.0f;       // rotation around +Y, radians
  float     pitch_ = 0.3f;       // rotation around camera-right, radians

  glm::vec2 last_mouse_{0.0f};
  bool      dragging_ = false;
};

}  // namespace ck_editor
