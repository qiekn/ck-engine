#pragma once

#include "core.h"
#include "events/event.h"
#include "pch.h"

namespace ck {
struct WindowProps {  // Props -> properties
  std::string title;
  uint32_t width;
  uint32_t height;

  explicit WindowProps(const std::string& _title = "ck: Engine", uint32_t _width = 1600,
                       uint32_t _height = 900)
      : title(_title), width(_width), height(_height) {}
};

// Interface representing a desktop system based window
class Window {
public:
  using EventCallback = std::function<void(Event&)>;

  virtual ~Window() {}

  virtual void OnUpdate() = 0;

  virtual uint32_t GetWidth() const = 0;
  virtual uint32_t GetHeight() const = 0;
  virtual float GetScaleX() const = 0;
  virtual float GetScaleY() const = 0;

  // Window attributes
  virtual void SetEventCallback(const EventCallback& callback) = 0;
  virtual void SetVSync(bool enabled) = 0;
  virtual bool IsVSync() const = 0;

  virtual void* GetNativeWindow() const = 0;

  static Scope<Window> Create(const WindowProps& props = WindowProps());

public:
  static float s_high_dpi_scale_factor_;
};
}  // namespace ck
