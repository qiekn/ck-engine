#pragma once

#include <string>
#include <glm/glm.hpp>

#include "renderer/renderer_2d.h"

namespace ck {

// Display name for the SceneHierarchyPanel (and serialization key in 6.B.5).
struct TagComponent {
  std::string tag;

  TagComponent() = default;
  TagComponent(const std::string& t) : tag(t) {}
};

// World transform. Single mat4 for now — split into translation/rotation/
// scale when 6.B.3's PropertiesPanel needs to inspect them individually.
struct TransformComponent {
  glm::mat4 transform{1.0f};

  TransformComponent() = default;
  TransformComponent(const glm::mat4& m) : transform(m) {}

  operator glm::mat4&()             { return transform; }
  operator const glm::mat4&() const { return transform; }
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
