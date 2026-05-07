#include <ck.h>

#include <glm/gtc/matrix_transform.hpp>

#include "core/entry_point.h"  // IWYU pragma: keep

namespace ck {

// Single textured quad — the editor's smoke test for the Renderer2D path.
// Loads the checkerboard once on attach, draws one quad each frame.
class EditorLayer : public Layer {
public:
  EditorLayer() : Layer("EditorLayer") {}

  void OnAttach() override {
    checkerboard_ = Renderer2D::LoadTexture("assets/textures/checkerboard.png");
  }

  void OnUpdate(DeltaTime) override {
    Renderer2D::DrawQuad(glm::mat4(1.0f), checkerboard_);
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