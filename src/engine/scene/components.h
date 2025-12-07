#pragma once

#include <string>
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_float.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "scene/scene_camera.h"
#include "scene/scriptable_entity.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace ck {
// ----------------------------------------------------------------------------: Transform
struct TransformComponent {
  glm::vec3 position = {0.0f, 0.0f, 0.0f};
  glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
  glm::vec3 scale = {1.0f, 1.0f, 1.0f};

  TransformComponent() = default;
  TransformComponent(const TransformComponent&) = default;

  explicit TransformComponent(const glm::vec3& _position) : position(_position) {}

  glm::mat4 GetTransform() const {
    glm::mat4 rotation_matrix = glm::toMat4(glm::quat(rotation));
    glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), scale);

    return translation_matrix * rotation_matrix * scale_matrix;
  }
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

// ----------------------------------------------------------------------------: Camera
struct CameraComponent {
  SceneCamera camera;
  bool is_primary = true;  // TODO(qiekn): think about moving to Scene

  bool is_fixed_aspect_ratio = false;
};

// ----------------------------------------------------------------------------: Native Script
struct NativeScriptComponent {
  ScriptableEntity* instance = nullptr;

  // Function pointers
  ScriptableEntity* (*InstantiateScript)();
  void (*DestoryScript)(NativeScriptComponent*);

  template <typename T>
  void Bind() {
    InstantiateScript = []() {
      return static_cast<ScriptableEntity*>(new T());
    };
    DestoryScript = [](NativeScriptComponent* nsc) {
      delete nsc->instance;
      nsc->instance = nullptr;
    };
  }
};
}  // namespace ck
