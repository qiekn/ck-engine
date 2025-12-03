#include "editor_layer.h"
#include "core/application.h"
#include "core/layer.h"
#include "debug/profiler.h"
#include "glm/ext/vector_float2.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include "renderer/frame_buffer.h"
#include "renderer/orthographic_camera_controller.h"
#include "renderer/render_command.h"
#include "renderer/renderer_2d.h"
#include "renderer/texture.h"

namespace ck {

EditorLayer::EditorLayer() : Layer("EditorLayer"), camera_controller_(1280.0f / 720.0f) {}

EditorLayer::~EditorLayer() {}

void EditorLayer::OnAttach() {
  CK_PROFILE_FUNCTION();

  checkerboard_texture_ = ck::Texture2D::Create("assets/textures/checkerboard.png");

  ck::FrameBufferSpecification fb_spec;
  fb_spec.width = 1280;
  fb_spec.height = 720;
  frame_buffer_ = ck::FrameBuffer::Create(fb_spec);
}

void EditorLayer::OnDetach() {
  CK_PROFILE_FUNCTION();
}

void EditorLayer::OnUpdate(DeltaTime dt) {
  CK_PROFILE_FUNCTION();

  // Update
  camera_controller_.OnUpdate(dt);

  // Render
  ck::Renderer2D::ResetStats();
  {
    CK_PROFILE_SCOPE("Renderer Prep");
    frame_buffer_->Bind();
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
    ck::Renderer2D::DrawQuad({0.0f, 0.0f, -0.1f}, {20.0f, 20.0f}, checkerboard_texture_, 10.0f);
    ck::Renderer2D::DrawRotatedQuad({-2.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, rotation, checkerboard_texture_, 20.0f);
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
    frame_buffer_->Unbind();
  }
}

void EditorLayer::OnImGuiRender() {
  CK_PROFILE_FUNCTION();

  // Note: Switch this to true to enable dockspace
  static bool dockingEnabled = true;
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
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) {
      window_flags |= ImGuiWindowFlags_NoBackground;
    }

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking,
    // otherwise any change of dockspace/settings would lead to windows being stuck in limbo and
    // never being visible.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
    ImGui::PopStyleVar();

    if (opt_fullscreen) {
      ImGui::PopStyleVar(2);
    }

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

        if (ImGui::MenuItem("Exit")) {
          ck::Application::Get().Close();
        }
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

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("Viewport");
    ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
    if (viewport_size_ != *(glm::vec2*)&viewport_panel_size) {
      frame_buffer_->Resize((uint32_t)viewport_panel_size.x, (uint32_t)viewport_panel_size.y);
      viewport_size_ = {viewport_panel_size.x, viewport_panel_size.y};

      camera_controller_.OnResize(viewport_panel_size.x, viewport_panel_size.y);
    }

    uint64_t texture_id = frame_buffer_->GetColorAttachmentRendererID();
    ImGui::Image(reinterpret_cast<ImTextureID>(texture_id),
                 ImVec2{viewport_size_.x, viewport_size_.y}, ImVec2{0, 1}, ImVec2{1, 0});
    ImGui::End();  // viewport
    ImGui::PopStyleVar();

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

void EditorLayer::OnEvent(Event& event) {
  camera_controller_.OnEvent(event);
}

}  // namespace ck
