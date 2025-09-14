#include "opengl_context.h"

#include "GLFW/glfw3.h"
#include "log.h"

namespace ck {

OpenGlContext::OpenGlContext(GLFWwindow* window) : window_(window) {
  CK_ENGINE_ASSERT(window_, "Window handle is null");
}

OpenGlContext::~OpenGlContext() {}

void OpenGlContext::Init() {
  glfwMakeContextCurrent(window_);
  int version = gladLoadGL(glfwGetProcAddress);
  CK_ENGINE_ASSERT(version, "Glad: Failed to initialize OpenGL context\n")
  CK_ENGINE_INFO("Loaded OpenGL {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
}

void OpenGlContext::SwapBuffers() { glfwSwapBuffers(window_); }

}  // namespace ck
