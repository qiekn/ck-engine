#pragma once

#include <filesystem>

#include "core/core.h"

namespace ck {

class Scene;

// YAML-backed scene save/load. Hazel-style format:
//   Scene: <name>
//   Entities:
//     - Entity: <handle>
//       TagComponent: { Tag: ... }
//       TransformComponent: { Translation, Rotation, Scale: [x,y,z] }
//       SpriteRendererComponent: { Color: [r,g,b,a], Texture: <path> }
//
// Texture handles are not portable, so SpriteRendererComponent persists
// the asset path; Deserialize re-runs Renderer2D::LoadTexture on load.
class SceneSerializer {
public:
  explicit SceneSerializer(const Ref<Scene>& scene) : scene_(scene) {}

  void Serialize(const std::filesystem::path& path);
  // Returns true on success. Clears the bound scene first; on failure
  // the scene is left empty (caller should react accordingly).
  bool Deserialize(const std::filesystem::path& path);

private:
  Ref<Scene> scene_;
};

}  // namespace ck
