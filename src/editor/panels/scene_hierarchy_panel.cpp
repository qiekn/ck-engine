#include "scene_hierarchy_panel.h"
#include <cstdint>
#include "imgui.h"
#include "scene/components.h"
#include "scene/entity.h"

namespace ck {

SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context) {
  SetContext(context);
}

void SceneHierarchyPanel::SetContext(const Ref<Scene>& context) {
  context_ = context;
}

void SceneHierarchyPanel::OnImGuiRender() {
  ImGui::Begin("Hierarchy");
  // Get all entities and process everyone
  auto entities = context_->registry_.view<entt::entity>();
  for (auto entity_id : entities) {
    Entity entity{entity_id, context_.get()};
    DrawEntityNode(entity);
  }
  ImGui::End();
}

void SceneHierarchyPanel::DrawEntityNode(Entity& entity) {
  auto& tag = entity.GetComponent<TagComponent>();

  ImGuiTreeNodeFlags flags = ((selection_context_ == entity) ? ImGuiTreeNodeFlags_Selected : 0) |
                             ImGuiTreeNodeFlags_OpenOnArrow;
  auto id = (uint64_t)(uint32_t)entity.GetID();
  bool is_open = ImGui::TreeNodeEx((void*)id, flags, "%s", tag.name.c_str());
  if (ImGui::IsItemClicked()) {
    selection_context_ = entity;
  }

  if (is_open) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
    bool opened = ImGui::TreeNodeEx((void*)9817239, flags, "%s", tag.name.c_str());
    // TEMP just for testing
    if (opened) {
      ImGui::TreePop();
    }
    ImGui::TreePop();
  }
}
}  // namespace ck
