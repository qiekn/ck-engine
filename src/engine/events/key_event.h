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
  KeyPressedEvent(int keycode, int repeat_count) : KeyEvent(keycode), repeat_count_(repeat_count) {}
  inline int GetRepeatCount() const { return repeat_count_; }

  std::string ToString() const override {
    std::stringstream ss;
    ss << "KeyPressEvent: " << keycode_ << "(" << repeat_count_ << " Repeats)";
    return ss.str();
  }

  EVENT_CLASS_TYPE(KeyPressed)

private:
  int repeat_count_;
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
