#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "renderer/renderer_2d.h"

namespace ck {

// Display name for the SceneHierarchyPanel (and serialization key in 6.B.5).
struct TagComponent {
  std::string tag;

  TagComponent() = default;
  TagComponent(const std::string& t) : tag(t) {}
};

// World transform stored as T/R/S so PropertiesPanel can edit each axis
// individually. GetTransform composes them on demand for Renderer2D.
// rotation is XYZ-euler in radians (Hazel convention via glm::quat ctor).
struct TransformComponent {
  glm::vec3 translation{0.0f, 0.0f, 0.0f};
  glm::vec3 rotation   {0.0f, 0.0f, 0.0f};
  glm::vec3 scale      {1.0f, 1.0f, 1.0f};

  TransformComponent() = default;
  TransformComponent(const glm::vec3& t) : translation(t) {}

  glm::mat4 GetTransform() const {
    glm::mat4 r = glm::mat4_cast(glm::quat(rotation));
    return glm::translate(glm::mat4(1.0f), translation) * r *
           glm::scale(glm::mat4(1.0f), scale);
  }
};

// Renderer2D quad. texture=kWhiteTexture + color = solid color.
// Real texture + color = texture tinted by color.
struct SpriteRendererComponent {
  Renderer2D::TextureHandle texture = Renderer2D::kWhiteTexture;
  glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};

  SpriteRendererComponent() = default;
  SpriteRendererComponent(Renderer2D::TextureHandle tex) : texture(tex) {}
  SpriteRendererComponent(const glm::vec4& c) : color(c) {}
};

}  // namespace ck
