import ck;

#include "panels/stats_panel.h"
#include "panels/viewport_panel.h"

namespace ck_editor {

// Single textured quad — the editor's smoke test for the Renderer2D path.
class EditorLayer : public ck::Layer {
public:
  EditorLayer() : Layer("EditorLayer") {}

  void OnAttach() override {
    checkerboard_ = ck::Renderer2D::LoadTexture("assets/textures/checkerboard.png");
  }

  void OnUpdate(ck::DeltaTime) override {
    ck::Renderer2D::DrawQuad(glm::mat4(1.0f), checkerboard_);
  }

  void OnImGuiRender() override {
    // Full-window dockspace: panels can dock into edges of the main viewport.
    // Explicit args sidestep the cross-module default-argument risk called
    // out in phase-6-plan.md.
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), 0, nullptr);

    viewport_panel_.OnImGuiRender();
    stats_panel_.OnImGuiRender();
  }

private:
  ck::Renderer2D::TextureHandle checkerboard_ = ck::Renderer2D::kWhiteTexture;
  ViewportPanel viewport_panel_;
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