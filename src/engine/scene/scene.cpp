#include "scene/scene.h"

#include "renderer/renderer_2d.h"
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

void Scene::OnUpdate(DeltaTime /*ts*/) {
  auto view = registry_.view<TransformComponent, SpriteRendererComponent>();
  for (auto e : view) {
    auto [t, s] = view.get<TransformComponent, SpriteRendererComponent>(e);
    Renderer2D::DrawQuad(t.transform, s.texture, s.color);
  }
}

}  // namespace ck
