#pragma once

#include "core/deltatime.h"
#include "events/application_event.h"
#include "events/event.h"
#include "events/mouse_event.h"
#include "glm/ext/vector_float3.hpp"
#include "renderer/orthographic_camera.h"

namespace ck {
class OrthographicCameraController {
public:
  explicit OrthographicCameraController(float aspect_ratio, bool rotation = false);

  void OnUpdate(DeltaTime);
  void OnEvent(Event&);

  void OnResize(float width, float height);

  const auto& Camera() const { return camera_; }

  auto& Camera() { return camera_; }

  float ZoomLevel() const { return zoom_level_; }

  void SetZoomLevel(float level) { zoom_level_ = level; }

private:
  bool OnMouseScrolled(MouseScrollEvent&);
  bool OnWindowResized(WindowResizeEvent&);

private:
  float aspect_ratio_;
  float zoom_level_{1};

  OrthographicCamera camera_;

  glm::vec3 camera_position_{0.0f};
  float camera_rotation_{0.0f};
  float camera_translation_speed_{5.0f};
  float camera_rotation_speed_{100.0f};

  bool rotation_;
};
}  // namespace ck
