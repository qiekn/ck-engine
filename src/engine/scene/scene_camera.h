#pragma once

#include "glm/trigonometric.hpp"
#include "renderer/camera.h"

namespace ck {

class SceneCamera : public Camera {
public:
  enum class ProjectionType : std::uint8_t { kPerspective = 0, kOrthographic = 1 };

public:
  SceneCamera();
  virtual ~SceneCamera();

  void SetOrthographic(float size, float near_clip, float far_clip);

  void SetPerspective(float vertical_fov, float near_clip, float far_clip);

  void SetViewportSize(uint32_t width, uint32_t height);

  ProjectionType GetProjectionType() const { return projection_type_; }

  void SetProjectionType(ProjectionType type) { projection_type_ = type; }

  // ----------------------------------------------------------------------------: Perspective

  void SetPerspectiveVerticalFOV(float vertical_fov) { perspective_fov_ = vertical_fov; }

  void SetPerspectiveNearClip(float near_clip) {
    perspective_near_ = near_clip;
    RecalculateProjection();
  }

  void SetPerspectiveFarClip(float far_clip) {
    perspective_far_ = far_clip;
    RecalculateProjection();
  }

  float GetPerspectiveVerticalFOV() { return perspective_fov_; }

  float GetPerspectiveNearClip() { return perspective_near_; }

  float GetPerspectiveFarClip() { return perspective_far_; }

  // ----------------------------------------------------------------------------: Ortho

  void SetOrthographicSize(float size) {
    orthographic_size_ = size;
    RecalculateProjection();
  }

  void SetOrthographicNearClip(float nearClip) {
    orthographic_near_ = nearClip;
    RecalculateProjection();
  }

  void SetOrthographicFarClip(float farClip) {
    orthographic_far_ = farClip;
    RecalculateProjection();
  }

  float GetOrthographicSize() const { return orthographic_size_; }

  float GetOrthographicNearClip() const { return orthographic_near_; }

  float GetOrthographicFarClip() const { return orthographic_far_; }

private:
  void RecalculateProjection();

private:
  ProjectionType projection_type_ = ProjectionType::kOrthographic;

  float perspective_fov_ = glm::radians(45.0f);
  float perspective_near_ = 0.01f, perspective_far_ = 1000.0f;

  float orthographic_size_ = 10.0f;
  float orthographic_near_ = -1.0f, orthographic_far_ = 1.0f;

  float aspect_ratio_ = 0.0f;
};
}  // namespace ck
