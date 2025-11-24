#include "renderer_2d.h"

#include <memory>

#include "core/core.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float4.hpp"
#include "renderer/buffer.h"
#include "renderer/render_command.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "renderer/vertex_array.h"

namespace ck {

/*─────────────────────────────────────┐
│               Prepare                │
└──────────────────────────────────────*/

struct Renderer2DStorage {
  Scope<VertexArray> quad_vertex_array;
  Scope<Shader> textuer_shader;
  Scope<Texture> white_texture;
};

static Scope<Renderer2DStorage> s_data;

/*─────────────────────────────────────┐
│                Start                 │
└──────────────────────────────────────*/

void Renderer2D::Init() {
  CK_PROFILE_FUNCTION();
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

  s_data->white_texture = Texture2D::Create(1, 1);
  uint32_t white_texture_data = 0xffffffff;
  s_data->white_texture->SetData(&white_texture_data, sizeof(white_texture_data));
  s_data->textuer_shader = Shader::Create("assets/shaders/texture.glsl");
  s_data->textuer_shader->SetInt("t_texture", 0);
}

void Renderer2D::Shutdown() {
  CK_PROFILE_FUNCTION();
  s_data.reset();
}

void Renderer2D::BeginScene(const OrthographicCamera& camera) {
  CK_PROFILE_FUNCTION();
  s_data->textuer_shader->Bind();
  s_data->textuer_shader->SetMat4("u_view_projection", camera.GetViewProjectionMatrix());
}

void Renderer2D::EndScene() { CK_PROFILE_FUNCTION(); }

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size,
                          const glm::vec4& color) {
  DrawQuad({position.x, position.y, 0.0f}, size, color);
}

void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size,
                          const glm::vec4& color) {
  CK_PROFILE_FUNCTION();
  auto& texture_shader = s_data->textuer_shader;
  texture_shader->Bind();
  texture_shader->SetFloat4("u_color", color);

  auto transform = glm::translate(glm::mat4(1.0f), position) *
                   glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
  texture_shader->SetMat4("u_transform", transform);

  s_data->white_texture->Bind();
  s_data->quad_vertex_array->Bind();
  RenderCommand::DrawIndexed(s_data->quad_vertex_array.get());
}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size,
                          const Ref<Texture2D>& texture, float tiling_factor,
                          const glm::vec4& tint_color) {
  DrawQuad({position.x, position.y, 1.0f}, size, texture, tiling_factor, tint_color);
}

void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size,
                          const Ref<Texture2D>& texture, float tiling_factor,
                          const glm::vec4& tint_color) {
  CK_PROFILE_FUNCTION();
  s_data->textuer_shader->Bind();
  s_data->textuer_shader->SetFloat("u_tiling_factor", tiling_factor);
  s_data->textuer_shader->SetFloat4("u_color", tint_color);

  auto transform = glm::translate(glm::mat4(1.0f), position) *
                   glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
  s_data->textuer_shader->SetMat4("u_transform", transform);

  texture->Bind();
  s_data->quad_vertex_array->Bind();
  RenderCommand::DrawIndexed(s_data->quad_vertex_array.get());
}

void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation,
                                 const glm::vec4& color) {
  DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, color);
}

void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation,
                                 const glm::vec4& color) {
  CK_PROFILE_FUNCTION();
  s_data->textuer_shader->SetFloat("u_tiling_factor", 1.0f);

  s_data->textuer_shader->SetFloat4("u_color", color);
  glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                        glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f}) *
                        glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
  s_data->textuer_shader->SetMat4("u_transform", transform);

  s_data->white_texture->Bind();
  s_data->quad_vertex_array->Bind();
  RenderCommand::DrawIndexed(s_data->quad_vertex_array.get());
}

void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation,
                                 const Ref<Texture2D>& texture, float tiling_factor,
                                 const glm::vec4& tint_color) {
  DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, texture, tiling_factor,
                  tint_color);
}

void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation,
                                 const Ref<Texture2D>& texture, float tiling_factor,
                                 const glm::vec4& tint_color) {
  CK_PROFILE_FUNCTION();

  s_data->textuer_shader->SetFloat4("u_color", tint_color);
  s_data->textuer_shader->SetFloat("u_tiling_factor", tiling_factor);
  glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                        glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f}) *
                        glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
  s_data->textuer_shader->SetMat4("u_transform", transform);

  texture->Bind();
  s_data->quad_vertex_array->Bind();
  RenderCommand::DrawIndexed(s_data->quad_vertex_array.get());
}
}  // namespace ck
