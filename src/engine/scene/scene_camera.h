#pragma once

#include "renderer/camera.h"

namespace ck {

class SceneCamera : public Camera {
public:
  SceneCamera();
  virtual ~SceneCamera();

  void SetOrthographic(float size, float near_clip, float far_clip);

  void SetViewportSize(uint32_t width, uint32_t height);

  void SetOrthographicSize(float size) {
    orthographic_size_ = size;
    RecalculateProjection();
  }

  float GetOrthographicSize() const { return orthographic_size_; }

private:
  void RecalculateProjection();

private:
  float orthographic_size_ = 10.0f;
  float orthographic_near_ = -1.0f;
  float orthographic_far_ = 1.0f;

  float aspect_ratio_ = 0.0f;
};
}  // namespace ck
