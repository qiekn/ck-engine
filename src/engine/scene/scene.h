#pragma once

#include "core/deltatime.h"
#include "entt.hpp"
#include "renderer/editor_camera.h"

namespace ck {
class Entity;

class Scene {
public:
  Scene();
  virtual ~Scene();

  Entity CreateEntity(const std::string& name = std::string());
  void DestroyEntity(const Entity& entity);

  void OnUpdateRuntime(DeltaTime dt);
  void OnUpdateEditor(DeltaTime dt, EditorCamera& camera);
  void OnViewportResize(uint32_t width, uint32_t height);
  Entity GetPrimaryCameraEntity();

private:
  template <typename T>
  void OnComponentAdded(const Entity& entity, T& component);

private:
  entt::registry registry_;
  uint32_t viewport_width = 0, viewport_height = 0;

  friend class Entity;
  friend class SceneSerializer;
  friend class SceneHierarchyPanel;
};
}  // namespace ck
