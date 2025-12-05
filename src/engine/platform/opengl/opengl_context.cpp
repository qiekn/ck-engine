#include "opengl_context.h"

#include "GLFW/glfw3.h"
#include "core/log.h"

namespace ck {

OpenGlContext::OpenGlContext(GLFWwindow* window) : window_(window) {
  CK_ENGINE_ASSERT(window_, "Window handle is null");
}

OpenGlContext::~OpenGlContext() {}

void OpenGlContext::Init() {
  CK_PROFILE_FUNCTION();
  glfwMakeContextCurrent(window_);
  int version = gladLoadGL(glfwGetProcAddress);
  CK_ENGINE_ASSERT(version, "Glad: Failed to initialize OpenGL context\n")

  int version_minor = GLAD_VERSION_MINOR(version);
  int version_major = GLAD_VERSION_MAJOR(version);
  CK_ENGINE_INFO("Loaded OpenGL {}.{}", version_major, version_major);
  CK_ENGINE_ASSERT(version_major > 4 || (version_major == 4 && version_minor >= 5),
                   "ck-engine requires at least OpenGL version 4.5!");
}

void OpenGlContext::SwapBuffers() {
  CK_PROFILE_FUNCTION();
  glfwSwapBuffers(window_);
}

}  // namespace ck
