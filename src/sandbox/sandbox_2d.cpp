#include "sandbox_2d.h"

#include <cstring>

#include "core/profile_timer.h"
#include "debug/profiler.h"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include "renderer/orthographic_camera_controller.h"
#include "renderer/render_command.h"
#include "renderer/renderer_2d.h"
#include "renderer/texture.h"

Sandbox2D::Sandbox2D() : Layer("Sandbox 2D"), camera_controller_(16.0f / 9.0f) {}

void Sandbox2D::OnAttach() {
  CK_PROFILE_FUNCTION();
  checkboard_texture_ = ck::Texture2D::Create("assets/textures/checkerboard.png");
}

void Sandbox2D::OnDetach() { CK_PROFILE_FUNCTION(); }

void Sandbox2D::OnUpdate(ck::DeltaTime dt) {
  CK_PROFILE_FUNCTION();

  camera_controller_.OnUpdate(dt);

  {
    CK_PROFILE_SCOPE("Renderer Prep");
    ck::RenderCommand::SetClearColor(background_color_);
    ck::RenderCommand::Clear();
  }

  {
    CK_PROFILE_SCOPE("Renderer Draw");
    ck::Renderer2D::BeginScene(camera_controller_.Camera());
    ck::Renderer2D::DrawQuad(quad_pos_1_, quad_size_1_, quad_color_1_);
    ck::Renderer2D::DrawQuad({0.5f, -0.5f}, {0.5f, 0.75f}, quad_color_2_);
    ck::Renderer2D::DrawQuad({0.0f, 0.0f, -0.1f}, {10.0f, 10.0f}, checkboard_texture_);
    ck::Renderer2D::EndScene();
  }
}

void Sandbox2D::OnImGuiRender() {
  ImGui::Begin("Settings");
  ImGui::ColorEdit4("Background Color", glm::value_ptr(background_color_));
  ImGui::DragFloat2("Quad Pos 1", glm::value_ptr(quad_pos_1_));
  ImGui::DragFloat2("Quad Size 1", glm::value_ptr(quad_size_1_));
  ImGui::ColorEdit4("Quad Color 1", glm::value_ptr(quad_color_1_));
  ImGui::ColorEdit4("Quad Color 2", glm::value_ptr(quad_color_2_));

  for (auto& result : profile_results_) {
    char label[50];
    strcpy(label, "%.3fms ");
    strcat(label, result.name);
    ImGui::Text(label, result.time);
  }
  profile_results_.clear();
  ImGui::End();
}

void Sandbox2D::OnEvent(ck::Event& event) { camera_controller_.OnEvent(event); }
