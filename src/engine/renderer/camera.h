#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace ck {

// Orthographic camera, centered around |position|. NDC is
// [-aspect*zoom, aspect*zoom] horizontally and [-zoom, zoom] vertically;
// aspect is derived from the viewport extent. Vulkan Y-down NDC is
// handled internally so callers always work in "+Y up" world space.
//
// Default value is identity (1x1 viewport at origin, zoom=1) — call
// SetViewport at least once before reading view_projection().
class Camera {
public:
  Camera() = default;

  void SetViewport(uint32_t width, uint32_t height);
  void SetPosition(const glm::vec3& position);
  // zoom = world half-height in units. >1 zooms out, <1 zooms in. Clamped
  // to a sane positive range to avoid degenerate projections.
  void SetZoom(float zoom);

  const glm::mat4& view_projection() const { return view_projection_; }
  const glm::vec3& position()        const { return position_; }
  float            zoom()            const { return zoom_; }
  uint32_t         width()           const { return width_; }
  uint32_t         height()          const { return height_; }

private:
  void Recompute();

  glm::vec3 position_{0.0f};
  float     zoom_   = 1.0f;
  uint32_t  width_  = 1;
  uint32_t  height_ = 1;
  glm::mat4 view_projection_{1.0f};
};

}  // namespace ck
