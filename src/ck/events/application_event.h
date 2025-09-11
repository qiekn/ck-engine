#pragma once

#include <sstream>

#include "events/event.h"

namespace ck {
class WindowResizeEvent : public Event {
public:
  WindowResizeEvent(int width, int height) : width_(width), height_(height) {}

  inline int GetWindowWidth() const { return width_; };
  inline int GetWindowHeight() const { return height_; };

  std::string ToString() const override {
    std::stringstream ss;
    ss << "WindowResizeEvent: New Window Size: (" << width_ << "," << height_ << ")";
    return ss.str();
  }

  EVENT_CLASS_TYPE(WindowResize)
  EVENT_CLASS_CATEGORY(EventCategoryApplication)
private:
  unsigned int width_, height_;
};

class WindowCloseEvent : public Event {
public:
  WindowCloseEvent() {}

  EVENT_CLASS_TYPE(WindowClose)
  EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class AppTickEvent : public Event {
public:
  AppTickEvent() {}

  EVENT_CLASS_TYPE(AppTick)
  EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class AppUpdateEvent : public Event {
public:
  AppUpdateEvent() {}

  EVENT_CLASS_TYPE(AppUpdate)
  EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class AppRenderEvent : public Event {
public:
  AppRenderEvent() {}

  EVENT_CLASS_TYPE(AppRender)
  EVENT_CLASS_CATEGORY(EventCategoryApplication)
};
}  // namespace ck
