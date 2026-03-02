#pragma once

#include "box2d/id.h"
#include "core/deltatime.h"
#include "core/uuid.h"
#include "entt.hpp"
#include "renderer/editor_camera.h"

namespace ck {
class Entity;

class Scene {
public:
  Scene();
  virtual ~Scene();

  static Ref<Scene> Copy(Ref<Scene> other);

  Entity CreateEntity(const std::string& name = std::string());
  Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
  void DestroyEntity(const Entity& entity);

  void OnRuntimeStart();
  void OnRuntimeStop();

  void OnUpdateRuntime(DeltaTime dt);
  void OnUpdateEditor(DeltaTime dt, EditorCamera& camera);
  void OnViewportResize(uint32_t width, uint32_t height);

  void DuplicateEntity(Entity entity);

  Entity GetPrimaryCameraEntity();

  template <typename... Components>
  auto GetAllEntitiesWith() {
    return registry_.view<Components...>();
  }

private:
  template <typename T>
  void OnComponentAdded(const Entity& entity, T& component);

private:
  entt::registry registry_;
  uint32_t viewport_width = 0, viewport_height = 0;

  b2WorldId physics_world_ = {};

  friend class Entity;
  friend class SceneSerializer;
  friend class SceneHierarchyPanel;
};
}  // namespace ck
