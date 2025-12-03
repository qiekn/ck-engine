#include "sandbox_2d.h"
#include <cstdint>

#include "core/profile_timer.h"
#include "debug/profiler.h"
#include "glm/ext/vector_float4.hpp"
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

void Sandbox2D::OnDetach() {
  CK_PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(ck::DeltaTime dt) {
  CK_PROFILE_FUNCTION();

  camera_controller_.OnUpdate(dt);

  ck::Renderer2D::ResetStats();
  {
    CK_PROFILE_SCOPE("Renderer Prep");
    ck::RenderCommand::SetClearColor(background_color_);
    ck::RenderCommand::Clear();
  }

  {
    static float rotation = 0.0f;
    rotation += dt * 50.0f;

    CK_PROFILE_SCOPE("Renderer Draw");
    // clang-format off
    ck::Renderer2D::BeginScene(camera_controller_.Camera());
    ck::Renderer2D::DrawRotatedQuad({1.0f, 0.0f}, {0.8f, 0.8f}, -45.0f, {0.8f, 0.2f, 0.3, 1.0f});
    ck::Renderer2D::DrawQuad({-1.0f, 0.0f}, {0.8f, 0.8f}, color_1_);
    ck::Renderer2D::DrawQuad({0.5f, -0.5f}, {0.5f, 0.75f}, color_2_);
    ck::Renderer2D::DrawQuad({0.0f, 0.0f, -0.1f}, {20.0f, 20.0f}, checkboard_texture_, 10.0f);
    ck::Renderer2D::DrawRotatedQuad({-2.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, rotation, checkboard_texture_, 20.0f);
    ck::Renderer2D::EndScene();
    // clang-format on

    ck::Renderer2D::BeginScene(camera_controller_.Camera());
    for (float y = -5.0f; y < 5.0f; y += 0.5f) {
      for (float x = -5.0f; x < 5.0f; x += 0.5f) {
        glm::vec4 color = {(x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f};
        ck::Renderer2D::DrawQuad({x, y}, {0.45, 0.45f}, color);
      }
    }
    ck::Renderer2D::EndScene();
  }
}

void Sandbox2D::OnImGuiRender() {
  CK_PROFILE_FUNCTION();

  ImGui::Begin("Settings");

  ImGui::Text("Edit Colors");
  ImGui::ColorEdit4("Background Color", glm::value_ptr(background_color_));
  ImGui::ColorEdit4("Color 1", glm::value_ptr(color_1_));
  ImGui::ColorEdit4("Color 2", glm::value_ptr(color_2_));

  auto stats = ck::Renderer2D::GetStats();
  ImGui::Text("Renderer2D Stats:");
  ImGui::Text("Draw Calls: %d", stats.draw_calls);
  ImGui::Text("Quads: %d", stats.quad_count);
  ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
  ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

  ImGui::End();
}

void Sandbox2D::OnEvent(ck::Event& event) {
  camera_controller_.OnEvent(event);
}
