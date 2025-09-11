#pragma once

#include "events/event.h"
#include "pch.h"

namespace ck {
struct WindowProps {  // Props -> properties
  std::string title;
  unsigned int width;
  unsigned int height;

  WindowProps(const std::string& _title = "Ck Engine", unsigned int _width = 1280,
              unsigned int _height = 720)
      : title(_title), width(_width), height(_height) {}
};

// Interface representing a desktop system based window
class Window {
public:
  using EventCallback = std::function<void(Event&)>;
  virtual ~Window() {}

  virtual void OnUpdate() = 0;

  virtual unsigned int GetWidth() const = 0;
  virtual unsigned int GetHeight() const = 0;

  // Window attributes
  virtual void SetEventCallback(const EventCallback& callback) = 0;
  virtual void SetVSync(bool enabled) = 0;
  virtual bool IsVSync() const = 0;

  static Window* Create(const WindowProps& props = WindowProps());
};
}  // namespace ck
