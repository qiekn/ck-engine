#pragma once

#include <string>
#include <vector>
#include <entt.hpp>

#include "core/deltatime.h"

namespace ck {

class Entity;

// Scene wraps entt::registry. Clients add entities via CreateEntity, attach
// components through Entity's templated AddComponent, and let OnUpdate iterate
// the (Transform, SpriteRenderer) view to drive Renderer2D.
//
// entt itself is intentionally NOT re-exported through `import ck;`. Clients
// that want raw registry access can include this header directly inside the
// engine sources, but the public API stays Entity-shaped.
class Scene {
public:
  Scene() = default;
  ~Scene() = default;

  Scene(const Scene&) = delete;
  Scene& operator=(const Scene&) = delete;

  Entity CreateEntity(const std::string& tag = std::string());
  void DestroyEntity(Entity entity);

  // Iterates (TransformComponent, SpriteRendererComponent) view and calls
  // Renderer2D::DrawQuad for each. Caller must have already opened a
  // Renderer2D::BeginScene (handled by Renderer::BeginFrame).
  void OnUpdate(DeltaTime ts);

  // Snapshot of every live entity wrapped in ck::Entity. Allocated per
  // call so the SceneHierarchyPanel never has to touch entt:: directly.
  std::vector<Entity> GetAllEntities();

private:
  friend class Entity;
  entt::registry registry_;
};

}  // namespace ck
