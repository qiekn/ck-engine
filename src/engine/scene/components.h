#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "core/core.h"
#include "renderer/mesh.h"
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
//
// texture_path is the persistent identifier for serialization (6.B.5);
// texture is the runtime bindless slot, repopulated via Renderer2D::
// LoadTexture(texture_path, filter) on deserialize. Empty path = white
// fallback. filter chooses between Linear (default, smooth) and Nearest
// (crisp, for pixel-art / checkerboard textures).
struct SpriteRendererComponent {
  Renderer2D::TextureHandle texture = Renderer2D::kWhiteTexture;
  std::string texture_path;
  Renderer2D::Filter filter = Renderer2D::Filter::Linear;
  glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};

  SpriteRendererComponent() = default;
  SpriteRendererComponent(Renderer2D::TextureHandle tex) : texture(tex) {}
  SpriteRendererComponent(const glm::vec4& c) : color(c) {}
};

// Renderer3D mesh draw. mesh_path is the persistent identifier
// (Mesh::Load tokens: "cube" / "" -> procedural cube, anything else ->
// FromFile). mesh is the runtime device-local Ref repopulated on
// deserialize / mesh_path edit.
struct MeshComponent {
  Ref<Mesh>   mesh;
  std::string mesh_path = "cube";
  glm::vec3   tint{1.0f, 1.0f, 1.0f};

  MeshComponent() = default;
  MeshComponent(const std::string& path) : mesh_path(path) {}
};

}  // namespace ck
