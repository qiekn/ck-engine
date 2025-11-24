#pragma once

#include "core/layer.h"
#include "renderer/orthographic_camera_controller.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "renderer/vertex_array.h"

class ExampleLayer : public ck::Layer {
public:
  ExampleLayer();
  virtual ~ExampleLayer() = default;

  virtual void OnAttach() override;
  virtual void OnDetach() override;

  void OnUpdate(ck::DeltaTime dt) override;
  void OnImGuiRender() override;
  void OnEvent(ck::Event& event) override;

private:
  ck::ShaderLibrary shader_library_;

  ck::Ref<ck::VertexArray> vertex_array_;
  ck::Ref<ck::Shader> shader_;

  ck::Ref<ck::VertexArray> square_va_;
  ck::Ref<ck::Shader> flat_color_shader_;

  ck::OrthographicCameraController camera_controller_;

  ck::Ref<ck::Texture2D> cat_texture_;
  ck::Ref<ck::Texture2D> apple_texture_;
};
