#pragma once

#include <string>
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float4.hpp"
#include "renderer/camera.h"

namespace ck {
// ----------------------------------------------------------------------------: Transform
struct TransformComponent {
  glm::mat4 transform{1.0f};

  TransformComponent() = default;
  TransformComponent(const TransformComponent&) = default;

  explicit TransformComponent(const glm::mat4& _transform) : transform(_transform) {}

  explicit operator glm::mat4&() { return transform; }

  explicit operator const glm::mat4&() const { return transform; }
};

// ----------------------------------------------------------------------------: SpriteRenderer
struct SpriteRendererComponent {
  glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};

  SpriteRendererComponent() = default;
  SpriteRendererComponent(const SpriteRendererComponent&) = default;

  explicit SpriteRendererComponent(const glm::vec4& _color) : color(_color) {}
};

// ----------------------------------------------------------------------------: Tag
struct TagComponent {
  std::string name;

  TagComponent() = default;
  TagComponent(const TagComponent&) = default;

  explicit TagComponent(const std::string& _name) : name(_name) {}
};
}  // namespace ck

// ----------------------------------------------------------------------------: Camera
struct CameraComponent {
  ck::Camera camera;
  bool is_primary = true;  // TODO(qiekn): think about moving to Scene

  explicit CameraComponent(const glm::mat4& projection) : camera(projection) {}
};
