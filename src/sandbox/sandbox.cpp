#include <ck.h>

#include <array>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "core/entry_point.h"  // IWYU pragma: keep

namespace {

// 10x10 grid of textured quads, alternating between three textures.
// Demonstrates Renderer2D batching: one draw call flushes the whole grid.
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

    if (++log_counter_ % 120 == 0) {
      auto s = ck::Renderer2D::stats();
      ck::log::info("Renderer2D flushed: {} quads, {} textures, {} draw call(s)",
                     s.quad_count, s.texture_count, s.draw_calls);
    }
  }

private:
  std::array<ck::Renderer2D::TextureHandle, 3> textures_{};
  uint32_t log_counter_ = 0;
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