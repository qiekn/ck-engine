#include "scene_serializer.h"
#include "core/log.h"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "scene/components.h"
#include "scene/entity.h"
#include "scene/scene.h"
#include "yaml-cpp/emitter.h"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/parse.h"
#include "yaml-cpp/yaml.h"

namespace YAML {
template <>
struct convert<glm::vec2> {
  static Node encode(const glm::vec2& rhs) {
    Node node;
    node.push_back(rhs.x);
    node.push_back(rhs.y);
    return node;
  }

  static bool decode(const Node& node, glm::vec2& rhs) {
    if (!node.IsSequence() || node.size() != 2)
      return false;
    rhs.x = node[0].as<float>();
    rhs.y = node[1].as<float>();
    return true;
  }
};

template <>
struct convert<glm::vec3> {
  static Node encode(const glm::vec3& rhs) {
    Node node;
    node.push_back(rhs.x);
    node.push_back(rhs.y);
    node.push_back(rhs.z);
    return node;
  }

  static bool decode(const Node& node, glm::vec3& rhs) {
    if (!node.IsSequence() || node.size() != 3) {
      return false;
    }

    rhs.x = node[0].as<float>();
    rhs.y = node[1].as<float>();
    rhs.z = node[2].as<float>();
    return true;
  }
};

template <>
struct convert<glm::vec4> {
  static Node encode(const glm::vec4& rhs) {
    Node node;
    node.push_back(rhs.x);
    node.push_back(rhs.y);
    node.push_back(rhs.z);
    node.push_back(rhs.w);
    return node;
  }

  static bool decode(const Node& node, glm::vec4& rhs) {
    if (!node.IsSequence() || node.size() != 4) {
      return false;
    }

    rhs.x = node[0].as<float>();
    rhs.y = node[1].as<float>();
    rhs.z = node[2].as<float>();
    rhs.w = node[3].as<float>();
    return true;
  }
};

}  // namespace YAML

namespace ck {

// ----------------------------------------------------------------------------: Prepare

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v) {
  out << YAML::Flow;
  out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
  return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v) {
  out << YAML::Flow;
  out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
  return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v) {
  out << YAML::Flow;
  out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
  return out;
}

// ----------------------------------------------------------------------------: Serializer

SceneSerializer::SceneSerializer(const Ref<Scene>& scene) : scene_(scene) {}

SceneSerializer::~SceneSerializer() {}

static void SerializeEntity(YAML::Emitter& out, const Entity& entity) {
  // ----------------------------------------------------------------------------: Entity Start
  CK_ENGINE_ASSERT(entity.HasComponent<IDComponent>(), "Entity missing IDComponent");

  out << YAML::BeginMap;
  out << YAML::Key << "Entity" << YAML::Value
      << entity.GetUUID();

  // ----------------------------------------------------------------------------: Tag
  if (entity.HasComponent<TagComponent>()) {
    out << YAML::Key << "TagComponent";
    out << YAML::BeginMap;

    const auto& tag = entity.GetComponent<TagComponent>();
    out << YAML::Key << "Tag" << YAML::Value << tag.name;

    out << YAML::EndMap;
  }

  // ----------------------------------------------------------------------------: Transform
  if (entity.HasComponent<TransformComponent>()) {
    out << YAML::Key << "TransformComponent";
    out << YAML::BeginMap;

    auto& tc = entity.GetComponent<TransformComponent>();
    out << YAML::Key << "Translation" << YAML::Value << tc.position;
    out << YAML::Key << "Rotation" << YAML::Value << tc.rotation;
    out << YAML::Key << "Scale" << YAML::Value << tc.scale;

    out << YAML::EndMap;
  }

  // ----------------------------------------------------------------------------: Camera
  if (entity.HasComponent<CameraComponent>()) {
    out << YAML::Key << "CameraComponent";
    out << YAML::BeginMap;

    auto& cameraComponent = entity.GetComponent<CameraComponent>();
    auto& camera = cameraComponent.camera;

    out << YAML::Key << "Camera" << YAML::Value;
    out << YAML::BeginMap;
    out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
    out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
    out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
    out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
    out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
    out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
    out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
    out << YAML::EndMap;

    out << YAML::Key << "Primary" << YAML::Value << cameraComponent.is_primary;
    out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.is_fixed_aspect_ratio;

    out << YAML::EndMap;
  }

  // ----------------------------------------------------------------------------: SpriteRenderer
  if (entity.HasComponent<SpriteRendererComponent>()) {
    out << YAML::Key << "SpriteRendererComponent";
    out << YAML::BeginMap;

    auto& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
    out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.color;

    out << YAML::EndMap;
  }

  // ----------------------------------------------------------------------------: CircleRenderer
  if (entity.HasComponent<CircleRendererComponent>()) {
    out << YAML::Key << "CircleRendererComponent";
    out << YAML::BeginMap;

    auto& circle = entity.GetComponent<CircleRendererComponent>();
    out << YAML::Key << "Color" << YAML::Value << circle.color;
    out << YAML::Key << "Thickness" << YAML::Value << circle.thickness;
    out << YAML::Key << "Fade" << YAML::Value << circle.fade;

    out << YAML::EndMap;
  }

  // ----------------------------------------------------------------------------: Rigidbody2D
  if (entity.HasComponent<Rigidbody2DComponent>()) {
    out << YAML::Key << "Rigidbody2DComponent";
    out << YAML::BeginMap;

    auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
    out << YAML::Key << "BodyType" << YAML::Value << (int)rb2d.body_type;
    out << YAML::Key << "FixedRotation" << YAML::Value << rb2d.fixed_rotation;

    out << YAML::EndMap;
  }

  // ----------------------------------------------------------------------------: BoxCollider2D
  if (entity.HasComponent<BoxCollider2DComponent>()) {
    out << YAML::Key << "BoxCollider2DComponent";
    out << YAML::BeginMap;

    auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
    out << YAML::Key << "Offset" << YAML::Value << bc2d.offset;
    out << YAML::Key << "Size" << YAML::Value << bc2d.size;
    out << YAML::Key << "Density" << YAML::Value << bc2d.density;
    out << YAML::Key << "Friction" << YAML::Value << bc2d.friction;
    out << YAML::Key << "Restitution" << YAML::Value << bc2d.restitution;
    out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2d.restitution_threshold;

    out << YAML::EndMap;
  }

  // ----------------------------------------------------------------------------: CircleCollider2D
  if (entity.HasComponent<CircleCollider2DComponent>()) {
    out << YAML::Key << "CircleCollider2DComponent";
    out << YAML::BeginMap;

    auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
    out << YAML::Key << "Offset" << YAML::Value << cc2d.offset;
    out << YAML::Key << "Radius" << YAML::Value << cc2d.radius;
    out << YAML::Key << "Density" << YAML::Value << cc2d.density;
    out << YAML::Key << "Friction" << YAML::Value << cc2d.friction;
    out << YAML::Key << "Restitution" << YAML::Value << cc2d.restitution;
    out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc2d.restitution_threshold;

    out << YAML::EndMap;
  }

  // ----------------------------------------------------------------------------: Entity End
  out << YAML::EndMap;  // Entity
}

void SceneSerializer::Serialize(const std::string& filepath) {
  YAML::Emitter out;
  out << YAML::BeginMap;
  out << YAML::Key << "Scene" << YAML::Value << "Untitled";
  out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
  auto view = scene_->registry_.view<entt::entity>();
  for (auto entity_id : view) {
    Entity entity = {entity_id, scene_.get()};
    if (!entity) {
      return;
    }
    SerializeEntity(out, entity);
  }
  out << YAML::EndSeq;
  out << YAML::EndMap;

  std::ofstream fout(filepath);
  fout << out.c_str();
}

bool SceneSerializer::Deserialize(const std::string& filepath) {
  YAML::Node data;
  try {
    data = YAML::LoadFile(filepath);
  } catch (YAML::ParserException e) {
    CK_ENGINE_ERROR("Failed to load scene file '{0}'\n  {1}", filepath, e.what());
    return false;
  }

  if (!data["Scene"]) {
    return false;
  }

  std::string scene_name = data["Scene"].as<std::string>();
  CK_ENGINE_TRACE("Deserializing scene '{0}'", scene_name);

  auto entities = data["Entities"];
  if (entities) {
    for (auto entity : entities) {
      uint64_t uuid = entity["Entity"].as<uint64_t>();

      std::string name;
      auto tag_comp = entity["TagComponent"];
      if (tag_comp) {
        name = tag_comp["Tag"].as<std::string>();
      }

      CK_ENGINE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

      Entity deserialized_entity = scene_->CreateEntityWithUUID(uuid, name);

      auto transform_comp = entity["TransformComponent"];
      if (transform_comp) {
        // Entities always have transforms
        auto& tc = deserialized_entity.GetComponent<TransformComponent>();
        tc.position = transform_comp["Translation"].as<glm::vec3>();
        tc.rotation = transform_comp["Rotation"].as<glm::vec3>();
        tc.scale = transform_comp["Scale"].as<glm::vec3>();
      }

      auto camera_comp = entity["CameraComponent"];
      if (camera_comp) {
        auto& cc = deserialized_entity.AddComponent<CameraComponent>();

        const auto& camera_props = camera_comp["Camera"];
        cc.camera.SetProjectionType(
            (SceneCamera::ProjectionType)camera_props["ProjectionType"].as<int>());

        cc.camera.SetPerspectiveVerticalFOV(camera_props["PerspectiveFOV"].as<float>());
        cc.camera.SetPerspectiveNearClip(camera_props["PerspectiveNear"].as<float>());
        cc.camera.SetPerspectiveFarClip(camera_props["PerspectiveFar"].as<float>());

        cc.camera.SetOrthographicSize(camera_props["OrthographicSize"].as<float>());
        cc.camera.SetOrthographicNearClip(camera_props["OrthographicNear"].as<float>());
        cc.camera.SetOrthographicFarClip(camera_props["OrthographicFar"].as<float>());

        cc.is_primary = camera_comp["Primary"].as<bool>();
        cc.is_fixed_aspect_ratio = camera_comp["FixedAspectRatio"].as<bool>();
      }

      auto sprite_renderer_comp = entity["SpriteRendererComponent"];
      if (sprite_renderer_comp) {
        auto& src = deserialized_entity.AddComponent<SpriteRendererComponent>();
        src.color = sprite_renderer_comp["Color"].as<glm::vec4>();
      }

      auto circle_renderer_comp = entity["CircleRendererComponent"];
      if (circle_renderer_comp) {
        auto& crc = deserialized_entity.AddComponent<CircleRendererComponent>();
        crc.color = circle_renderer_comp["Color"].as<glm::vec4>();
        crc.thickness = circle_renderer_comp["Thickness"].as<float>();
        crc.fade = circle_renderer_comp["Fade"].as<float>();
      }

      auto rigidbody2d_comp = entity["Rigidbody2DComponent"];
      if (rigidbody2d_comp) {
        auto& rb2d = deserialized_entity.AddComponent<Rigidbody2DComponent>();
        rb2d.body_type = (Rigidbody2DComponent::BodyType)rigidbody2d_comp["BodyType"].as<int>();
        rb2d.fixed_rotation = rigidbody2d_comp["FixedRotation"].as<bool>();
      }

      auto box_collider2d_comp = entity["BoxCollider2DComponent"];
      if (box_collider2d_comp) {
        auto& bc2d = deserialized_entity.AddComponent<BoxCollider2DComponent>();
        bc2d.offset = box_collider2d_comp["Offset"].as<glm::vec2>();
        bc2d.size = box_collider2d_comp["Size"].as<glm::vec2>();
        bc2d.density = box_collider2d_comp["Density"].as<float>();
        bc2d.friction = box_collider2d_comp["Friction"].as<float>();
        bc2d.restitution = box_collider2d_comp["Restitution"].as<float>();
        bc2d.restitution_threshold = box_collider2d_comp["RestitutionThreshold"].as<float>();
      }

      auto circle_collider2d_comp = entity["CircleCollider2DComponent"];
      if (circle_collider2d_comp) {
        auto& cc2d = deserialized_entity.AddComponent<CircleCollider2DComponent>();
        cc2d.offset = circle_collider2d_comp["Offset"].as<glm::vec2>();
        cc2d.radius = circle_collider2d_comp["Radius"].as<float>();
        cc2d.density = circle_collider2d_comp["Density"].as<float>();
        cc2d.friction = circle_collider2d_comp["Friction"].as<float>();
        cc2d.restitution = circle_collider2d_comp["Restitution"].as<float>();
        cc2d.restitution_threshold =
            circle_collider2d_comp["RestitutionThreshold"].as<float>();
      }
    }
  }

  return true;
}

// ----------------------------------------------------------------------------: Runtime

void SceneSerializer::SerializeRuntime(const std::string& filepath) {
  CK_ENGINE_ASSERT(false, "not impl yet");
}

bool SceneSerializer::DeserializeRuntime(const std::string& filepath) {
  CK_ENGINE_ASSERT(false, "not impl yet");
  return false;
}

}  // namespace ck
