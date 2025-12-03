#include "scene.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "renderer/camera.h"
#include "renderer/renderer_2d.h"
#include "scene/components.h"
#include "scene/entity.h"

namespace ck {
// ----------------------------------------------------------------------------: Static Func

static void DoMath(const glm::mat4& transform) {}

static void OnTransformConstruct(entt::registry& registry, entt::entity entity) {}

// ----------------------------------------------------------------------------: Class Impl

Scene::Scene() {}

Scene::~Scene() {}

Entity Scene::CreateEntity(const std::string& name) {
  Entity entity = {registry_.create(), this};
  entity.AddComponent<TransformComponent>();
  auto& tag = entity.AddComponent<TagComponent>();
  tag.name = name.empty() ? "Entity" : name;
  return entity;
}

void Scene::OnUpdate(DeltaTime dt) {
  // Render 2D
  Camera* main_camera = nullptr;
  glm::mat4* camera_transform = nullptr;
  {
    auto group = registry_.view<TransformComponent, CameraComponent>();
    for (auto entity : group) {
      const auto& [transform_comp, camera_comp] =
          group.get<TransformComponent, CameraComponent>(entity);

      if (camera_comp.is_primary) {
        main_camera = &camera_comp.camera;
        camera_transform = &transform_comp.transform;
        break;
      }
    }
  }

  if (main_camera) {
    Renderer2D::BeginScene(*main_camera, *camera_transform);

    auto group = registry_.group<TransformComponent>(entt::get<SpriteRendererComponent>);
    for (auto entity : group) {
      const auto& [Transform, Sprite] =
          group.get<TransformComponent, SpriteRendererComponent>(entity);

      Renderer2D::DrawQuad(Transform.transform, Sprite.color);
    }

    Renderer2D::EndScene();
  }
}

}  // namespace ck
