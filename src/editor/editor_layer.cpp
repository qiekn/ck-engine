#include "editor_layer.h"
#include <optional>
#include <string>
#include "core/application.h"
#include "core/core.h"
#include "core/deltatime.h"
#include "core/input.h"
#include "core/layer.h"
#include "debug/profiler.h"
#include "events/event.h"
#include "events/key_codes.h"
#include "events/key_event.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include "math/math.h"
#include "renderer/editor_camera.h"
#include "renderer/frame_buffer.h"
#include "renderer/orthographic_camera_controller.h"
#include "renderer/render_command.h"
#include "renderer/renderer_2d.h"
#include "renderer/texture.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "scene/scene.h"
#include "scene/scene_serializer.h"
#include "scene/scriptable_entity.h"
#include "utils/platform_utils.h"

#include "imgui_internal.h"
#include "ImGuizmo.h"

namespace ck {

EditorLayer::EditorLayer() : Layer("EditorLayer"), camera_controller_(1280.0f / 720.0f) {}

EditorLayer::~EditorLayer() {}

void EditorLayer::OnAttach() {
  CK_PROFILE_FUNCTION();

  checkerboard_texture_ = Texture2D::Create("assets/textures/checkerboard.png");

  FrameBufferSpecification fb_spec;
  fb_spec.attachments = {
      {FrameBufferTextureFormat::RGBA8, FrameBufferTextureFormat::RedInteger,
       FrameBufferTextureFormat::Depth}};
  fb_spec.width = 1280;
  fb_spec.height = 720;
  frame_buffer_ = FrameBuffer::Create(fb_spec);

  active_scene_ = CreateRef<Scene>();

  editor_camera_ = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);

#if 0
  auto square = active_scene_->CreateEntity("Green Square");
  square.AddComponent<SpriteRendererComponent>(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
  square_entity_ = square;

  auto red_square = active_scene_->CreateEntity("Red Square");
  red_square.AddComponent<SpriteRendererComponent>(glm::vec4{1.0f, 0.0f, 0.0f, 1.0f});

  main_camera_ = active_scene_->CreateEntity("Camera A");
  main_camera_.AddComponent<CameraComponent>();

  second_camera_ = active_scene_->CreateEntity("Camera B");
  auto& cc = second_camera_.AddComponent<CameraComponent>();
  cc.is_primary = false;

  // Native Script
  class CameraController : public ScriptableEntity {
    void OnCreate() {}

    void OnDestroy() {}

    void OnUpdate(DeltaTime dt) {
      auto& position = GetComponent<TransformComponent>().position;
      float speed = 5.0f;

      if (Input::IsKeyPressed(KeyCode::A)) {
        position.x -= speed * dt;
      }
      if (Input::IsKeyPressed(KeyCode::D)) {
        position.x += speed * dt;
      }
      if (Input::IsKeyPressed(KeyCode::W)) {
        position.y += speed * dt;
      }
      if (Input::IsKeyPressed(KeyCode::S)) {
        position.y -= speed * dt;
      }
    }
  };

  main_camera_.AddComponent<NativeScriptComponent>().Bind<CameraController>();
  second_camera_.AddComponent<NativeScriptComponent>().Bind<CameraController>();
#endif
  scene_hierarachy_panel_.SetContext(active_scene_);
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
    editor_camera_.SetViewportSize(viewport_size_.x, viewport_size_.y);
  }

  // Update
  if (is_viewprot_focused_) {
    camera_controller_.OnUpdate(dt);
  }

  editor_camera_.OnUpdate(dt);

  // Render
  Renderer2D::ResetStats();
  frame_buffer_->Bind();
  RenderCommand::SetClearColor(background_color_);
  RenderCommand::Clear();

  // Clear entity ID attachment to -1
  frame_buffer_->ClearAttachment(1, -1);

  active_scene_->OnUpdateEditor(dt, editor_camera_);

  // Mouse picking
  auto [mx, my] = ImGui::GetMousePos();
  mx -= viewport_bounds_[0].x;
  my -= viewport_bounds_[0].y;
  glm::vec2 viewport_size = viewport_bounds_[1] - viewport_bounds_[0];
  my = viewport_size.y - my;
  int mouse_x = (int)mx;
  int mouse_y = (int)my;

  if (mouse_x >= 0 && mouse_y >= 0 && mouse_x < (int)viewport_size.x &&
      mouse_y < (int)viewport_size.y) {
    int pixel_data = frame_buffer_->ReadPixel(1, mouse_x, mouse_y);
    hovered_entity_ =
        pixel_data == -1 ? Entity() : Entity((entt::entity)pixel_data, active_scene_.get());
  }

  frame_buffer_->Unbind();
}

void EditorLayer::OnImGuiRender() {
  CK_PROFILE_FUNCTION();

  static bool dockspaceOpen = true;
  static bool opt_fullscreen_persistant = true;
  bool opt_fullscreen = opt_fullscreen_persistant;
  static ImGuiDockNodeFlags dockspace_flags =
      ImGuiDockNodeFlags_NoWindowMenuButton;

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
  ImGuiStyle& style = ImGui::GetStyle();
  float window_min_size_x = style.WindowMinSize.x;
  style.WindowMinSize.x = 370.0f;  // set dockspace min width
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
  }
  style.WindowMinSize.x = window_min_size_x;

  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      // Disabling fullscreen would allow the window to be moved to the front of other windows,
      // which we can't undo at the moment without finer window depth/z control.
      // ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

      if (ImGui::MenuItem("New", "Ctrl+N")) {
        NewScene();
      }

      if (ImGui::MenuItem("Open...", "Ctrl+O")) {
        OpenScene();
      }

      if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
        SaveSceneAs();
      }

      if (ImGui::MenuItem("Exit")) {
        Application::Get().Close();
      }
      ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
  }

  scene_hierarachy_panel_.OnImGuiRender();

  ImGui::Begin("Stats");

  std::string name = "None";
  if (hovered_entity_) name = hovered_entity_.GetComponent<TagComponent>().name;
  ImGui::Text("Hovered Entity: %s", name.c_str());

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

  // debug viewport hover & focus
  ImGui::Separator();
  ImGui::Text("Debug Viewpoint");
  ImGui::Text("Hovered: %s", is_viewport_hovered_ ? "true " : "false");
  ImGui::Text("Focused: %s", is_viewprot_focused_ ? "true" : "false");

  ImGui::End();  // settings

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
  ImGui::Begin("Viewport");
  auto viewport_offset = ImGui::GetCursorPos();  // Includes tab bar

  is_viewprot_focused_ = ImGui::IsWindowFocused();
  is_viewport_hovered_ = ImGui::IsWindowHovered();
  Application::Get().GetImGuiLayer()->BlockEvent(!is_viewport_hovered_ && !is_viewprot_focused_);
  ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
  viewport_size_ = {viewport_panel_size.x, viewport_panel_size.y};

  uint64_t texture_id = frame_buffer_->GetColorAttachmentRendererID();
  ImGui::Image(reinterpret_cast<ImTextureID>(texture_id),
               ImVec2{viewport_size_.x, viewport_size_.y}, ImVec2{0, 1}, ImVec2{1, 0});

  auto window_size = ImGui::GetWindowSize();
  ImVec2 min_bound = ImGui::GetWindowPos();
  min_bound.x += viewport_offset.x;
  min_bound.y += viewport_offset.y;

  ImVec2 max_bound = {min_bound.x + window_size.x, min_bound.y + window_size.y};
  viewport_bounds_[0] = {min_bound.x, min_bound.y};
  viewport_bounds_[1] = {max_bound.x, max_bound.y};

  // ----------------------------------------------------------------------------: Gizmos
  Entity selected_entity = scene_hierarachy_panel_.GetSelectedEntity();
  if (selected_entity && gizmo_type != -1) {
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();

    auto viewport_min_region = ImGui::GetWindowContentRegionMin();
    auto viewport_offset = ImGui::GetWindowPos();
    ImVec2 viewport_bounds_min = {viewport_min_region.x + viewport_offset.x,
                                  viewport_min_region.y + viewport_offset.y};
    ImGuizmo::SetRect(viewport_bounds_min.x, viewport_bounds_min.y,
                      viewport_size_.x, viewport_size_.y);

    // Editor camera
    const glm::mat4& camera_projection = editor_camera_.GetProjection();
    glm::mat4 camera_view = editor_camera_.GetViewMatrx();

    // Runtime camera from entity
    // auto camera_entity = active_scene_->GetPrimaryCameraEntity();
    // const auto& camera = camera_entity.GetComponent<CameraComponent>().camera;
    // const glm::mat4& camera_projection = camera.GetProjection();
    // glm::mat4 camera_view =
    //     glm::inverse(camera_entity.GetComponent<TransformComponent>().GetTransform());

    // Entity transform
    auto& tc = selected_entity.GetComponent<TransformComponent>();
    glm::mat4 transform = tc.GetTransform();

    // Snapping
    bool snap = Input::IsKeyPressed(Key::LeftControl);
    float snap_value = 0.5f;  // Snap to 0.5m for translation/scale
    // Snap to 45 degrees for rotation
    if (gizmo_type == ImGuizmo::OPERATION::ROTATE) {
      snap_value = 45.0f;
    }

    float snap_values[3] = {snap_value, snap_value, snap_value};

    ImGuizmo::Manipulate(glm::value_ptr(camera_view), glm::value_ptr(camera_projection),
                         (ImGuizmo::OPERATION)gizmo_type, ImGuizmo::LOCAL,
                         glm::value_ptr(transform), nullptr, snap ? snap_values : nullptr);

    if (ImGuizmo::IsUsing()) {
      glm::vec3 position, rotation, scale;
      if (math::DecomposeTransform(transform, position, rotation, scale)) {
        glm::vec3 deltaRotation = rotation - tc.rotation;
        tc.position = position;
        tc.rotation += deltaRotation;
        tc.scale = scale;
      }
    }
  }

  ImGui::End();  // viewport
  ImGui::PopStyleVar();

  ImGui::End();  // dockspace demo
}

void EditorLayer::OnEvent(Event& event) {
  camera_controller_.OnEvent(event);
  editor_camera_.OnEvent(event);

  EventDispatcher dispatcher(event);
  dispatcher.DispatchEvent<KeyPressedEvent>(CK_BIND_EVENT(EditorLayer::OnKeyPressed));
}

bool EditorLayer::OnKeyPressed(KeyPressedEvent& e) {
  // Shortcuts
  if (e.GetRepeatCount() > 0) {
    return false;
  }

  bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
  bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
  switch (static_cast<Key>(e.GetKeyCode())) {
    case Key::N: {
      if (control) {
        NewScene();
      }
      break;
    }
    case Key::O: {
      if (control) {
        OpenScene();
      }
      break;
    }
    case Key::S: {
      if (control && shift) {
        SaveSceneAs();
      }
      break;
    }
    // Gizmos
    case Key::Q:
      if (!ImGuizmo::IsUsing()) gizmo_type = -1;
      break;
    case Key::W:
      if (!ImGuizmo::IsUsing()) gizmo_type = ImGuizmo::OPERATION::TRANSLATE;
      break;
    case Key::E:
      if (!ImGuizmo::IsUsing()) gizmo_type = ImGuizmo::OPERATION::ROTATE;
      break;
    case Key::R:
      if (!ImGuizmo::IsUsing()) gizmo_type = ImGuizmo::OPERATION::SCALE;
      break;
    default:
      break;
  }

  return true;
}

void EditorLayer::NewScene() {
  active_scene_ = CreateRef<Scene>();
  active_scene_->OnViewportResize((uint32_t)viewport_size_.x, (uint32_t)viewport_size_.y);
  scene_hierarachy_panel_.SetContext(active_scene_);
}

void EditorLayer::OpenScene() {
  std::optional<std::string> filepath =
      FileDialogs::OpenFile("SeedEngine Scene (*.scene)\0*.scene\0");
  if (filepath) {
    active_scene_ = CreateRef<Scene>();
    active_scene_->OnViewportResize((uint32_t)viewport_size_.x, (uint32_t)viewport_size_.y);
    scene_hierarachy_panel_.SetContext(active_scene_);

    SceneSerializer serializer(active_scene_);
    serializer.Deserialize(*filepath);
  }
}

void EditorLayer::SaveSceneAs() {
  std::optional<std::string> filepath =
      FileDialogs::SaveFile("SeedEngine Scene (*.scene)\0*.scene\0");
  if (filepath.has_value()) {
    SceneSerializer serializer(active_scene_);
    serializer.Serialize(filepath.value());
  }
}
}  // namespace ck
