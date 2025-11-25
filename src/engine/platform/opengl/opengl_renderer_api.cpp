#include "opengl_renderer_api.h"

#include <glm/glm.hpp>

#include "core/application.h"
#include "core/core.h"
#include "core/window.h"
#include "glad/gl.h"

namespace ck {
void OpenGLRendererAPI::Init() {
  CK_PROFILE_FUNCTION();
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_DEPTH_TEST);
}

void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
  const Scope<Window>& window = Application::Get().GetWindow();
  glViewport(x, y, width * window->GetScaleX(), height * window->GetScaleY());
}

void OpenGLRendererAPI::Clear() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

void OpenGLRendererAPI::SetClearColor(const glm::vec4& color) {
  glClearColor(color.r, color.g, color.b, color.a);
}

void OpenGLRendererAPI::DrawIndexed(const VertexArray* vertex_array, uint32_t index_count) {
  uint32_t count = index_count ? index_count : vertex_array->GetIndexBuffer()->Count();
  glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
  glBindTexture(GL_TEXTURE_2D, 0);
}

}  // namespace ck
