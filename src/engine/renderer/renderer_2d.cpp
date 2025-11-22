#include "renderer_2d.h"

#include <algorithm>
#include <memory>

#include "core/core.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "platform/opengl/opengl_shader.h"
#include "renderer/buffer.h"
#include "renderer/render_command.h"
#include "renderer/shader.h"
#include "renderer/vertex_array.h"

namespace ck {

/*─────────────────────────────────────┐
│               Prepare                │
└──────────────────────────────────────*/

struct Renderer2DStorage {
  Scope<VertexArray> quad_vertex_array;
  Scope<Shader> flat_color_shader;
  Scope<Shader> textuer_shader;
};

static Scope<Renderer2DStorage> s_data;

/*─────────────────────────────────────┐
│                Start                 │
└──────────────────────────────────────*/

void Renderer2D::Init() {
  s_data = std::make_unique<Renderer2DStorage>();
  s_data->quad_vertex_array = VertexArray::Create();

  // clang-format off
  float square_vertices[] = {
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
     0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
     0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
    -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
  };
  // clang-format on

  // Vertex Buffer
  Ref<VertexBuffer> square_vertex_buffer =
      VertexBuffer::Create(square_vertices, sizeof(square_vertices));
  square_vertex_buffer->SetLayout(
      {{ShaderDataType::kFloat3, "a_position"}, {ShaderDataType::kFloat2, "a_tex_coord"}});
  s_data->quad_vertex_array->AddVertexBuffer(square_vertex_buffer);

  // Index Buffer
  uint32_t square_indices[] = {0, 1, 2, 2, 3, 0};
  Ref<IndexBuffer> square_index_buffer_ =
      IndexBuffer::Create(square_indices, sizeof(square_indices) / sizeof(uint32_t));
  s_data->quad_vertex_array->SetIndexBuffer(square_index_buffer_);

  s_data->flat_color_shader = Shader::Create("assets/shaders/flat_color.glsl");
  s_data->textuer_shader = Shader::Create("assets/shaders/texture.glsl");
  s_data->textuer_shader->SetInt("t_texture", 0);
}

void Renderer2D::Shutdown() { s_data.reset(); }

void Renderer2D::BeginScene(const OrthographicCamera& camera) {
  s_data->flat_color_shader->Bind();
  s_data->flat_color_shader->SetMat4("u_view_projection", camera.GetViewProjectionMatrix());

  s_data->textuer_shader->Bind();
  s_data->textuer_shader->SetMat4("u_view_projection", camera.GetViewProjectionMatrix());
}

void Renderer2D::EndScene() {}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size,
                          const glm::vec4& color) {
  DrawQuad({position.x, position.y, 0.0f}, size, color);
}

void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size,
                          const glm::vec4& color) {
  auto& shader = s_data->flat_color_shader;
  shader->Bind();
  shader->SetFloat4("u_color", color);

  auto transform = glm::translate(glm::mat4(1.0f), position) *
                   glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
  shader->SetMat4("u_transform", transform);

  s_data->quad_vertex_array->Bind();
  RenderCommand::DrawIndexed(s_data->quad_vertex_array.get());
}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size,
                          const Ref<Texture2D>& texture) {
  DrawQuad({position.x, position.y, 1.0f}, size, texture);
}

void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size,
                          const Ref<Texture2D>& texture) {
  s_data->textuer_shader->Bind();
  auto transform = glm::translate(glm::mat4(1.0f), position) *
                   glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
  s_data->textuer_shader->SetMat4("u_transform", transform);
  texture->Bind();
  s_data->quad_vertex_array->Bind();
  RenderCommand::DrawIndexed(s_data->quad_vertex_array.get());
}

}  // namespace ck
