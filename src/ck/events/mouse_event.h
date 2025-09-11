#pragma once

#include "events/event.h"

namespace ck {
class MouseMoveEvent : public Event {
public:
  MouseMoveEvent(float x, float y) : mouse_x_(x), mouse_y_(y) {}
  inline float GetMouseX() { return mouse_x_; }
  inline float GetMouseY() { return mouse_y_; }

  std::string ToString() const override {
    std::stringstream ss;
    ss << "MouseMovedEvent: (" << mouse_x_ << "," << mouse_y_ << ")";
    return ss.str();
  }

  EVENT_CLASS_TYPE(MouseMoved)
  EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)

private:
  float mouse_x_, mouse_y_;
};

class MouseScrollEvent : public Event {
public:
  MouseScrollEvent(float x, float y) : mouse_x_offset_(x), mouse_y_offset_(y) {}
  inline float GetMouseXOffset() { return mouse_x_offset_; }
  inline float GetMouseYOffset() { return mouse_y_offset_; }

  std::string ToString() const override {
    return std::format("MouseScrollEvent: ({},{})", mouse_x_offset_, mouse_y_offset_);
  }

  EVENT_CLASS_TYPE(MouseScrolled)
  EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)

private:
  float mouse_x_offset_, mouse_y_offset_;
};

class MouseButtonEvent : public Event {
public:
  MouseButtonEvent(int button) : mouse_button_(button) {}
  inline int GetMouseButton() const { return mouse_button_; }

  EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse | EventCategoryMouseButton);

protected:
  int mouse_button_;
};

class MouseButtonPressedEvent : MouseButtonEvent {
public:
  MouseButtonPressedEvent(int button) : MouseButtonEvent(button) {}

  std::string ToString() const override {
    std::stringstream ss;
    ss << "MouseButtonPressedEvent: " << mouse_button_;
    return ss.str();
  }

  EVENT_CLASS_TYPE(MouseButtonPressed);
};

class MouseButtonReleasedEvent : public MouseButtonEvent {
public:
  MouseButtonReleasedEvent(int button) : MouseButtonEvent(button) {}

  std::string ToString() const override {
    std::stringstream ss;
    ss << "MouseButtonReleasedEvent: " << mouse_button_;
    return ss.str();
  }

  EVENT_CLASS_TYPE(MouseButtonReleased);
};
}  // namespace ck
