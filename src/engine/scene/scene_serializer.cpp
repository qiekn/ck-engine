#include "scene/scene_serializer.h"

#include <fstream>

#include <yaml-cpp/yaml.h>

#include "core/log.h"
#include "renderer/renderer_2d.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "scene/scene.h"

namespace YAML {

template <>
struct convert<glm::vec3> {
  static Node encode(const glm::vec3& v) {
    Node n;
    n.push_back(v.x);
    n.push_back(v.y);
    n.push_back(v.z);
    n.SetStyle(EmitterStyle::Flow);
    return n;
  }
  static bool decode(const Node& n, glm::vec3& v) {
    if (!n.IsSequence() || n.size() != 3) return false;
    v.x = n[0].as<float>();
    v.y = n[1].as<float>();
    v.z = n[2].as<float>();
    return true;
  }
};

template <>
struct convert<glm::vec4> {
  static Node encode(const glm::vec4& v) {
    Node n;
    n.push_back(v.x);
    n.push_back(v.y);
    n.push_back(v.z);
    n.push_back(v.w);
    n.SetStyle(EmitterStyle::Flow);
    return n;
  }
  static bool decode(const Node& n, glm::vec4& v) {
    if (!n.IsSequence() || n.size() != 4) return false;
    v.x = n[0].as<float>();
    v.y = n[1].as<float>();
    v.z = n[2].as<float>();
    v.w = n[3].as<float>();
    return true;
  }
};

}  // namespace YAML

namespace ck {

namespace {

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v) {
  out << YAML::Flow << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
  return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v) {
  out << YAML::Flow << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
  return out;
}

void SerializeEntity(YAML::Emitter& out, Entity entity) {
  out << YAML::BeginMap;
  out << YAML::Key << "Entity" << YAML::Value << static_cast<uint32_t>(entity);

  if (entity.HasComponent<TagComponent>()) {
    out << YAML::Key << "TagComponent";
    out << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "Tag" << YAML::Value << entity.GetComponent<TagComponent>().tag;
    out << YAML::EndMap;
  }

  if (entity.HasComponent<TransformComponent>()) {
    out << YAML::Key << "TransformComponent";
    out << YAML::Value << YAML::BeginMap;
    const auto& t = entity.GetComponent<TransformComponent>();
    out << YAML::Key << "Translation" << YAML::Value << t.translation;
    out << YAML::Key << "Rotation"    << YAML::Value << t.rotation;
    out << YAML::Key << "Scale"       << YAML::Value << t.scale;
    out << YAML::EndMap;
  }

  if (entity.HasComponent<SpriteRendererComponent>()) {
    out << YAML::Key << "SpriteRendererComponent";
    out << YAML::Value << YAML::BeginMap;
    const auto& s = entity.GetComponent<SpriteRendererComponent>();
    out << YAML::Key << "Color"   << YAML::Value << s.color;
    out << YAML::Key << "Texture" << YAML::Value << s.texture_path;
    out << YAML::Key << "Filter"  << YAML::Value
        << (s.filter == Renderer2D::Filter::Nearest ? "Nearest" : "Linear");
    out << YAML::EndMap;
  }

  out << YAML::EndMap;
}

}  // namespace

void SceneSerializer::Serialize(const std::filesystem::path& path) {
  YAML::Emitter out;
  out << YAML::BeginMap;
  out << YAML::Key << "Scene" << YAML::Value << "Untitled";
  out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

  for (Entity e : scene_->GetAllEntities()) {
    SerializeEntity(out, e);
  }

  out << YAML::EndSeq;
  out << YAML::EndMap;

  std::filesystem::create_directories(path.parent_path());
  std::ofstream f(path);
  f << out.c_str();
  log::info("Scene saved to {}", path.string());
}

bool SceneSerializer::Deserialize(const std::filesystem::path& path) {
  YAML::Node root;
  try {
    root = YAML::LoadFile(path.string());
  } catch (const YAML::Exception& e) {
    log::error("Scene load failed ({}): {}", path.string(), e.what());
    return false;
  }
  if (!root["Entities"]) {
    log::error("Scene file {} has no Entities sequence", path.string());
    return false;
  }

  scene_->Clear();
  for (const auto& entity_node : root["Entities"]) {
    std::string tag = "Entity";
    if (auto tc = entity_node["TagComponent"]) {
      tag = tc["Tag"].as<std::string>("Entity");
    }
    Entity e = scene_->CreateEntity(tag);

    if (auto tc = entity_node["TransformComponent"]) {
      auto& t = e.GetComponent<TransformComponent>();
      t.translation = tc["Translation"].as<glm::vec3>(glm::vec3{0.0f});
      t.rotation    = tc["Rotation"]   .as<glm::vec3>(glm::vec3{0.0f});
      t.scale       = tc["Scale"]      .as<glm::vec3>(glm::vec3{1.0f});
    }

    if (auto sc = entity_node["SpriteRendererComponent"]) {
      auto& s = e.AddComponent<SpriteRendererComponent>();
      s.color = sc["Color"].as<glm::vec4>(glm::vec4{1.0f});
      s.texture_path = sc["Texture"].as<std::string>(std::string{});
      const std::string filter = sc["Filter"].as<std::string>(std::string{"Linear"});
      s.filter = (filter == "Nearest") ? Renderer2D::Filter::Nearest
                                       : Renderer2D::Filter::Linear;
      if (!s.texture_path.empty()) {
        s.texture = Renderer2D::LoadTexture(s.texture_path, s.filter);
      }
    }
  }
  log::info("Scene loaded from {}", path.string());
  return true;
}

}  // namespace ck
