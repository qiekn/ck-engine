#pragma once

#include "core/core.h"
#include "scene/entity.h"
#include "scene/scene.h"

namespace ck {
class SceneHierarchyPanel {
public:
  SceneHierarchyPanel() = default;
  explicit SceneHierarchyPanel(const Ref<Scene>& scene);

  void SetContext(const Ref<Scene>& scene);

  void OnImGuiRender();

private:
  void DrawEntityNode(Entity& entity);

private:
  Ref<Scene> context_;
  Entity selection_context_;
};
}  // namespace ck
