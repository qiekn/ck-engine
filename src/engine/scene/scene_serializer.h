#pragma once

#include <string>
#include "core/core.h"
#include "scene/scene.h"

namespace ck {
class SceneSerializer {
public:
  explicit SceneSerializer(const Ref<Scene>& scene);
  virtual ~SceneSerializer();

  void Serialize(const std::string& filepath);
  void SerializeRuntime(const std::string& filepath);

  bool Deserialize(const std::string& filepath);
  bool DeserializeRuntime(const std::string& filepath);

private:
  Ref<Scene> scene_;
};
}  // namespace ck
