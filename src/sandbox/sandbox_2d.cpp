#include "sandbox_2d.h"

#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include "renderer/orthographic_camera_controller.h"
#include "renderer/render_command.h"
#include "renderer/renderer_2d.h"

Sandbox2D::Sandbox2D() : Layer("Sandbox 2D"), camera_controller_(16.0f / 9.0f) {}

void Sandbox2D::OnAttach() {}

void Sandbox2D::OnDetach() {}

void Sandbox2D::OnUpdate(ck::DeltaTime dt) {
  camera_controller_.OnUpdate(dt);

  ck::RenderCommand::SetClearColor(background_color_);
  ck::RenderCommand::Clear();

  ck::Renderer2D::BeginScene(camera_controller_.Camera());
  ck::Renderer2D::DrawQuad({0.0f, 0.0f}, {1.0f, 1.0f}, square_color_);
  ck::Renderer2D::EndScene();
}

void Sandbox2D::OnImGuiRender() {
  ImGui::Begin("Settings");
  ImGui::ColorEdit4("Background Color", glm::value_ptr(background_color_));
  ImGui::ColorEdit4("Square Color", glm::value_ptr(square_color_));
  ImGui::End();
}

void Sandbox2D::OnEvent(ck::Event& event) { camera_controller_.OnEvent(event); }
