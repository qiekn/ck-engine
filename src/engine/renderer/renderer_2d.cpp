#include "renderer_2d.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <memory>

#include "core/core.h"
#include "debug/profiler.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/trigonometric.hpp"
#include "renderer/buffer.h"
#include "renderer/render_command.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "renderer/uniform_buffer.h"
#include "renderer/vertex_array.h"

namespace ck {

// ----------------------------------------------------------------------------: Prepare

struct QuadVertex {
  glm::vec3 position;
  glm::vec4 color;
  glm::vec2 tex_coord;
  float tex_index;
  float tiling_factor;

  // Editor-only
  int entity_id;
};

struct Renderer2DData {
  static const uint32_t kMaxQuads = 20000;
  static const uint32_t kMaxVertices = kMaxQuads * 4;
  static const uint32_t kMaxIndices = kMaxQuads * 6;
  static const uint32_t kMaxTextureSlots = 32;  // TOOD: RenderCaps

  Ref<VertexArray> quad_vertex_array;
  Ref<VertexBuffer> quad_vertex_buffer;
  Ref<Shader> texture_shader;
  Ref<Texture2D> white_texture;

  uint32_t quad_index_count = 0;
  QuadVertex* quad_vertex_buffer_base = nullptr;
  QuadVertex* quad_vertex_buffer_ptr = nullptr;

  std::array<Ref<Texture2D>, kMaxTextureSlots> texture_slots;
  uint32_t texture_slot_index = 1;  // 0 = white texture

  glm::vec4 quad_vertex_position[4];

  Renderer2D::Statistics stats;

  struct CameraData {
    glm::mat4 view_projection;
  };
  CameraData camera_buffer;
  Ref<UniformBuffer> camera_uniform_buffer;
};

static Renderer2DData s_data;

// ----------------------------------------------------------------------------: Start

void Renderer2D::Init() {
  CK_PROFILE_FUNCTION();
  s_data.quad_vertex_array = VertexArray::Create();

  // Vertex Buffer
  s_data.quad_vertex_buffer = VertexBuffer::Create(s_data.kMaxVertices * sizeof(QuadVertex));
  s_data.quad_vertex_buffer->SetLayout({
      {ShaderDataType::kFloat3, "a_position"},
      {ShaderDataType::kFloat4, "a_color"},
      {ShaderDataType::kFloat2, "a_tex_coord"},
      {ShaderDataType::kFloat, "a_tex_index"},
      {ShaderDataType::kFloat, "a_tiling_factor"},
      {ShaderDataType::kInt, "a_entity_id"},
  });
  s_data.quad_vertex_array->AddVertexBuffer(s_data.quad_vertex_buffer);
  s_data.quad_vertex_buffer_base = new QuadVertex[s_data.kMaxVertices];

  // Index Buffer
  uint32_t* quad_indices = new uint32_t[s_data.kMaxIndices];
  for (uint32_t i = 0, offset = 0; i < s_data.kMaxIndices; i += 6, offset += 4) {
    quad_indices[i + 0] = offset + 0;
    quad_indices[i + 1] = offset + 1;
    quad_indices[i + 2] = offset + 2;

    quad_indices[i + 3] = offset + 2;
    quad_indices[i + 4] = offset + 3;
    quad_indices[i + 5] = offset + 0;
  }
  Ref<IndexBuffer> quad_index_buffer = IndexBuffer::Create(quad_indices, s_data.kMaxIndices);
  s_data.quad_vertex_array->SetIndexBuffer(quad_index_buffer);
  delete[] quad_indices;

  // White Texture
  s_data.white_texture = Texture2D::Create(1, 1);
  uint32_t white_texture_data = 0xffffffff;
  s_data.white_texture->SetData(&white_texture_data, sizeof(white_texture_data));

  int32_t samplers[s_data.kMaxTextureSlots];
  for (uint32_t i = 0; i < s_data.kMaxTextureSlots; i++) {
    samplers[i] = (int32_t)i;
  }

  s_data.texture_shader = Shader::Create("assets/shaders/texture.glsl");

  // Initialize Texture Slots
  for (uint32_t i = 0; i < s_data.kMaxTextureSlots; i++) {
    s_data.texture_slots[i] = 0;
  }
  s_data.texture_slots[0] = s_data.white_texture;

  s_data.quad_vertex_position[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
  s_data.quad_vertex_position[1] = {0.5f, -0.5f, 0.0f, 1.0f};
  s_data.quad_vertex_position[2] = {0.5f, 0.5f, 0.0f, 1.0f};
  s_data.quad_vertex_position[3] = {-0.5f, 0.5f, 0.0f, 1.0f};

  s_data.camera_uniform_buffer =
      UniformBuffer::Create(sizeof(Renderer2DData::CameraData), 0);
}

void Renderer2D::Shutdown() {
  CK_PROFILE_FUNCTION();
  delete[] s_data.quad_vertex_buffer_base;
}

void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform) {
  CK_PROFILE_FUNCTION();

  s_data.camera_buffer.view_projection = camera.GetProjection() * glm::inverse(transform);
  s_data.camera_uniform_buffer->SetData(&s_data.camera_buffer,
                                        sizeof(Renderer2DData::CameraData));

  StartBatch();
}

void Renderer2D::BeginScene(const EditorCamera& camera) {
  CK_PROFILE_FUNCTION();

  s_data.camera_buffer.view_projection = camera.GetViewProjection();
  s_data.camera_uniform_buffer->SetData(&s_data.camera_buffer,
                                        sizeof(Renderer2DData::CameraData));

  StartBatch();
}

void Renderer2D::BeginScene(const OrthographicCamera& camera) {
  CK_PROFILE_FUNCTION();

  s_data.camera_buffer.view_projection = camera.GetViewProjectionMatrix();
  s_data.camera_uniform_buffer->SetData(&s_data.camera_buffer,
                                        sizeof(Renderer2DData::CameraData));

  StartBatch();
}

void Renderer2D::EndScene() {
  CK_PROFILE_FUNCTION();

  Flush();
}

void Renderer2D::Flush() {
  if (s_data.quad_index_count == 0) {
    return;
  }

  uint32_t data_size =
      (uint8_t*)s_data.quad_vertex_buffer_ptr - (uint8_t*)s_data.quad_vertex_buffer_base;
  s_data.quad_vertex_buffer->SetData(s_data.quad_vertex_buffer_base, data_size);

  // Bind Textures
  for (uint32_t i = 0; i < s_data.texture_slot_index; i++) {
    s_data.texture_slots[i]->Bind(i);
  }

  s_data.texture_shader->Bind();
  RenderCommand::DrawIndexed(s_data.quad_vertex_array.get(), s_data.quad_index_count);
  s_data.stats.draw_calls++;
}

// ----------------------------------------------------------------------------: Primitives

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size,
                          const glm::vec4& color) {
  DrawQuad({position.x, position.y, 0.0f}, size, color);
}

void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size,
                          const glm::vec4& color) {
  glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                        glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

  DrawQuad(transform, color);
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

  glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                        glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
  DrawQuad(transform, texture, tiling_factor);
}

void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entity_id) {
  CK_PROFILE_FUNCTION();

  if (s_data.quad_index_count >= Renderer2DData::kMaxIndices) {
    NextBatch();
  }

  const float texture_index = 0.0f;  // white texture
  const float tiling_factor = 1.0f;
  constexpr size_t quad_vertex_count = 4;

  glm::vec2 uvs[4] = {
      {0.0f, 0.0f},
      {1.0f, 0.0f},
      {1.0f, 1.0f},
      {0.0f, 1.0f},
  };

  for (int i = 0; i < quad_vertex_count; i++) {
    s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_position[i];
    s_data.quad_vertex_buffer_ptr->color = color;
    s_data.quad_vertex_buffer_ptr->tex_coord = uvs[i];
    s_data.quad_vertex_buffer_ptr->tex_index = texture_index;
    s_data.quad_vertex_buffer_ptr->tiling_factor = tiling_factor;
    s_data.quad_vertex_buffer_ptr->entity_id = entity_id;
    s_data.quad_vertex_buffer_ptr++;
  }

  s_data.quad_index_count += 6;

  s_data.stats.quad_count++;
}

void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture,
                          float tiling_factor, const glm::vec4& tint_color, int entity_id) {
  CK_PROFILE_FUNCTION();

  if (s_data.quad_index_count >= Renderer2DData::kMaxIndices) {
    NextBatch();
  }

  float texture_index = 0.0f;

  // Check if this texture already in texture_slots
  for (uint32_t i = 1; i < s_data.texture_slot_index; i++) {
    if (*s_data.texture_slots[i].get() == *texture.get()) {
      texture_index = (float)i;
      break;
    }
  }

  // Texture Index still be zero means it's unique one that not in texture slots array
  if (texture_index == 0.0f) {

    if (s_data.quad_index_count >= Renderer2DData::kMaxIndices) {
      NextBatch();
    }
    texture_index = (float)s_data.texture_slot_index;
    s_data.texture_slots[s_data.texture_slot_index] = texture;
    s_data.texture_slot_index++;
  }

  glm::vec2 uvs[4] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};

  for (int i = 0; i < 4; i++) {
    s_data.quad_vertex_buffer_ptr->position = transform * s_data.quad_vertex_position[i];
    s_data.quad_vertex_buffer_ptr->color = tint_color;
    s_data.quad_vertex_buffer_ptr->tex_coord = uvs[i];
    s_data.quad_vertex_buffer_ptr->tex_index = texture_index;
    s_data.quad_vertex_buffer_ptr->tiling_factor = tiling_factor;
    s_data.quad_vertex_buffer_ptr->entity_id = entity_id;
    s_data.quad_vertex_buffer_ptr++;
  }

  s_data.quad_index_count += 6;

  s_data.stats.quad_count++;
}

// ----------------------------------------------------------------------------: (Rotated Version)

void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation,
                                 const glm::vec4& color) {
  DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, color);
}

void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation,
                                 const glm::vec4& color) {
  CK_PROFILE_FUNCTION();

  glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                        glm::rotate(glm::mat4(1.0f), glm::radians(rotation), {0.0f, 0.0f, 1.0f}) *
                        glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
  DrawQuad(transform, color);
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

  glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                        glm::rotate(glm::mat4(1.0f), glm::radians(rotation), {0.0f, 0.0f, 1.0f}) *
                        glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

  DrawQuad(transform, texture, tiling_factor, tint_color);
}

void Renderer2D::DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src,
                            int entity_id) {
  DrawQuad(transform, src.color, entity_id);
}

// ----------------------------------------------------------------------------: Stats

void Renderer2D::ResetStats() {
  memset(&s_data.stats, 0, sizeof(Statistics));
}

Renderer2D::Statistics Renderer2D::GetStats() {
  return s_data.stats;
}

void Renderer2D::StartBatch() {
  s_data.quad_index_count = 0;
  s_data.quad_vertex_buffer_ptr = s_data.quad_vertex_buffer_base;

  s_data.texture_slot_index = 1;
}

void Renderer2D::NextBatch() {
  Flush();
  StartBatch();
}

}  // namespace ck
