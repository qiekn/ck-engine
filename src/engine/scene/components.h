#pragma once

#include <string>
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float4.hpp"
#include "scene/scene_camera.h"
#include "scene/scriptable_entity.h"

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
