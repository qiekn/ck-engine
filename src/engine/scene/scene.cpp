#include "scene.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "renderer/renderer_2d.h"
#include "scene/components.h"

namespace ck {
// ----------------------------------------------------------------------------: Static Func

static void DoMath(const glm::mat4& transform) {}

static void OnTransformConstruct(entt::registry& registry, entt::entity entity) {}

// ----------------------------------------------------------------------------: Class Impl

Scene::Scene() {}

Scene::~Scene() {}

entt::entity Scene::CreateEntity() {
  return registry_.create();
}

void Scene::OnUpdate(DeltaTime dt) {
  auto group = registry_.group<TransformComponent>(entt::get<SpriteRendererComponent>);
  for (auto entity : group) {
    const auto& [transform, sprite] =
        group.get<TransformComponent, SpriteRendererComponent>(entity);
    Renderer2D::DrawQuad(transform.transform, sprite.color);
  }
}

}  // namespace ck
