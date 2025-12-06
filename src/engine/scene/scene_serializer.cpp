#include "scene_serializer.h"
#include "core/log.h"
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
  out << YAML::BeginMap;
  out << YAML::Key << "Entity" << YAML::Value
      << "12837192831273";  // TODO(qiekn): Entity ID goes here

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
  std::ifstream stream(filepath);
  std::stringstream strStream;
  strStream << stream.rdbuf();

  YAML::Node data = YAML::Load(strStream.str());
  if (!data["Scene"]) {
    return false;
  }

  std::string scene_name = data["Scene"].as<std::string>();
  CK_ENGINE_TRACE("Deserializing scene '{0}'", scene_name);

  auto entities = data["Entities"];
  if (entities) {
    for (auto entity : entities) {
      uint64_t uuid = entity["Entity"].as<uint64_t>();  // TODO(qiekn)

      std::string name;
      auto tag_comp = entity["TagComponent"];
      if (tag_comp) {
        name = tag_comp["Tag"].as<std::string>();
      }

      CK_ENGINE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

      Entity deserialized_entity = scene_->CreateEntity(name);

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
