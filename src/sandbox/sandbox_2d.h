#pragma once

#include <vector>

#include "ck.h"
#include "core/core.h"
#include "renderer/texture.h"

class Sandbox2D : public ck::Layer {
public:
  Sandbox2D();
  ~Sandbox2D() = default;

  void OnAttach() override;
  void OnDetach() override;
  void OnUpdate(ck::DeltaTime dt) override;
  void OnImGuiRender() override;
  void OnEvent(ck::Event& event) override;

private:
  ck::OrthographicCameraController camera_controller_;

  ck::Ref<ck::VertexArray> square_vertex_array_;
  ck::Ref<ck::VertexBuffer> square_vertex_buffer_;
  ck::Ref<ck::IndexBuffer> square_index_buffer_;
  ck::Ref<ck::Shader> flat_color_shader_;

  ck::Ref<ck::Texture2D> checkboard_texture_;

  glm::vec4 color_1_{0.8f, 0.2f, 0.3f, 1.0f};
  glm::vec4 color_2_{0.2f, 0.3f, 0.8f, 1.0f};
  glm::vec4 background_color_{0.25f, 0.2f, 0.2f, 1.0f};

  // Profiling
  struct ProfileResult {
    const char* name;
    float time;
  };

  std::vector<ProfileResult> profile_results_;
};
