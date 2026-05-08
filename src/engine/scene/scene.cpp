#include "scene/scene.h"

#include "renderer/renderer_2d.h"
#include "renderer/renderer_3d.h"
#include "scene/components.h"
#include "scene/entity.h"

namespace ck {

Entity Scene::CreateEntity(const std::string& tag) {
  Entity entity(registry_.create(), this);
  entity.AddComponent<TransformComponent>();
  auto& tagcomp = entity.AddComponent<TagComponent>();
  tagcomp.tag = tag.empty() ? "Entity" : tag;
  return entity;
}

void Scene::DestroyEntity(Entity entity) {
  registry_.destroy(static_cast<entt::entity>(entity));
}

void Scene::Clear() {
  registry_.clear();
}

void Scene::OnUpdate(DeltaTime /*ts*/) {
  // 2D sprites
  {
    auto view = registry_.view<TransformComponent, SpriteRendererComponent>();
    for (auto e : view) {
      auto [t, s] = view.get<TransformComponent, SpriteRendererComponent>(e);
      Renderer2D::DrawQuad(t.GetTransform(), s.texture, s.color);
    }
  }
  // 3D meshes
  {
    auto view = registry_.view<TransformComponent, MeshComponent>();
    for (auto e : view) {
      auto [t, m] = view.get<TransformComponent, MeshComponent>(e);
      if (m.mesh) Renderer3D::DrawMesh(m.mesh, t.GetTransform(), m.tint);
    }
  }
}

std::vector<Entity> Scene::GetAllEntities() {
  auto& storage = registry_.storage<entt::entity>();
  std::vector<Entity> out;
  out.reserve(storage.size());
  for (auto e : storage) {
    out.emplace_back(e, this);
  }
  return out;
}

}  // namespace ck
