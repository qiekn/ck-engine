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

  Entity GetSelectedEntity() const { return selection_context_; }
  void SetSelectedEntity(Entity entity);

private:
  void DrawEntityNode(const Entity& entity);
  void DrawComponents(Entity& entity);

private:
  Ref<Scene> context_;
  Entity selection_context_;
};
}  // namespace ck
