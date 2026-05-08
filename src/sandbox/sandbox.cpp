import ck;

namespace {

// 10x10 grid of textured quads, alternating between three textures.
// One draw call flushes the whole grid via Renderer2D batching.
class GridLayer : public ck::Layer {
public:
  GridLayer() : Layer("GridLayer") {}

  void OnAttach() override {
    textures_[0] = ck::Renderer2D::LoadTexture("assets/textures/checkerboard.png");
    textures_[1] = ck::Renderer2D::LoadTexture("assets/textures/cat.jpg");
    textures_[2] = ck::Renderer2D::LoadTexture("assets/textures/apple.png");
  }

  void OnUpdate(ck::DeltaTime) override {
    constexpr int kSide = 10;
    constexpr float kSpacing = 0.18f;
    constexpr float kSize = 0.16f;
    for (int y = 0; y < kSide; ++y) {
      for (int x = 0; x < kSide; ++x) {
        float fx = (x - kSide / 2.0f + 0.5f) * kSpacing;
        float fy = (y - kSide / 2.0f + 0.5f) * kSpacing;
        glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(fx, fy, 0.0f)) *
                      glm::scale(glm::mat4(1.0f), glm::vec3(kSize, kSize, 1.0f));
        ck::Renderer2D::DrawQuad(t, textures_[(x + y) % 3]);
      }
    }
  }

  // Doubles as the import-ck → ImGui::* re-export validation per phase 6.A.2.
  // Now also responsible for showing the Renderer2D output: with the
  // 6.A.3 flow the engine no longer blits color_target straight to the
  // swapchain, so a panel hosting the texture is the only way to see it.
  void OnImGuiRender() override {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Sandbox Viewport");
    ImVec2 size = ImGui::GetContentRegionAvail();
    if (size.x > 0.0f && size.y > 0.0f) {
      ck::Application::Get().OnViewportResize(static_cast<uint32_t>(size.x),
                                              static_cast<uint32_t>(size.y));
    }
    ImTextureID tex = ck::Application::Get().GetImGuiLayer().viewport_texture_id();
    if (tex) ImGui::Image(tex, size);
    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::Begin("Sandbox Stats");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    auto s = ck::Renderer2D::stats();
    ImGui::Text("Quads: %u", s.quad_count);
    ImGui::Text("Textures: %u", s.texture_count);
    ImGui::Text("Draw calls: %u", s.draw_calls);
    ImGui::End();
  }

private:
  ck::Renderer2D::TextureHandle textures_[3]{};
};

}  // namespace

class Sandbox : public ck::Application {
public:
  explicit Sandbox(const ck::ApplicationSpecification& spec) : Application(spec) {
    PushLayer(ck::CreateScope<GridLayer>());
  }
  ~Sandbox() {}
};

ck::Application* ck::CreateApplication(ck::ApplicationCommandLineArgs args) {
  ck::ApplicationSpecification spec;
  spec.name = "Sandbox";
  spec.command_line_args = args;
  return new Sandbox(spec);
}

int main(int argc, char** argv) { return ck::EntryPoint(argc, argv); }