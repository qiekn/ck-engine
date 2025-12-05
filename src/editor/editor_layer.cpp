#include "editor_layer.h"
#include "core/application.h"
#include "core/core.h"
#include "core/deltatime.h"
#include "core/input.h"
#include "core/layer.h"
#include "debug/profiler.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "renderer/frame_buffer.h"
#include "renderer/orthographic_camera_controller.h"
#include "renderer/render_command.h"
#include "renderer/renderer_2d.h"
#include "renderer/texture.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "scene/scene.h"
#include "scene/scriptable_entity.h"

namespace ck {

EditorLayer::EditorLayer() : Layer("EditorLayer"), camera_controller_(1280.0f / 720.0f) {}

EditorLayer::~EditorLayer() {}

void EditorLayer::OnAttach() {
  CK_PROFILE_FUNCTION();

  checkerboard_texture_ = Texture2D::Create("assets/textures/checkerboard.png");

  FrameBufferSpecification fb_spec;
  fb_spec.width = 1280;
  fb_spec.height = 720;
  frame_buffer_ = FrameBuffer::Create(fb_spec);

  active_scene_ = CreateRef<Scene>();

  auto square = active_scene_->CreateEntity("Green Square");
  square.AddComponent<SpriteRendererComponent>(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
  square_entity_ = square;

  main_camera_ = active_scene_->CreateEntity("Main Camera");
  main_camera_.AddComponent<CameraComponent>();

  second_camera_ = active_scene_->CreateEntity("Clip-Space Camera");
  auto& cc = second_camera_.AddComponent<CameraComponent>();
  cc.is_primary = false;

  // Native Script
  class CameraController : public ScriptableEntity {
    void OnCreate() {}

    void OnDestroy() {}

    void OnUpdate(DeltaTime dt) {
      auto& transform = GetComponent<TransformComponent>().transform;
      float speed = 5.0f;

      // transform[3][0] = X Translation
      // transform[3][1] = Y Translation
      if (Input::IsKeyPressed(KeyCode::A)) {
        transform[3][0] -= speed * dt;
      }
      if (Input::IsKeyPressed(KeyCode::D)) {
        transform[3][0] += speed * dt;
      }
      if (Input::IsKeyPressed(KeyCode::W)) {
        transform[3][1] += speed * dt;
      }
      if (Input::IsKeyPressed(KeyCode::S)) {
        transform[3][1] -= speed * dt;
      }
    }
  };

  main_camera_.AddComponent<NativeScriptComponent>().Bind<CameraController>();
}

void EditorLayer::OnDetach() {
  CK_PROFILE_FUNCTION();
}

void EditorLayer::OnUpdate(DeltaTime dt) {
  CK_PROFILE_FUNCTION();

  // Resize
  auto spec = frame_buffer_->GetSpecification();
  if (viewport_size_.x > 0.0f && viewport_size_.y > 0.0f &&
      (spec.width != (uint32_t)viewport_size_.x || spec.height != (uint32_t)viewport_size_.y)) {
    frame_buffer_->Resize((uint32_t)viewport_size_.x, (uint32_t)viewport_size_.y);
    camera_controller_.OnResize(viewport_size_.x, viewport_size_.y);
    active_scene_->OnViewportResize((uint32_t)viewport_size_.x, (uint32_t)viewport_size_.y);
  }

  // Update
  if (is_viewprot_focused_) {
    camera_controller_.OnUpdate(dt);
  }

  // Render
  Renderer2D::ResetStats();
  frame_buffer_->Bind();
  RenderCommand::SetClearColor(background_color_);
  RenderCommand::Clear();

  active_scene_->OnUpdate(dt);

  frame_buffer_->Unbind();
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
          Application::Get().Close();
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

    if (square_entity_) {
      ImGui::Separator();
      auto& tag = square_entity_.GetComponent<TagComponent>().name;
      ImGui::Text("%s", tag.c_str());

      auto& square_color = square_entity_.GetComponent<SpriteRendererComponent>().color;
      ImGui::ColorEdit4("Square Entity Color", glm::value_ptr(square_color));
    }

    ImGui::DragFloat3("Camera Transform",
                      glm::value_ptr(main_camera_.GetComponent<TransformComponent>().transform[3]));

    if (ImGui::Checkbox("Camera A", &is_primary_camera)) {
      main_camera_.GetComponent<CameraComponent>().is_primary = is_primary_camera;
      second_camera_.GetComponent<CameraComponent>().is_primary = !is_primary_camera;
    }

    {
      auto& camera = second_camera_.GetComponent<CameraComponent>().camera;
      float ortho_size = camera.GetOrthographicSize();
      if (ImGui::DragFloat("Second Camera Ortho Size", &ortho_size)) {
        camera.SetOrthographicSize(ortho_size);
      }
    }

    auto stats = Renderer2D::GetStats();
    ImGui::Text("Renderer2D Stats:");
    ImGui::Text("Draw Calls: %d", stats.draw_calls);
    ImGui::Text("Quads: %d", stats.quad_count);
    ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
    ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

    // debug viewport hover & focus
    ImGui::Separator();
    ImGui::Text("Debug Viewpoint");
    ImGui::Text("Hovered: %s", is_viewport_hovered_ ? "true " : "false");
    ImGui::Text("Focused: %s", is_viewprot_focused_ ? "true" : "false");

    ImGui::End();  // settings

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("Viewport");
    is_viewprot_focused_ = ImGui::IsWindowFocused();
    is_viewport_hovered_ = ImGui::IsWindowHovered();
    Application::Get().GetImGuiLayer()->BlockEvent(!is_viewport_hovered_ || !is_viewprot_focused_);
    ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
    viewport_size_ = {viewport_panel_size.x, viewport_panel_size.y};

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
    auto stats = Renderer2D::GetStats();
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
