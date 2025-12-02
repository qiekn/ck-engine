#include "sandbox_2d.h"

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

void Sandbox2D::OnDetach() { CK_PROFILE_FUNCTION(); }

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

  /*
   */

  // Note: Switch this to true to enable dockspace
  static bool dockingEnabled = false;
  if (dockingEnabled) {
    static bool dockspaceOpen = true;
    static bool opt_fullscreen_persistant = true;
    bool opt_fullscreen = opt_fullscreen_persistant;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen) {
      ImGuiViewport* viewport = ImGui::GetMainViewport();
      ImGui::SetNextWindowPos(viewport->Pos);
      ImGui::SetNextWindowSize(viewport->Size);
      ImGui::SetNextWindowViewport(viewport->ID);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
      window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
      window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and
    // handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
      window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking,
    // otherwise any change of dockspace/settings would lead to windows being stuck in limbo and
    // never being visible.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
    ImGui::PopStyleVar();

    if (opt_fullscreen) ImGui::PopStyleVar(2);

    // DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
      ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
      ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        // Disabling fullscreen would allow the window to be moved to the front of other windows,
        // which we can't undo at the moment without finer window depth/z control.
        // ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

        if (ImGui::MenuItem("Exit")) ck::Application::Get().Close();
        ImGui::EndMenu();
      }

      ImGui::EndMenuBar();
    }

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

    ImGui::End();  // settings
    ImGui::End();  // dockspace demo

  } else {
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
}

void Sandbox2D::OnEvent(ck::Event& event) { camera_controller_.OnEvent(event); }
