#include "windows_window.h"

#include "events/application_event.h"
#include "events/key_event.h"
#include "events/mouse_event.h"
#include "log.h"

namespace ck {
static bool is_glfw_initialized = false;

static void GLFWErrorCallback(int error, const char* description) {
  CK_CLIENT_ERROR("GLFW ERROR ({}) {}", error, description);
}

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
    CK_ENGINE_ASSERT(success, "Could not initialize GLFW");
    glfwSetErrorCallback(GLFWErrorCallback);
    is_glfw_initialized = true;
    // Set all the required options for GLFW
    // NOTE: OpenGL on MacOS has effectively been deprecated since 2011.
    // It's stuck on an ancient version (4.1)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  }

  window_ =
      glfwCreateWindow((int)props.width, (int)props.height, data_.title.c_str(), nullptr, nullptr);
  glfwMakeContextCurrent(window_);

  int version = gladLoadGL(glfwGetProcAddress);
  CK_ENGINE_ASSERT(version, "Glad: Failed to initialize OpenGL context\n")
  CK_ENGINE_INFO("Loaded OpenGL {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

  glfwSetWindowUserPointer(window_, &data_);
  SetVSync(true);

  glfwSetWindowSizeCallback(window_, [](GLFWwindow* window, int width, int height) {
    WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
    data.width = width;
    data.height = height;

    WindowResizeEvent event(width, height);
    data.event_callback(event);
  });

  glfwSetWindowCloseCallback(window_, [](GLFWwindow* window) {
    WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
    WindowCloseEvent event;
    data.event_callback(event);
  });

  glfwSetKeyCallback(window_, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
    WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
    switch (action) {
      case GLFW_PRESS: {
        KeyPressedEvent event(key, 0);
        data.event_callback(event);
        break;
      }
      case GLFW_RELEASE: {
        KeyReleasedEvent event(key);
        data.event_callback(event);
        break;
      }
      case GLFW_REPEAT: {
        KeyPressedEvent event(key, 1);
        data.event_callback(event);
        break;
      }
    }
  });

  glfwSetMouseButtonCallback(window_, [](GLFWwindow* window, int button, int action, int mods) {
    WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
    switch (action) {
      case GLFW_PRESS: {
        MouseButtonPressedEvent event(button);
        data.event_callback(event);
        break;
      }
      case GLFW_RELEASE: {
        MouseButtonReleasedEvent event(button);
        data.event_callback(event);
        break;
      }
    }
  });

  glfwSetScrollCallback(window_, [](GLFWwindow* window, double x_offset, double y_offset) {
    WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
    MouseScrollEvent event((float)x_offset, (float)y_offset);
    data.event_callback(event);
  });

  glfwSetCursorPosCallback(window_, [](GLFWwindow* window, double x_pos, double y_pos) {
    WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
    MouseMoveEvent event((float)x_pos, (float)y_pos);
    data.event_callback(event);
  });

  CK_CLIENT_INFO("Window Init Finished");
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
