#include "panels/properties_panel.h"

#include <cstring>
#include <numbers>

namespace ck_editor {
namespace {

constexpr float kRadToDeg = 180.0f / std::numbers::pi_v<float>;
constexpr float kDegToRad = std::numbers::pi_v<float> / 180.0f;

void DrawTag(ck::Entity entity) {
  if (!entity.HasComponent<ck::TagComponent>()) return;
  auto& tag = entity.GetComponent<ck::TagComponent>().tag;

  // Fixed-size buffer so we don't pull in misc/cpp/imgui_stdlib.h.
  char buf[256] = {};
  std::strncpy(buf, tag.c_str(), sizeof(buf) - 1);
  if (ImGui::InputText("Tag", buf, sizeof(buf), 0, nullptr, nullptr)) {
    tag = buf;
  }
}

void DrawTransform(ck::Entity entity) {
  if (!entity.HasComponent<ck::TransformComponent>()) return;
  if (!ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) return;
  auto& t = entity.GetComponent<ck::TransformComponent>();

  ImGui::DragFloat3("Translation", &t.translation.x, 0.1f, 0.0f, 0.0f, "%.3f", 0);

  // Edit rotation in degrees, store radians (matches Hazel's PropertiesPanel).
  glm::vec3 deg = t.rotation * kRadToDeg;
  if (ImGui::DragFloat3("Rotation", &deg.x, 1.0f, 0.0f, 0.0f, "%.1f", 0)) {
    t.rotation = deg * kDegToRad;
  }

  ImGui::DragFloat3("Scale", &t.scale.x, 0.1f, 0.0f, 0.0f, "%.3f", 0);
}

void DrawSpriteRenderer(ck::Entity entity) {
  if (!entity.HasComponent<ck::SpriteRendererComponent>()) return;
  if (!ImGui::CollapsingHeader("Sprite Renderer", ImGuiTreeNodeFlags_DefaultOpen)) return;
  auto& s = entity.GetComponent<ck::SpriteRendererComponent>();
  ImGui::ColorEdit4("Color", &s.color.x, 0);
}

}  // namespace

void PropertiesPanel::OnImGuiRender(ck::Entity selected) {
  ImGui::Begin("Properties");
  if (selected) {
    DrawTag(selected);
    ImGui::Separator();
    DrawTransform(selected);
    DrawSpriteRenderer(selected);
  }
  ImGui::End();
}

}  // namespace ck_editor
