import ck;

#include "editor_camera_3d.h"
#include "panels/properties_panel.h"
#include "panels/scene_hierarchy_panel.h"
#include "panels/stats_panel.h"
#include "panels/viewport_panel.h"

namespace ck_editor {

// Phase 6.C: editor camera switched to 3D arcball; renderer pass now
// composites a depth-tested 3D mesh (procedural cube) under the
// Scene-driven Renderer2D quads.
class EditorLayer : public ck::Layer {
public:
  EditorLayer() : Layer("EditorLayer") {}

  void OnAttach() override {
    auto& renderer = ck::Application::Get().GetRenderer();

    scene_ = ck::CreateRef<ck::Scene>();
    auto entity = scene_->CreateEntity("Checkerboard");
    auto& sr = entity.AddComponent<ck::SpriteRendererComponent>();
    sr.texture_path = "assets/textures/checkerboard.png";
    sr.filter  = ck::Renderer2D::Filter::Nearest;
    sr.texture = ck::Renderer2D::LoadTexture(sr.texture_path, sr.filter);

    hierarchy_panel_.SetContext(scene_);

    cube_ = ck::Mesh::CreateCube(renderer.allocator());
    cube_transform_ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.6f));
  }

  void OnUpdate(ck::DeltaTime ts) override {
    auto& renderer = ck::Application::Get().GetRenderer();
    auto extent = renderer.color_target_extent();
    if (extent.width > 0 && extent.height > 0) {
      editor_camera_.SetViewport(extent.width, extent.height);
    }
    editor_camera_.OnUpdate(ts, viewport_panel_.IsHovered());
    renderer.SetActiveCamera(editor_camera_.view_projection());

    // Demo 3D draw alongside the 2D Scene path.
    ck::Renderer3D::DrawMesh(cube_, cube_transform_, glm::vec3(0.85f, 0.45f, 0.25f));

    scene_->OnUpdate(ts);
  }

  void OnEvent(ck::Event& e) override {
    editor_camera_.OnEvent(e, viewport_panel_.IsHovered());
  }

  void OnImGuiRender() override {
    // Full-window dockspace: panels can dock into edges of the main viewport.
    // Explicit args sidestep the cross-module default-argument risk called
    // out in phase-6-plan.md.
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), 0, nullptr);

    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Save Scene", nullptr, false, true)) {
          ck::SceneSerializer(scene_).Serialize(kScenePath);
        }
        if (ImGui::MenuItem("Load Scene", nullptr, false, true)) {
          ck::SceneSerializer(scene_).Deserialize(kScenePath);
          hierarchy_panel_.SetSelectedEntity({});
        }
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }

    viewport_panel_.OnImGuiRender();
    hierarchy_panel_.OnImGuiRender();
    properties_panel_.OnImGuiRender(hierarchy_panel_.SelectedEntity());
    stats_panel_.OnImGuiRender();
  }

private:
  static constexpr const char* kScenePath = "assets/scenes/default.ckscene";

  ck::Ref<ck::Scene> scene_;
  EditorCamera3D editor_camera_;
  ViewportPanel viewport_panel_;
  SceneHierarchyPanel hierarchy_panel_;
  PropertiesPanel properties_panel_;
  StatsPanel stats_panel_;

  ck::Ref<ck::Mesh> cube_;
  glm::mat4 cube_transform_{1.0f};
};

class Editor : public ck::Application {
public:
  explicit Editor(const ck::ApplicationSpecification& spec) : Application(spec) {
    PushLayer(ck::CreateScope<EditorLayer>());
  }
  ~Editor() {}
};

}  // namespace ck_editor

ck::Application* ck::CreateApplication(ck::ApplicationCommandLineArgs args) {
  ck::ApplicationSpecification spec;
  spec.name = "CK Editor";
  spec.command_line_args = args;
  return new ck_editor::Editor(spec);
}

int main(int argc, char** argv) { return ck::EntryPoint(argc, argv); }