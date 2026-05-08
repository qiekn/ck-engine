#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace ck {

// Perspective look-at camera. The arcball editor camera (and any in-game
// 3D cameras) build position/target each frame and push them in via
// SetView; this class composes the view + projection matrix and exposes
// the result to whatever pipeline is rendering. Vulkan Y-down NDC is
// handled internally so callers think in OpenGL-style "+Y up" world space.
class PerspectiveCamera {
public:
  PerspectiveCamera() { Recompute(); }

  // Aspect ratio is recomputed from |width|/|height|. Renderer drives this
  // each frame from the offscreen color target's extent.
  void SetViewport(uint32_t width, uint32_t height);
  // |fov| in radians. Defaults: 60deg fov, near 0.1, far 1000.
  void SetPerspective(float fov, float near_plane, float far_plane);
  void SetView(const glm::vec3& position, const glm::vec3& target,
               const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));

  const glm::mat4& view_projection() const { return view_projection_; }
  const glm::vec3& position()        const { return position_; }
  const glm::vec3& target()          const { return target_; }

  uint32_t width()  const { return width_; }
  uint32_t height() const { return height_; }

private:
  void Recompute();

  float    fov_  = 1.0471975511965976f;  // 60deg
  float    near_ = 0.1f;
  float    far_  = 1000.0f;
  uint32_t width_  = 1;
  uint32_t height_ = 1;

  glm::vec3 position_{0.0f, 0.0f, 5.0f};
  glm::vec3 target_  {0.0f, 0.0f, 0.0f};
  glm::vec3 up_      {0.0f, 1.0f, 0.0f};
  glm::mat4 view_projection_{1.0f};
};

}  // namespace ck
