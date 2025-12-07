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

void Scene::DestroyEntity(const Entity& entity) {
  registry_.destroy(entity.GetID());
}

void Scene::OnUpdate(DeltaTime dt) {
  // Update Scripts
  {
    registry_.view<NativeScriptComponent>().each([=, this](auto entity, auto& nsc) {
      // TODO(qiekn): Move to Scene::OnScenePlay
      if (!nsc.instance) {
        nsc.instance = nsc.InstantiateScript();
        nsc.instance->entity_ = Entity{entity, this};
        nsc.instance->OnCreate();
      }

      nsc.instance->OnUpdate(dt);
    });
  }

  // Render 2D
  Camera* main_camera = nullptr;
  glm::mat4 camera_transform;
  {
    auto view = registry_.view<TransformComponent, CameraComponent>();
    for (auto entity : view) {
      const auto& [transform_comp, camera_comp] =
          view.get<TransformComponent, CameraComponent>(entity);

      if (camera_comp.is_primary) {
        main_camera = &camera_comp.camera;
        camera_transform = transform_comp.GetTransform();
        break;
      }
    }
  }

  if (main_camera) {
    Renderer2D::BeginScene(*main_camera, camera_transform);

    auto group = registry_.group<TransformComponent>(entt::get<SpriteRendererComponent>);
    for (auto entity : group) {
      const auto& [Transform, Sprite] =
          group.get<TransformComponent, SpriteRendererComponent>(entity);

      Renderer2D::DrawQuad(Transform.GetTransform(), Sprite.color);
    }

    Renderer2D::EndScene();
  }
}

void Scene::OnViewportResize(uint32_t width, uint32_t height) {
  viewport_width = width;
  viewport_height = height;

  // Resize our non-FixedAspectRatio Cameras
  auto view = registry_.view<CameraComponent>();
  for (auto entity : view) {
    auto& camera_comp = view.get<CameraComponent>(entity);
    if (!camera_comp.is_fixed_aspect_ratio) {
      camera_comp.camera.SetViewportSize(width, height);
    }
  }
}

Entity Scene::GetPrimaryCameraEntity() {
  auto view = registry_.view<CameraComponent>();
  for (auto entity : view) {
    const auto& camera = view.get<CameraComponent>(entity);
    if (camera.is_primary) {
      return Entity{entity, this};
    }
  }
  return {};
}

template <typename T>
void Scene::OnComponentAdded(const Entity& entity, T& component) {
  static_assert(false);
}

template <>
void Scene::OnComponentAdded<TransformComponent>(const Entity& entity,
                                                 TransformComponent& component) {}

template <>
void Scene::OnComponentAdded<CameraComponent>(const Entity& entity, CameraComponent& component) {
  component.camera.SetViewportSize(viewport_width, viewport_height);
}

template <>
void Scene::OnComponentAdded<SpriteRendererComponent>(const Entity& entity,
                                                      SpriteRendererComponent& component) {}

template <>
void Scene::OnComponentAdded<TagComponent>(const Entity& entity, TagComponent& component) {}

template <>
void Scene::OnComponentAdded<NativeScriptComponent>(const Entity& entity,
                                                    NativeScriptComponent& component) {}

}  // namespace ck
