#include "renderer.h"

#include "renderer/render_command.h"

namespace ck {
void Renderer::BeginScene() {}

void Renderer::EndScene() {}

void Renderer::Submit(VertexArray* vertex_array) {
  vertex_array->Bind();
  RenderCommand::DrawIndexed(vertex_array);
}
}  // namespace ck
