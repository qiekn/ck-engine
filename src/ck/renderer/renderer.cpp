#include "renderer.h"

#include "glm/ext/matrix_float4x4.hpp"
#include "platform/opengl/opengl_shader.h"
#include "renderer/render_command.h"

namespace ck {
void Renderer::Init() { RenderCommand::Init(); }

void Renderer::BeginScene(OrthographicCamera& camera) {
  scene_data_->view_projection_ = camera.GetViewProjectionMatrix();
}

void Renderer::Submit(const Shader* shader, const VertexArray* vertex_array,
                      const glm::mat4& transform) {
  auto opengl_shader = static_cast<const OpenGLShader*>(shader);
  opengl_shader->Bind();
  opengl_shader->UploadUniformMat4("u_view_projection", scene_data_->view_projection_);
  opengl_shader->UploadUniformMat4("u_transform", transform);
  vertex_array->Bind();
  RenderCommand::DrawIndexed(vertex_array);
}

void Renderer::EndScene() {}

Renderer::SceneData* Renderer::scene_data_ = new SceneData();
}  // namespace ck
