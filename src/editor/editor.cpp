import ck;

#include "editor_camera.h"
#include "panels/properties_panel.h"
#include "panels/scene_hierarchy_panel.h"
#include "panels/stats_panel.h"
#include "panels/viewport_panel.h"

namespace ck_editor {

// Single textured quad — driven through Scene + (Transform, SpriteRenderer)
// instead of a direct Renderer2D::DrawQuad call as of phase 6.B.1.
class EditorLayer : public ck::Layer {
public:
  EditorLayer() : Layer("EditorLayer") {}

  void OnAttach() override {
    scene_ = ck::CreateRef<ck::Scene>();
    auto entity = scene_->CreateEntity("Checkerboard");
    entity.AddComponent<ck::SpriteRendererComponent>(
        ck::Renderer2D::LoadTexture("assets/textures/checkerboard.png"));

    hierarchy_panel_.SetContext(scene_);
  }

  void OnUpdate(ck::DeltaTime ts) override {
    editor_camera_.OnUpdate(ts, viewport_panel_.IsHovered());
    editor_camera_.PushTo(ck::Application::Get().GetRenderer().GetCamera());
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

    viewport_panel_.OnImGuiRender();
    hierarchy_panel_.OnImGuiRender();
    properties_panel_.OnImGuiRender(hierarchy_panel_.SelectedEntity());
    stats_panel_.OnImGuiRender();
  }

private:
  ck::Ref<ck::Scene> scene_;
  EditorCamera editor_camera_;
  ViewportPanel viewport_panel_;
  SceneHierarchyPanel hierarchy_panel_;
  PropertiesPanel properties_panel_;
  StatsPanel stats_panel_;
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