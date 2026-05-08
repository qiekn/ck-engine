import ck;

namespace ck {

// Single textured quad — the editor's smoke test for the Renderer2D path.
class EditorLayer : public Layer {
public:
  EditorLayer() : Layer("EditorLayer") {}

  void OnAttach() override {
    checkerboard_ = Renderer2D::LoadTexture("assets/textures/checkerboard.png");
  }

  void OnUpdate(DeltaTime) override {
    Renderer2D::DrawQuad(glm::mat4(1.0f), checkerboard_);
  }

  void OnImGuiRender() override {
    // Full-window dockspace: panels can dock into edges of the main viewport.
    // Explicit args sidestep the cross-module default-argument risk called
    // out in phase-6-plan.md.
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), 0, nullptr);

    // Viewport panel — samples the engine's offscreen color_target as
    // ImTextureID. WindowPadding=0 so the Image fills the panel content
    // area edge-to-edge (default 8px padding * dpi shows the panel BG
    // through as a "black border"). Image stretches to whatever size the
    // panel currently is; panel-driven camera resize is the next sub-step
    // (6.A.3.5).
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Viewport");
    ImTextureID tex = Application::Get().GetImGuiLayer().viewport_texture_id();
    if (tex) ImGui::Image(tex, ImGui::GetContentRegionAvail());
    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::Begin("Stats");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    auto s = Renderer2D::stats();
    ImGui::Text("Quads: %u", s.quad_count);
    ImGui::Text("Textures: %u", s.texture_count);
    ImGui::Text("Draw calls: %u", s.draw_calls);
    ImGui::End();
  }

private:
  Renderer2D::TextureHandle checkerboard_ = Renderer2D::kWhiteTexture;
};

class Editor : public Application {
public:
  explicit Editor(const ApplicationSpecification& spec) : Application(spec) {
    PushLayer(CreateScope<EditorLayer>());
  }
  ~Editor() {}
};

}  // namespace ck

ck::Application* ck::CreateApplication(ck::ApplicationCommandLineArgs args) {
  ck::ApplicationSpecification spec;
  spec.name = "CK Editor";
  spec.command_line_args = args;
  return new ck::Editor(spec);
}

int main(int argc, char** argv) { return ck::EntryPoint(argc, argv); }