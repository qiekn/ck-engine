#pragma once

#include "core.h"

namespace ck {
// clang-format off
enum class EventType {
  None = 0,

  WindowClose,
  WindowResize,
  WindowFocus,
  WindowLostFocus,
  WindowMoved,

  AppTick,
  AppUpdate,
  AppRender,

  KeyPressed,
  KeyReleased,

  MouseButtonPressed,
  MouseButtonReleased,
  MouseMoved,
  MouseScrolled
};

// 这个枚举是用来做 Filter 的
// 这是使用普通枚举类型, 目的是方便 OR / AND 操作
// 可以隐式转换为 int, 而 enum class 需要显式 cast
enum EventCategory {
  None = 0,
  EventCategoryApplication = BIT(0), // 0b1
  EventCategoryInput =       BIT(1), // 0b10
  EventCategoryKeyboard =    BIT(2), // 0b100
  EventCategoryMouse =       BIT(3),
  EventCategoryMouseButton = BIT(4)
};
// clang-format on

// # 用来把参数转为字符串字面量
#define EVENT_CLASS_TYPE(type)                                                \
  static EventType GetStaticType() { return EventType::type; }                \
  virtual EventType GetEventType() const override { return GetStaticType(); } \
  virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) \
  virtual int GetCategoryFlags() const override { return category; }

class Event {
  friend class EventDispatcher;

public:
  virtual auto GetEventType() const -> EventType = 0;
  virtual const char* GetName() const = 0;
  virtual int GetCategoryFlags() const = 0;
  virtual std::string ToString() const { return GetName(); }

  inline virtual bool IsInCategorpy(EventCategory category) {
    return GetCategoryFlags() & category;
  }

protected:
  bool handled_{false};
};

class EventDispatcher {
  template <typename T>
  using EventFunc = std::function<bool(T&)>;

public:
  EventDispatcher(Event& event) : event_(event) {}

  template <typename T>
  bool DispatchEvent(EventFunc<T> Func) {
    if (event_.GetEventType() == T::GetStaticType()) {
      event_.handled_ = Func(*(T*)&event_);
      return true;
    }
    return false;
  }

private:
  Event& event_;
};

}  // namespace ck
