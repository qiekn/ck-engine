#include "windows_window.h"

#include "GLFW/glfw3.h"
#include "log.h"

namespace ck {
static bool is_glfw_initialized = false;

Window* Window::Create(const WindowProps& props) { return new WindowsWindow(props); }

WindowsWindow::WindowsWindow(const WindowProps& props) { Init(props); }

WindowsWindow::~WindowsWindow() { Shutdown(); }

void WindowsWindow::Init(const WindowProps& props) {
  data_.title = props.title;
  data_.width = props.width;
  data_.height = props.height;
  CK_ENGINE_TRACE("Create window {}, ({}, {})", props.title, props.width, props.height);

  if (!is_glfw_initialized) {
    // TODO: glfwTerminate on system shutdown <2025-09-12 06:59, @qiekn> //
    int success = glfwInit();
    CK_ENGINE_ASSET(success, "Could not initialize GLFW");
    is_glfw_initialized = true;
  }

  window_ =
      glfwCreateWindow((int)props.width, (int)props.height, data_.title.c_str(), nullptr, nullptr);
  glfwMakeContextCurrent(window_);
  glfwSetWindowUserPointer(window_, &data_);
  SetVSync(true);
}

void WindowsWindow::OnUpdate() {
  glfwPollEvents();
  glfwSwapBuffers(window_);
}

void WindowsWindow::SetVSync(bool enabled) {
  if (enabled) {
    glfwSwapInterval(1);
  } else {
    glfwSwapInterval(0);
  }
  data_.vsync = enabled;
}

bool WindowsWindow::IsVSync() const { return data_.vsync; }

void WindowsWindow::Shutdown() { glfwDestroyWindow(window_); }
}  // namespace ck
