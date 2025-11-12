#include "renderer.h"

#include "glm/ext/matrix_float4x4.hpp"
#include "renderer/render_command.h"

namespace ck {
void Renderer::BeginScene(OrthographicCamera& camera) {
  scene_data_->view_projection_ = camera.GetViewProjectionMatrix();
}

void Renderer::Submit(const Shader* shader, const VertexArray* vertex_array,
                      const glm::mat4& transform) {
  shader->Bind();
  shader->UploadUniformMat4("u_view_projection", scene_data_->view_projection_);
  shader->UploadUniformMat4("u_transform", transform);
  vertex_array->Bind();
  RenderCommand::DrawIndexed(vertex_array);
}

void Renderer::EndScene() {}

Renderer::SceneData* Renderer::scene_data_ = new SceneData();
}  // namespace ck
