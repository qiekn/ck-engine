#include "renderer_2d.h"

#include <cstdint>
#include <memory>

#include "core/core.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
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

struct QuadVertex {
  glm::vec3 position;
  glm::vec4 color;
  glm::vec2 tex_coord;
  // TODO: texid;
};

struct Renderer2DData {
  const uint32_t kMaxQuads = 10000;
  const uint32_t kMaxVertices = kMaxQuads * 4;
  const uint32_t kMaxIndices = kMaxQuads * 6;

  Ref<VertexArray> quad_vertex_array;
  Ref<VertexBuffer> quad_vertex_buffer;
  Ref<Shader> textuer_shader;
  Ref<Texture> white_texture;

  uint32_t quad_index_count = 0;
  QuadVertex* quad_vertex_buffer_base = nullptr;
  QuadVertex* quad_vertex_buffer_ptr = nullptr;
};

static Renderer2DData s_data;

/*─────────────────────────────────────┐
│                Start                 │
└──────────────────────────────────────*/

void Renderer2D::Init() {
  CK_PROFILE_FUNCTION();
  s_data.quad_vertex_array = VertexArray::Create();

  // Vertex Buffer
  s_data.quad_vertex_buffer = VertexBuffer::Create(s_data.kMaxVertices * sizeof(QuadVertex));
  s_data.quad_vertex_buffer->SetLayout({{ShaderDataType::kFloat3, "a_position"},
                                        {ShaderDataType::kFloat4, "a_color"},
                                        {ShaderDataType::kFloat2, "a_tex_coord"}});
  s_data.quad_vertex_array->AddVertexBuffer(s_data.quad_vertex_buffer);

  s_data.quad_vertex_buffer_base = new QuadVertex[s_data.kMaxVertices];

  // Index Buffer
  uint32_t* quad_indices = new uint32_t[s_data.kMaxIndices];
  uint32_t offset = 0;

  for (uint32_t i = 0; i < s_data.kMaxIndices; i += 6) {
    quad_indices[i + 0] = offset + 0;
    quad_indices[i + 1] = offset + 1;
    quad_indices[i + 2] = offset + 2;

    quad_indices[i + 3] = offset + 2;
    quad_indices[i + 4] = offset + 3;
    quad_indices[i + 5] = offset + 0;

    offset += 4;
  }

  Ref<IndexBuffer> quad_index_buffer = IndexBuffer::Create(quad_indices, s_data.kMaxIndices);
  s_data.quad_vertex_array->SetIndexBuffer(quad_index_buffer);
  delete[] quad_indices;

  s_data.white_texture = Texture2D::Create(1, 1);
  uint32_t white_texture_data = 0xffffffff;
  s_data.white_texture->SetData(&white_texture_data, sizeof(white_texture_data));

  s_data.textuer_shader = Shader::Create("assets/shaders/texture.glsl");
  s_data.textuer_shader->SetInt("t_texture", 0);
}

void Renderer2D::Shutdown() { CK_PROFILE_FUNCTION(); }

void Renderer2D::BeginScene(const OrthographicCamera& camera) {
  CK_PROFILE_FUNCTION();
  s_data.textuer_shader->Bind();
  s_data.textuer_shader->SetMat4("u_view_projection", camera.GetViewProjectionMatrix());

  s_data.quad_index_count = 0;
  s_data.quad_vertex_buffer_ptr = s_data.quad_vertex_buffer_base;
}

void Renderer2D::EndScene() {
  CK_PROFILE_FUNCTION();
  uint32_t data_size =
      (uint8_t*)s_data.quad_vertex_buffer_ptr - (uint8_t*)s_data.quad_vertex_buffer_base;
  s_data.quad_vertex_buffer->SetData(s_data.quad_vertex_buffer_base, data_size);

  Flush();
}

void Renderer2D::Flush() {
  RenderCommand::DrawIndexed(s_data.quad_vertex_array.get(), s_data.quad_index_count);
}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size,
                          const glm::vec4& color) {
  DrawQuad({position.x, position.y, 0.0f}, size, color);
}

void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size,
                          const glm::vec4& color) {
  CK_PROFILE_FUNCTION();

  s_data.quad_vertex_buffer_ptr->position = position;
  s_data.quad_vertex_buffer_ptr->color = color;
  s_data.quad_vertex_buffer_ptr->tex_coord = {0.0f, 0.0f};
  s_data.quad_vertex_buffer_ptr++;

  s_data.quad_vertex_buffer_ptr->position = {position.x + size.x, position.y, 0.0f};
  s_data.quad_vertex_buffer_ptr->color = color;
  s_data.quad_vertex_buffer_ptr->tex_coord = {1.0f, 0.0f};
  s_data.quad_vertex_buffer_ptr++;

  s_data.quad_vertex_buffer_ptr->position = {position.x + size.x, position.y + size.y, 0.0f};
  s_data.quad_vertex_buffer_ptr->color = color;
  s_data.quad_vertex_buffer_ptr->tex_coord = {1.0f, 1.0f};
  s_data.quad_vertex_buffer_ptr++;

  s_data.quad_vertex_buffer_ptr->position = {position.x, position.y + size.y, 0.0f};
  s_data.quad_vertex_buffer_ptr->color = color;
  s_data.quad_vertex_buffer_ptr->tex_coord = {0.0f, 1.0f};
  s_data.quad_vertex_buffer_ptr++;

  s_data.quad_index_count += 6;

  /*
  auto& texture_shader = s_data.textuer_shader;
  texture_shader->Bind();
  texture_shader->SetFloat4("u_color", color);

  auto transform = glm::translate(glm::mat4(1.0f), position) *
                   glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
  texture_shader->SetMat4("u_transform", transform);

  s_data.white_texture->Bind();
  s_data.quad_vertex_array->Bind();
  RenderCommand::DrawIndexed(s_data.quad_vertex_array.get());
  */
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

  s_data.textuer_shader->SetFloat4("u_color", tint_color);
  s_data.textuer_shader->SetFloat("u_tiling_factor", 1.0f);

  auto transform = glm::translate(glm::mat4(1.0f), position) *
                   glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
  s_data.textuer_shader->SetMat4("u_transform", transform);

  texture->Bind();
  s_data.quad_vertex_array->Bind();
  RenderCommand::DrawIndexed(s_data.quad_vertex_array.get());
}

void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation,
                                 const glm::vec4& color) {
  DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, color);
}

void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation,
                                 const glm::vec4& color) {
  CK_PROFILE_FUNCTION();

  s_data.textuer_shader->SetFloat4("u_color", color);
  s_data.textuer_shader->SetFloat("u_tiling_factor", 1.0f);
  glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                        glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f}) *
                        glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
  s_data.textuer_shader->SetMat4("u_transform", transform);

  s_data.white_texture->Bind();
  s_data.quad_vertex_array->Bind();
  RenderCommand::DrawIndexed(s_data.quad_vertex_array.get());
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

  s_data.textuer_shader->SetFloat4("u_color", tint_color);
  s_data.textuer_shader->SetFloat("u_tiling_factor", tiling_factor);
  glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                        glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f}) *
                        glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
  s_data.textuer_shader->SetMat4("u_transform", transform);

  texture->Bind();
  s_data.quad_vertex_array->Bind();
  RenderCommand::DrawIndexed(s_data.quad_vertex_array.get());
}
}  // namespace ck
