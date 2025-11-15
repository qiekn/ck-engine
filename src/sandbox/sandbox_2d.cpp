#include "sandbox_2d.h"

#include <memory>

#include "core/core.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include "platform/opengl/opengl_shader.h"
#include "renderer/buffer.h"
#include "renderer/orthographic_camera_controller.h"
#include "renderer/render_command.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "renderer/vertex_array.h"

Sandbox2D::Sandbox2D() : Layer("Sandbox 2D"), camera_controller_(16.0f / 9.0f) {}

void Sandbox2D::OnAttach() {
  // Square
  square_vertex_array_ = ck::VertexArray::Create();

  // clang-format off
  float square_vertices[] = {
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
     0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
     0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
    -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
  };
  // clang-format on

  ck::Ref<ck::VertexBuffer> square_vertex_buffer =
      ck::VertexBuffer::Create(square_vertices, sizeof(square_vertices));
  square_vertex_buffer->SetLayout(
      {{ck::ShaderDataType::kFloat3, "a_position"}, {ck::ShaderDataType::kFloat2, "a_tex_coord"}});
  square_vertex_array_->AddVertexBuffer(square_vertex_buffer);

  uint32_t square_indices[] = {0, 1, 2, 3, 0};
  square_index_buffer_ =
      ck::IndexBuffer::Create(square_indices, sizeof(square_indices) / sizeof(uint32_t));
  square_vertex_array_->SetIndexBuffer(square_index_buffer_);

  flat_color_shader_ = ck::Shader::Create("assets/shaders/flat_color.glsl");
}

void Sandbox2D::OnDetach() {}

void Sandbox2D::OnUpdate(ck::DeltaTime dt) {
  camera_controller_.OnUpdate(dt);

  ck::RenderCommand::SetClearColor({0.25f, 0.2f, 0.2f, 1.0f});
  ck::RenderCommand::Clear();

  ck::Renderer::BeginScene(camera_controller_.Camera());

  auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

  flat_color_shader_->Bind();
  std::dynamic_pointer_cast<ck::OpenGLShader>(flat_color_shader_)
      ->UploadUniformFlaot4("u_color", square_color_);
  ck::Renderer::Submit(flat_color_shader_.get(), square_vertex_array_.get());

  ck::Renderer::EndScene();
}

void Sandbox2D::OnImGuiRender() {
  ImGui::Begin("Settings");
  ImGui::ColorEdit4("Square Color", glm::value_ptr(square_color_));
  ImGui::End();
}

void Sandbox2D::OnEvent(ck::Event& event) { camera_controller_.OnEvent(event); }
