#pragma once

#include "GLFW/glfw3.h"
#include "window.h"

namespace ck {
class WindowsWindow : public Window {
public:
  WindowsWindow(const WindowProps& props);
  virtual ~WindowsWindow();

  void OnUpdate() override;

  unsigned int GetWidth() const override { return data_.width; }
  unsigned int GetHeight() const override { return data_.height; }

  void SetEventCallback(const EventCallback& callback) override { data_.event_callback = callback; }
  void SetVSync(bool enabled) override;
  bool IsVSync() const override;

private:
  virtual void Init(const WindowProps& props);
  virtual void Shutdown();

  GLFWwindow* window_;

  struct WindowData {
    std::string title;
    unsigned int width, height;
    bool vsync;
    EventCallback event_callback;
  };

  WindowData data_;
};
}  // namespace ck
