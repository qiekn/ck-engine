#include "scene_hierarchy_panel.h"
#include <sec_api/string_s.h>
#include <cstdint>
#include <cstring>
#include <string>
#include "glm/gtc/type_ptr.hpp"
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

  ImGui::Begin("Properties");
  if (selection_context_) {
    DrawComponents(selection_context_);
  }
  ImGui::End();
}

void SceneHierarchyPanel::DrawEntityNode(const Entity& entity) {
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

void SceneHierarchyPanel::DrawComponents(Entity& entity) {
  if (entity.HasComponent<TagComponent>()) {
    auto& tag = entity.GetComponent<TagComponent>();

    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    strcpy_s(buffer, sizeof(buffer), tag.name.c_str());
    if (ImGui::InputText("Tag", buffer, sizeof(buffer))) {
      tag.name = std::string(buffer);
    }
  }

  if (entity.HasComponent<TransformComponent>()) {
    if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(),
                          ImGuiTreeNodeFlags_DefaultOpen, "Transform")) {
      auto& transform = entity.GetComponent<TransformComponent>().transform;
      ImGui::DragFloat3("Position", glm::value_ptr(transform[3]), 0.1f);

      ImGui::TreePop();
    }
  }
}
}  // namespace ck
