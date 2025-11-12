#include "renderer.h"

#include "renderer/render_command.h"

namespace ck {
void Renderer::BeginScene(OrthographicCamera& camera) {
  scene_data_->view_projection_ = camera.GetViewProjectionMatrix();
}

void Renderer::Submit(const Shader* shader, const VertexArray* vertex_array) {
  shader->Bind();
  shader->UploadUniformMat4("u_view_projection", scene_data_->view_projection_);
  vertex_array->Bind();
  RenderCommand::DrawIndexed(vertex_array);
}

void Renderer::EndScene() {}

Renderer::SceneData* Renderer::scene_data_ = new SceneData();
}  // namespace ck
