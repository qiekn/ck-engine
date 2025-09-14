#pragma once

#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "renderer/graphics_context.h"
#include "window.h"

namespace ck {
class WindowsWindow : public Window {
public:
  WindowsWindow(const WindowProps& props);
  virtual ~WindowsWindow();

  void OnUpdate() override;

  unsigned int GetWidth() const override { return data_.width; }
  unsigned int GetHeight() const override { return data_.height; }
  float GetScaleX() const override { return data_.content_scale_x; }
  float GetScaleY() const override { return data_.content_scale_y; }

  void SetEventCallback(const EventCallback& callback) override { data_.event_callback = callback; }
  void SetVSync(bool enabled) override;
  bool IsVSync() const override;

  void* GetNativeWindow() const override { return window_; }

private:
  virtual void Init(const WindowProps& props);
  virtual void Shutdown();

  GLFWwindow* window_;
  std::unique_ptr<GraphicContext> context_;

  struct WindowData {
    std::string title;
    unsigned int width, height;
    float content_scale_x, content_scale_y;
    bool vsync;
    EventCallback event_callback;
  };

  WindowData data_;
};
}  // namespace ck
