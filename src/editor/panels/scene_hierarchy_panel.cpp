#include "scene_hierarchy_panel.h"
#include <sec_api/string_s.h>
#include <cstdint>
#include <cstring>
#include <string>
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
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
