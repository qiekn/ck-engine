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

static void DrawVec3Control(const std::string& label, glm::vec3& values, float reset_value = 0.0f,
                            float column_width = 100.0f) {
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
  if (ImGui::Button("X", button_size)) {
    values.x = reset_value;
  }
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
  ImGui::PopItemWidth();
  ImGui::SameLine();

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
  if (ImGui::Button("Y", button_size)) {
    values.y = reset_value;
  }
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
  ImGui::PopItemWidth();
  ImGui::SameLine();

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
  if (ImGui::Button("Z", button_size)) {
    values.z = reset_value;
  }
  ImGui::PopStyleColor(3);

  ImGui::SameLine();
  ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
  ImGui::PopItemWidth();

  ImGui::PopStyleVar(2);

  ImGui::Columns(1);

  ImGui::PopID();
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
      auto& tc = entity.GetComponent<TransformComponent>();
      DrawVec3Control("Position", tc.position);

      glm::vec3 rotation = glm::degrees(tc.rotation);
      DrawVec3Control("Rotation", tc.rotation);
      tc.rotation = glm::radians(rotation);

      DrawVec3Control("Scale", tc.scale, 1.0f);

      ImGui::TreePop();
    }
  }

  if (entity.HasComponent<CameraComponent>()) {
    if (ImGui::TreeNodeEx((void*)typeid(CameraComponent).hash_code(),
                          ImGuiTreeNodeFlags_DefaultOpen, "Camera")) {
      auto& camera_comp = entity.GetComponent<CameraComponent>();
      auto& camera = camera_comp.camera;

      ImGui::Checkbox("Primary", &camera_comp.is_primary);

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
          ImGui::EndCombo();
        }
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

        ImGui::Checkbox("Fixed Aspect Ratio", &camera_comp.is_fixed_aspect_ratio);
      }
      ImGui::TreePop();
    }
  }  // end of camera component

  if (entity.HasComponent<SpriteRendererComponent>()) {
    if (ImGui::TreeNodeEx((void*)typeid(SpriteRendererComponent).hash_code(),
                          ImGuiTreeNodeFlags_DefaultOpen, "Sprite Renderer")) {
      auto& src = entity.GetComponent<SpriteRendererComponent>();
      ImGui::ColorEdit4("Color", glm::value_ptr(src.color));
      ImGui::TreePop();
    }
  }
}
}  // namespace ck
