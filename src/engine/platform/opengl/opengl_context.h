#pragma once

#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "renderer/graphics_context.h"

namespace ck {
class OpenGlContext : public GraphicContext {
public:
  OpenGlContext(GLFWwindow* window);
  virtual ~OpenGlContext();

  void Init() override;
  void SwapBuffers() override;

private:
  GLFWwindow* window_;
};
}  // namespace ck
