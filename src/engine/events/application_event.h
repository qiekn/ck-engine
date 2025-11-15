#pragma once

#include <format>

#include "event.h"

namespace ck {
class WindowResizeEvent : public Event {
public:
  WindowResizeEvent(int width, int height) : width_(width), height_(height) {}

  inline int GetWindowWidth() const { return width_; };
  inline int GetWindowHeight() const { return height_; };

  std::string ToString() const override {
    return std::format("WindowResizeEvent: New Window Size: ({},{})", width_, height_);
  }

  EVENT_CLASS_TYPE(WindowResize)
  EVENT_CLASS_CATEGORY(EventCategoryApplication)
private:
  int width_, height_;
};

class WindowCloseEvent : public Event {
public:
  EVENT_CLASS_TYPE(WindowClose)
  EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class AppTickEvent : public Event {
public:
  EVENT_CLASS_TYPE(AppTick)
  EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class AppUpdateEvent : public Event {
public:
  EVENT_CLASS_TYPE(AppUpdate)
  EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class AppRenderEvent : public Event {
public:
  EVENT_CLASS_TYPE(AppRender)
  EVENT_CLASS_CATEGORY(EventCategoryApplication)
};
}  // namespace ck
