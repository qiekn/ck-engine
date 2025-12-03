#pragma once

namespace ck {
typedef enum class MouseCode : uint8_t {
  // From glfw3.h
  Button0 = 0,
  Button1 = 1,
  Button2 = 2,
  Button3 = 3,
  Button4 = 4,
  Button5 = 5,
  Button6 = 6,
  Button7 = 7,

  ButtonLast = Button7,
  ButtonLeft = Button0,
  ButtonRight = Button1,
  ButtonMiddle = Button2
} Mouse;

inline std::ostream& operator<<(std::ostream& os, MouseCode mouseCode) {
  os << static_cast<int32_t>(mouseCode);
  return os;
}
}  // namespace ck

#define CK_MOUSE_BUTTON_0 ::ck::Mouse::Button0
#define CK_MOUSE_BUTTON_1 ::ck::Mouse::Button1
#define CK_MOUSE_BUTTON_2 ::ck::Mouse::Button2
#define CK_MOUSE_BUTTON_3 ::ck::Mouse::Button3
#define CK_MOUSE_BUTTON_4 ::ck::Mouse::Button4
#define CK_MOUSE_BUTTON_5 ::ck::Mouse::Button5
#define CK_MOUSE_BUTTON_6 ::ck::Mouse::Button6
#define CK_MOUSE_BUTTON_7 ::ck::Mouse::Button7
#define CK_MOUSE_BUTTON_LAST ::ck::Mouse::ButtonLast
#define CK_MOUSE_BUTTON_LEFT ::ck::Mouse::ButtonLeft
#define CK_MOUSE_BUTTON_RIGHT ::ck::Mouse::ButtonRight
#define CK_MOUSE_BUTTON_MIDDLE ::ck::Mouse::ButtonMiddle
