#include "scene_hierarchy_panel.h"
#include <sec_api/string_s.h>
#include <cstdint>
#include <cstring>
#include <string>
#include "glm/ext/vector_float3.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/trigonometric.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "scene/scene.h"

namespace ck {

SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context) {
  SetContext(context);
}

void SceneHierarchyPanel::SetContext(const Ref<Scene>& context) {
  context_ = context;
  selection_context_ = {};
}

void SceneHierarchyPanel::OnImGuiRender() {
  ImGui::Begin("Hierarchy");
  // Get all entities and process everyone
  auto entities = context_->registry_.view<entt::entity>();
  for (auto entity_id : entities) {
    Entity entity{entity_id, context_.get()};
    DrawEntityNode(entity);
  }
  if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
    selection_context_ = {};
  }
  // Right-click on blank space
  if (ImGui::BeginPopupContextWindow(
          nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
    if (ImGui::MenuItem("Create Empty Entity")) {
      context_->CreateEntity("Empty Entity");
    }
    ImGui::EndPopup();
  }
  ImGui::End();  // end of hierarchy

  ImGui::Begin("Properties");
  if (selection_context_) {
    DrawComponents(selection_context_);
  }
  ImGui::End();  // end of properties
}

void SceneHierarchyPanel::SetSelectedEntity(Entity entity) {
  selection_context_ = entity;
}

void SceneHierarchyPanel::DrawEntityNode(const Entity& entity) {
  auto& tag = entity.GetComponent<TagComponent>();

  ImGuiTreeNodeFlags flags = ((selection_context_ == entity) ? ImGuiTreeNodeFlags_Selected : 0) |
                             ImGuiTreeNodeFlags_OpenOnArrow;
  flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
  auto id = (uint64_t)(uint32_t)entity.GetID();
  bool is_open = ImGui::TreeNodeEx((void*)id, flags, "%s", tag.name.c_str());
  if (ImGui::IsItemClicked()) {
    selection_context_ = entity;
  }

  bool is_entity_deleted = false;
  if (ImGui::BeginPopupContextItem()) {
    if (ImGui::MenuItem("Delete Entity")) {
      is_entity_deleted = true;
    }
    ImGui::EndPopup();
  }

  if (is_open) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    bool opened = ImGui::TreeNodeEx((void*)9817239, flags, "%s", tag.name.c_str());
    // TEMP just for testing
    if (opened) {
      ImGui::TreePop();
    }
    ImGui::TreePop();
  }

  if (is_entity_deleted) {
    context_->DestroyEntity(entity);
    if (selection_context_ == entity) {
      selection_context_ = {};
    }
  }
}

static void DrawVec3Control(const std::string& label, glm::vec3& values, float reset_value = 0.0f,
                            float column_width = 100.0f) {
  ImGuiIO& io = ImGui::GetIO();
  auto bold_font = io.Fonts->Fonts[0];

  ImGui::PushID(label.c_str());

  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, column_width);
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn();

  ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

  float line_height = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
  ImVec2 button_size = {line_height + 3.0f, line_height};

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
  ImGui::PushFont(bold_font);
  if (ImGui::Button("X", button_size)) {
    values.x = reset_value;
  }
  ImGui::PopFont();
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
  ImGui::PopItemWidth();
  ImGui::SameLine();

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
  ImGui::PushFont(bold_font);
  if (ImGui::Button("Y", button_size)) {
    values.y = reset_value;
  }
  ImGui::PopFont();
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
  ImGui::PopItemWidth();
  ImGui::SameLine();

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
  ImGui::PushFont(bold_font);
  if (ImGui::Button("Z", button_size)) {
    values.z = reset_value;
  }
  ImGui::PopFont();
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
  ImGui::PopItemWidth();

  ImGui::PopStyleVar(2);

  ImGui::Columns(1);

  ImGui::PopID();
}

template <typename T, typename UIFunction>
static void DrawComponent(const std::string& name, Entity entity, UIFunction ui_function) {
  ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_DefaultOpen;
  tree_node_flags |= ImGuiTreeNodeFlags_Framed;
  tree_node_flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
  tree_node_flags |= ImGuiTreeNodeFlags_AllowItemOverlap;
  tree_node_flags |= ImGuiTreeNodeFlags_FramePadding;
  if (entity.HasComponent<T>()) {
    auto& component = entity.GetComponent<T>();
    ImVec2 content_region_available = ImGui::GetContentRegionAvail();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
    float line_height = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
    ImGui::Separator();
    bool is_open =
        ImGui::TreeNodeEx((void*)typeid(T).hash_code(), tree_node_flags, "%s", name.c_str());
    ImGui::PopStyleVar();
    ImGui::SameLine(content_region_available.x - line_height * 0.5f);
    if (ImGui::Button("+", ImVec2{line_height, line_height})) {
      ImGui::OpenPopup("ComponentSettings");
    }

    bool is_remove_component = false;
    if (ImGui::BeginPopup("ComponentSettings")) {
      if (ImGui::MenuItem("Remove component")) {
        is_remove_component = true;
      }

      ImGui::EndPopup();
    }

    if (is_open) {
      ui_function(component);
      ImGui::TreePop();
    }

    if (is_remove_component) {
      entity.RemoveComponent<T>();
    }
  }
}

void SceneHierarchyPanel::DrawComponents(Entity& entity) {
  if (entity.HasComponent<TagComponent>()) {
    auto& tag = entity.GetComponent<TagComponent>();

    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    strcpy_s(buffer, sizeof(buffer), tag.name.c_str());
    if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
      tag.name = std::string(buffer);
    }
  }

  ImGui::SameLine();
  ImGui::PushItemWidth(-1);

  if (ImGui::Button("Add Component")) {
    ImGui::OpenPopup("AddComponent");
  }

  if (ImGui::BeginPopup("AddComponent")) {
    if (ImGui::MenuItem("Camera")) {
      if (!selection_context_.HasComponent<CameraComponent>()) {
        selection_context_.AddComponent<CameraComponent>();
      } else {
        CK_ENGINE_WARN("This entity already has the Camera component!");
      }
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("Sprite Renderer")) {

      if (!selection_context_.HasComponent<SpriteRendererComponent>()) {
        selection_context_.AddComponent<SpriteRendererComponent>();
      } else {
        CK_ENGINE_WARN("This entity already has the Camera component!");
      }
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }

  ImGui::PopItemWidth();

  DrawComponent<TransformComponent>("Transform", entity, [](auto& component) {
    DrawVec3Control("Position", component.position);

    glm::vec3 rotation = glm::degrees(component.rotation);
    DrawVec3Control("Rotation", rotation);
    component.rotation = glm::radians(rotation);

    DrawVec3Control("Scale", component.scale, 1.0f);
  });

  DrawComponent<CameraComponent>("Camera", entity, [](auto& component) {
    auto& camera = component.camera;

    ImGui::Checkbox("Primary", &component.is_primary);

    // pt is short for "projection type"
    const char* pt_strings[] = {"Perspective", "Orthographic"};
    const char* current_pt_string = pt_strings[(int)camera.GetProjectionType()];
    if (ImGui::BeginCombo("Projection", current_pt_string)) {
      for (int i = 0; i < 2; i++) {
        bool is_selected = current_pt_string == pt_strings[i];
        if (ImGui::Selectable(pt_strings[i], is_selected)) {
          current_pt_string = pt_strings[i];
          camera.SetProjectionType((SceneCamera::ProjectionType)i);
        }
        if (is_selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }

    if (camera.GetProjectionType() == SceneCamera::ProjectionType::kPerspective) {
      float perspect_vertical_fov = glm::degrees(camera.GetPerspectiveVerticalFOV());
      if (ImGui::DragFloat("Vertical FOV", &perspect_vertical_fov)) {
        camera.SetPerspectiveVerticalFOV(glm::radians(perspect_vertical_fov));
      }

      float perspective_near = camera.GetPerspectiveNearClip();
      if (ImGui::DragFloat("Near", &perspective_near)) {
        camera.SetPerspectiveNearClip(perspective_near);
      }

      float perspective_far = camera.GetPerspectiveFarClip();
      if (ImGui::DragFloat("Far", &perspective_far)) {
        camera.SetPerspectiveFarClip(perspective_far);
      }
    }

    if (camera.GetProjectionType() == SceneCamera::ProjectionType::kOrthographic) {
      float orthoSize = camera.GetOrthographicSize();
      if (ImGui::DragFloat("Size", &orthoSize)) {
        camera.SetOrthographicSize(orthoSize);
      }

      float orthoNear = camera.GetOrthographicNearClip();
      if (ImGui::DragFloat("Near", &orthoNear)) {
        camera.SetOrthographicNearClip(orthoNear);
      }

      float orthoFar = camera.GetOrthographicFarClip();
      if (ImGui::DragFloat("Far", &orthoFar)) {
        camera.SetOrthographicFarClip(orthoFar);
      }

      ImGui::Checkbox("Fixed Aspect Ratio", &component.is_fixed_aspect_ratio);
    }
  });

  DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component) {
    ImGui::ColorEdit4("Color", glm::value_ptr(component.color));
  });
}
}  // namespace ck
