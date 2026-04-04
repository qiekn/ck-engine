#pragma once

#include "events/event.h"

namespace ck {

class KeyEvent : public Event {
public:
  inline int GetKeyCode() const { return keycode_; }

  EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

protected:
  KeyEvent(int keycode) : keycode_(keycode) {}

  int keycode_;
};

class KeyPressedEvent : public KeyEvent {
public:
  KeyPressedEvent(int keycode, bool is_repeat = false)
      : KeyEvent(keycode), is_repeat_(is_repeat) {}

  bool IsRepeat() const { return is_repeat_; }

  std::string ToString() const override {
    std::stringstream ss;
    ss << "KeyPressEvent: " << keycode_ << " (repeat = " << is_repeat_ << ")";
    return ss.str();
  }

  EVENT_CLASS_TYPE(KeyPressed)

private:
  bool is_repeat_;
};

class KeyReleasedEvent : public KeyEvent {
public:
  KeyReleasedEvent(int keycode) : KeyEvent(keycode) {}

  std::string ToString() const override {
    std::stringstream ss;
    ss << "KeyReleasedEvent";
    return ss.str();
  }

  EVENT_CLASS_TYPE(KeyReleased)

private:
  int keycode_;
};
}  // namespace ck
