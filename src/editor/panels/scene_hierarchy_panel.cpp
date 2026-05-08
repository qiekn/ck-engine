#include "panels/scene_hierarchy_panel.h"

#include <cstdint>

namespace ck_editor {

void SceneHierarchyPanel::OnImGuiRender() {
  ImGui::Begin("Scene Hierarchy");

  if (context_) {
    for (ck::Entity entity : context_->GetAllEntities()) {
      DrawEntityNode(entity);
    }

    // Click on empty area inside the panel deselects (mouse button 0 = Left).
    if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered(0)) {
      selected_ = {};
    }

    // Right-click on empty area: Create Empty Entity. NoOpenOverItems keeps
    // this from stealing the per-item context menu in DrawEntityNode.
    constexpr ImGuiPopupFlags kCreateMenuFlags =
        ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems;
    if (ImGui::BeginPopupContextWindow(nullptr, kCreateMenuFlags)) {
      if (ImGui::MenuItem("Create Empty Entity", nullptr, false, true)) {
        context_->CreateEntity("Empty Entity");
      }
      ImGui::EndPopup();
    }
  }

  ImGui::End();
}

void SceneHierarchyPanel::DrawEntityNode(ck::Entity entity) {
  const auto& tag = entity.GetComponent<ck::TagComponent>().tag;

  ImGuiTreeNodeFlags flags =
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth |
      (selected_ == entity ? ImGuiTreeNodeFlags_Selected : 0);
  void* id = reinterpret_cast<void*>(static_cast<uintptr_t>(static_cast<uint32_t>(entity)));
  bool opened = ImGui::TreeNodeEx(id, flags, "%s", tag.c_str());

  if (ImGui::IsItemClicked(0)) {
    selected_ = entity;
  }

  bool entity_deleted = false;
  if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonRight)) {
    if (ImGui::MenuItem("Delete Entity", nullptr, false, true)) {
      entity_deleted = true;
    }
    ImGui::EndPopup();
  }

  if (opened) {
    ImGui::TreePop();
  }

  if (entity_deleted) {
    if (selected_ == entity) selected_ = {};
    context_->DestroyEntity(entity);
  }
}

}  // namespace ck_editor
