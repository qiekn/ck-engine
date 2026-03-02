#include "scene.h"
#include <algorithm>
#include <vector>
#include "box2d/box2d.h"
#include "box2d/math_functions.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "renderer/camera.h"
#include "renderer/renderer_2d.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "scene/scriptable_entity.h"

namespace ck {
// ----------------------------------------------------------------------------: Static Func

static b2BodyType Rigidbody2DTypeToBox2DType(Rigidbody2DComponent::BodyType body_type) {
  switch (body_type) {
    case Rigidbody2DComponent::BodyType::Static:
      return b2_staticBody;
    case Rigidbody2DComponent::BodyType::Dynamic:
      return b2_dynamicBody;
    case Rigidbody2DComponent::BodyType::Kinematic:
      return b2_kinematicBody;
  }

  CK_ENGINE_ASSERT(false, "Unknown body type");
  return b2_staticBody;
}

// ----------------------------------------------------------------------------: Class Impl

Scene::Scene() {}

Scene::~Scene() {
  if (b2World_IsValid(physics_world_))
    b2DestroyWorld(physics_world_);
}

template <typename... Component>
static void CopyComponent(entt::registry& dst, entt::registry& src,
                           const std::unordered_map<UUID, entt::entity>& entt_map) {
  ([&]() {
    auto view = src.view<Component>();
    for (auto src_entity : view) {
      entt::entity dst_entity = entt_map.at(src.get<IDComponent>(src_entity).id);

      auto& src_component = src.get<Component>(src_entity);
      dst.emplace_or_replace<Component>(dst_entity, src_component);
    }
  }(),
   ...);
}

template <typename... Component>
static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src,
                           const std::unordered_map<UUID, entt::entity>& entt_map) {
  CopyComponent<Component...>(dst, src, entt_map);
}

template <typename... Component>
static void CopyComponentIfExists(Entity dst, Entity src) {
  ([&]() {
    if (src.HasComponent<Component>())
      dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
  }(),
   ...);
}

template <typename... Component>
static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src) {
  CopyComponentIfExists<Component...>(dst, src);
}

Ref<Scene> Scene::Copy(Ref<Scene> other) {
  Ref<Scene> new_scene = CreateRef<Scene>();

  new_scene->viewport_width = other->viewport_width;
  new_scene->viewport_height = other->viewport_height;

  auto& src_registry = other->registry_;
  auto& dst_registry = new_scene->registry_;
  std::unordered_map<UUID, entt::entity> entt_map;

  // Create entities in new scene (reverse to preserve original order)
  auto id_view = src_registry.view<IDComponent>();
  std::vector<entt::entity> entities(id_view.begin(), id_view.end());
  std::reverse(entities.begin(), entities.end());
  for (auto e : entities) {
    UUID uuid = src_registry.get<IDComponent>(e).id;
    const auto& name = src_registry.get<TagComponent>(e).name;
    Entity new_entity = new_scene->CreateEntityWithUUID(uuid, name);
    entt_map[uuid] = (entt::entity)new_entity;
  }

  // Copy components (except IDComponent and TagComponent)
  CopyComponent(AllComponents{}, dst_registry, src_registry, entt_map);

  return new_scene;
}

Entity Scene::CreateEntity(const std::string& name) {
  return CreateEntityWithUUID(UUID(), name);
}

Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name) {
  Entity entity = {registry_.create(), this};
  entity.AddComponent<IDComponent>(uuid);
  entity.AddComponent<TransformComponent>();
  auto& tag = entity.AddComponent<TagComponent>();
  tag.name = name.empty() ? "Entity" : name;
  return entity;
}

void Scene::DestroyEntity(const Entity& entity) {
  registry_.destroy(entity.GetID());
}

void Scene::OnRuntimeStart() {
  OnPhysics2DStart();
}

void Scene::OnRuntimeStop() {
  OnPhysics2DStop();
}

void Scene::OnSimulationStart() {
  OnPhysics2DStart();
}

void Scene::OnSimulationStop() {
  OnPhysics2DStop();
}

void Scene::OnUpdateRuntime(DeltaTime dt) {
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

  // Physics
  {
    const int kSubStepCount = 4;
    b2World_Step(physics_world_, dt, kSubStepCount);

    auto view = registry_.view<Rigidbody2DComponent>();
    for (auto e : view) {
      Entity entity = {e, this};
      auto& transform = entity.GetComponent<TransformComponent>();
      auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

      b2BodyId body_id = b2LoadBodyId(rb2d.runtime_body_id);
      b2Vec2 position = b2Body_GetPosition(body_id);
      transform.position.x = position.x;
      transform.position.y = position.y;
      transform.rotation.z = b2Rot_GetAngle(b2Body_GetRotation(body_id));
    }
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

    // Draw sprites
    {
      auto group = registry_.group<TransformComponent>(entt::get<SpriteRendererComponent>);
      for (auto entity : group) {
        const auto& [Transform, Sprite] =
            group.get<TransformComponent, SpriteRendererComponent>(entity);

        Renderer2D::DrawSprite(Transform.GetTransform(), Sprite, (int)entity);
      }
    }

    // Draw circles
    {
      auto view = registry_.view<TransformComponent, CircleRendererComponent>();
      for (auto entity : view) {
        auto [transform, circle] =
            view.get<TransformComponent, CircleRendererComponent>(entity);

        Renderer2D::DrawCircle(transform.GetTransform(), circle.color, circle.thickness,
                               circle.fade, (int)entity);
      }
    }

    Renderer2D::EndScene();
  }
}

void Scene::OnUpdateSimulation(DeltaTime dt, EditorCamera& camera) {
  // Physics
  {
    const int kSubStepCount = 4;
    b2World_Step(physics_world_, dt, kSubStepCount);

    auto view = registry_.view<Rigidbody2DComponent>();
    for (auto e : view) {
      Entity entity = {e, this};
      auto& transform = entity.GetComponent<TransformComponent>();
      auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

      b2BodyId body_id = b2LoadBodyId(rb2d.runtime_body_id);
      b2Vec2 position = b2Body_GetPosition(body_id);
      transform.position.x = position.x;
      transform.position.y = position.y;
      transform.rotation.z = b2Rot_GetAngle(b2Body_GetRotation(body_id));
    }
  }

  // Render
  RenderScene(camera);
}

void Scene::OnUpdateEditor(DeltaTime dt, EditorCamera& camera) {
  RenderScene(camera);
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

void Scene::DuplicateEntity(Entity entity) {
  Entity new_entity = CreateEntity(entity.GetName());
  CopyComponentIfExists(AllComponents{}, new_entity, entity);
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

void Scene::OnPhysics2DStart() {
  b2WorldDef world_def = b2DefaultWorldDef();
  world_def.gravity = {0.0f, -9.8f};
  physics_world_ = b2CreateWorld(&world_def);

  auto view = registry_.view<Rigidbody2DComponent>();
  for (auto e : view) {
    Entity entity = {e, this};
    auto& transform = entity.GetComponent<TransformComponent>();
    auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

    b2BodyDef body_def = b2DefaultBodyDef();
    body_def.type = Rigidbody2DTypeToBox2DType(rb2d.body_type);
    body_def.position = {transform.position.x, transform.position.y};
    body_def.rotation = b2MakeRot(transform.rotation.z);
    body_def.motionLocks.angularZ = rb2d.fixed_rotation;

    b2BodyId body_id = b2CreateBody(physics_world_, &body_def);

    if (entity.HasComponent<BoxCollider2DComponent>()) {
      auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

      b2Polygon box = b2MakeOffsetBox(bc2d.size.x * transform.scale.x,
                                      bc2d.size.y * transform.scale.y,
                                      {bc2d.offset.x, bc2d.offset.y},
                                      b2MakeRot(0.0f));

      b2ShapeDef shape_def = b2DefaultShapeDef();
      shape_def.density = bc2d.density;
      shape_def.material.friction = bc2d.friction;
      shape_def.material.restitution = bc2d.restitution;
      b2CreatePolygonShape(body_id, &shape_def, &box);
    }

    if (entity.HasComponent<CircleCollider2DComponent>()) {
      auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();

      b2Circle circle;
      circle.center = {cc2d.offset.x, cc2d.offset.y};
      circle.radius = cc2d.radius * transform.scale.x;

      b2ShapeDef shape_def = b2DefaultShapeDef();
      shape_def.density = cc2d.density;
      shape_def.material.friction = cc2d.friction;
      shape_def.material.restitution = cc2d.restitution;
      b2CreateCircleShape(body_id, &shape_def, &circle);
    }

    rb2d.runtime_body_id = b2StoreBodyId(body_id);
  }
}

void Scene::OnPhysics2DStop() {
  b2DestroyWorld(physics_world_);
  physics_world_ = {};
}

void Scene::RenderScene(EditorCamera& camera) {
  Renderer2D::BeginScene(camera);

  // Draw sprites
  {
    auto group = registry_.group<TransformComponent>(entt::get<SpriteRendererComponent>);
    for (auto entity : group) {
      auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
      Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
    }
  }

  // Draw circles
  {
    auto view = registry_.view<TransformComponent, CircleRendererComponent>();
    for (auto entity : view) {
      auto [transform, circle] =
          view.get<TransformComponent, CircleRendererComponent>(entity);

      Renderer2D::DrawCircle(transform.GetTransform(), circle.color, circle.thickness,
                             circle.fade, (int)entity);
    }
  }

  Renderer2D::EndScene();
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
  if (viewport_width > 0 && viewport_height > 0)
    component.camera.SetViewportSize(viewport_width, viewport_height);
}

template <>
void Scene::OnComponentAdded<SpriteRendererComponent>(const Entity& entity,
                                                      SpriteRendererComponent& component) {}

template <>
void Scene::OnComponentAdded<CircleRendererComponent>(const Entity& entity,
                                                      CircleRendererComponent& component) {}

template <>
void Scene::OnComponentAdded<IDComponent>(const Entity& entity, IDComponent& component) {}

template <>
void Scene::OnComponentAdded<TagComponent>(const Entity& entity, TagComponent& component) {}

template <>
void Scene::OnComponentAdded<NativeScriptComponent>(const Entity& entity,
                                                    NativeScriptComponent& component) {}

template <>
void Scene::OnComponentAdded<Rigidbody2DComponent>(const Entity& entity,
                                                   Rigidbody2DComponent& component) {}

template <>
void Scene::OnComponentAdded<BoxCollider2DComponent>(const Entity& entity,
                                                     BoxCollider2DComponent& component) {}

template <>
void Scene::OnComponentAdded<CircleCollider2DComponent>(const Entity& entity,
                                                        CircleCollider2DComponent& component) {}

}  // namespace ck
