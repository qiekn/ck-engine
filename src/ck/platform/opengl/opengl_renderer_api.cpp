#include "opengl_renderer_api.h"

#include <glm/glm.hpp>

#include "glad/gl.h"

namespace ck {

void OpenGLRendererAPI::Clear() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

void OpenGLRendererAPI::SetClearColor(const glm::vec4& color) {
  glClearColor(color.r, color.g, color.b, color.a);
}

void OpenGLRendererAPI::DrawIndexed(const VertexArray* vertex_array) {
  glDrawElements(GL_TRIANGLES, vertex_array->GetIndexBuffer()->Count(), GL_UNSIGNED_INT, nullptr);
}

}  // namespace ck
