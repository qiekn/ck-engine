#pragma once

namespace ck {
typedef enum class KeyCode : uint16_t {
  // From glfw3.h
  Space = 32,
  Apostrophe = 39, /* ' */
  Comma = 44,      /* , */
  Minus = 45,      /* - */
  Period = 46,     /* . */
  Slash = 47,      /* / */

  D0 = 48, /* 0 */
  D1 = 49, /* 1 */
  D2 = 50, /* 2 */
  D3 = 51, /* 3 */
  D4 = 52, /* 4 */
  D5 = 53, /* 5 */
  D6 = 54, /* 6 */
  D7 = 55, /* 7 */
  D8 = 56, /* 8 */
  D9 = 57, /* 9 */

  Semicolon = 59, /* ; */
  Equal = 61,     /* = */

  A = 65,
  B = 66,
  C = 67,
  D = 68,
  E = 69,
  F = 70,
  G = 71,
  H = 72,
  I = 73,
  J = 74,
  K = 75,
  L = 76,
  M = 77,
  N = 78,
  O = 79,
  P = 80,
  Q = 81,
  R = 82,
  S = 83,
  T = 84,
  U = 85,
  V = 86,
  W = 87,
  X = 88,
  Y = 89,
  Z = 90,

  LeftBracket = 91,  /* [ */
  Backslash = 92,    /* \ */
  RightBracket = 93, /* ] */
  GraveAccent = 96,  /* ` */

  World1 = 161, /* non-US #1 */
  World2 = 162, /* non-US #2 */

  /* Function keys */
  Escape = 256,
  Enter = 257,
  Tab = 258,
  Backspace = 259,
  Insert = 260,
  Delete = 261,
  Right = 262,
  Left = 263,
  Down = 264,
  Up = 265,
  PageUp = 266,
  PageDown = 267,
  Home = 268,
  End = 269,
  CapsLock = 280,
  ScrollLock = 281,
  NumLock = 282,
  PrintScreen = 283,
  Pause = 284,
  F1 = 290,
  F2 = 291,
  F3 = 292,
  F4 = 293,
  F5 = 294,
  F6 = 295,
  F7 = 296,
  F8 = 297,
  F9 = 298,
  F10 = 299,
  F11 = 300,
  F12 = 301,
  F13 = 302,
  F14 = 303,
  F15 = 304,
  F16 = 305,
  F17 = 306,
  F18 = 307,
  F19 = 308,
  F20 = 309,
  F21 = 310,
  F22 = 311,
  F23 = 312,
  F24 = 313,
  F25 = 314,

  /* Keypad */
  KP0 = 320,
  KP1 = 321,
  KP2 = 322,
  KP3 = 323,
  KP4 = 324,
  KP5 = 325,
  KP6 = 326,
  KP7 = 327,
  KP8 = 328,
  KP9 = 329,
  KPDecimal = 330,
  KPDivide = 331,
  KPMultiply = 332,
  KPSubtract = 333,
  KPAdd = 334,
  KPEnter = 335,
  KPEqual = 336,

  LeftShift = 340,
  LeftControl = 341,
  LeftAlt = 342,
  LeftSuper = 343,
  RightShift = 344,
  RightControl = 345,
  RightAlt = 346,
  RightSuper = 347,
  Menu = 348
} Key;

inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode) {
  os << static_cast<int32_t>(keyCode);
  return os;
}
}  // namespace ck

// From glfw3.h
#define CK_KEY_SPACE ::ck::Key::Space
#define CK_KEY_APOSTROPHE ::ck::Key::Apostrophe /* ' */
#define CK_KEY_COMMA ::ck::Key::Comma           /* , */
#define CK_KEY_MINUS ::ck::Key::Minus           /* - */
#define CK_KEY_PERIOD ::ck::Key::Period         /* . */
#define CK_KEY_SLASH ::ck::Key::Slash           /* / */
#define CK_KEY_0 ::ck::Key::D0
#define CK_KEY_1 ::ck::Key::D1
#define CK_KEY_2 ::ck::Key::D2
#define CK_KEY_3 ::ck::Key::D3
#define CK_KEY_4 ::ck::Key::D4
#define CK_KEY_5 ::ck::Key::D5
#define CK_KEY_6 ::ck::Key::D6
#define CK_KEY_7 ::ck::Key::D7
#define CK_KEY_8 ::ck::Key::D8
#define CK_KEY_9 ::ck::Key::D9
#define CK_KEY_SEMICOLON ::ck::Key::Semicolon /* ; */
#define CK_KEY_EQUAL ::ck::Key::Equal         /* = */
#define CK_KEY_A ::ck::Key::A
#define CK_KEY_B ::ck::Key::B
#define CK_KEY_C ::ck::Key::C
#define CK_KEY_D ::ck::Key::D
#define CK_KEY_E ::ck::Key::E
#define CK_KEY_F ::ck::Key::F
#define CK_KEY_G ::ck::Key::G
#define CK_KEY_H ::ck::Key::H
#define CK_KEY_I ::ck::Key::I
#define CK_KEY_J ::ck::Key::J
#define CK_KEY_K ::ck::Key::K
#define CK_KEY_L ::ck::Key::L
#define CK_KEY_M ::ck::Key::M
#define CK_KEY_N ::ck::Key::N
#define CK_KEY_O ::ck::Key::O
#define CK_KEY_P ::ck::Key::P
#define CK_KEY_Q ::ck::Key::Q
#define CK_KEY_R ::ck::Key::R
#define CK_KEY_S ::ck::Key::S
#define CK_KEY_T ::ck::Key::T
#define CK_KEY_U ::ck::Key::U
#define CK_KEY_V ::ck::Key::V
#define CK_KEY_W ::ck::Key::W
#define CK_KEY_X ::ck::Key::X
#define CK_KEY_Y ::ck::Key::Y
#define CK_KEY_Z ::ck::Key::Z
#define CK_KEY_LEFT_BRACKET ::ck::Key::LeftBracket   /* [ */
#define CK_KEY_BACKSLASH ::ck::Key::Backslash        /* \ */
#define CK_KEY_RIGHT_BRACKET ::ck::Key::RightBracket /* ] */
#define CK_KEY_GRAVE_ACCENT ::ck::Key::GraveAccent   /* ` */
#define CK_KEY_WORLD_1 ::ck::Key::World1             /* non-US #1 */
#define CK_KEY_WORLD_2 ::ck::Key::World2             /* non-US #2 */

/* Function keys */
#define CK_KEY_ESCAPE ::ck::Key::Escape
#define CK_KEY_ENTER ::ck::Key::Enter
#define CK_KEY_TAB ::ck::Key::Tab
#define CK_KEY_BACKSPACE ::ck::Key::Backspace
#define CK_KEY_INSERT ::ck::Key::Insert
#define CK_KEY_DELETE ::ck::Key::Delete
#define CK_KEY_RIGHT ::ck::Key::Right
#define CK_KEY_LEFT ::ck::Key::Left
#define CK_KEY_DOWN ::ck::Key::Down
#define CK_KEY_UP ::ck::Key::Up
#define CK_KEY_PAGE_UP ::ck::Key::PageUp
#define CK_KEY_PAGE_DOWN ::ck::Key::PageDown
#define CK_KEY_HOME ::ck::Key::Home
#define CK_KEY_END ::ck::Key::End
#define CK_KEY_CAPS_LOCK ::ck::Key::CapsLock
#define CK_KEY_SCROLL_LOCK ::ck::Key::ScrollLock
#define CK_KEY_NUM_LOCK ::ck::Key::NumLock
#define CK_KEY_PRINT_SCREEN ::ck::Key::PrintScreen
#define CK_KEY_PAUSE ::ck::Key::Pause
#define CK_KEY_F1 ::ck::Key::F1
#define CK_KEY_F2 ::ck::Key::F2
#define CK_KEY_F3 ::ck::Key::F3
#define CK_KEY_F4 ::ck::Key::F4
#define CK_KEY_F5 ::ck::Key::F5
#define CK_KEY_F6 ::ck::Key::F6
#define CK_KEY_F7 ::ck::Key::F7
#define CK_KEY_F8 ::ck::Key::F8
#define CK_KEY_F9 ::ck::Key::F9
#define CK_KEY_F10 ::ck::Key::F10
#define CK_KEY_F11 ::ck::Key::F11
#define CK_KEY_F12 ::ck::Key::F12
#define CK_KEY_F13 ::ck::Key::F13
#define CK_KEY_F14 ::ck::Key::F14
#define CK_KEY_F15 ::ck::Key::F15
#define CK_KEY_F16 ::ck::Key::F16
#define CK_KEY_F17 ::ck::Key::F17
#define CK_KEY_F18 ::ck::Key::F18
#define CK_KEY_F19 ::ck::Key::F19
#define CK_KEY_F20 ::ck::Key::F20
#define CK_KEY_F21 ::ck::Key::F21
#define CK_KEY_F22 ::ck::Key::F22
#define CK_KEY_F23 ::ck::Key::F23
#define CK_KEY_F24 ::ck::Key::F24
#define CK_KEY_F25 ::ck::Key::F25

/* Keypad */
#define CK_KEY_KP_0 ::ck::Key::KP0
#define CK_KEY_KP_1 ::ck::Key::KP1
#define CK_KEY_KP_2 ::ck::Key::KP2
#define CK_KEY_KP_3 ::ck::Key::KP3
#define CK_KEY_KP_4 ::ck::Key::KP4
#define CK_KEY_KP_5 ::ck::Key::KP5
#define CK_KEY_KP_6 ::ck::Key::KP6
#define CK_KEY_KP_7 ::ck::Key::KP7
#define CK_KEY_KP_8 ::ck::Key::KP8
#define CK_KEY_KP_9 ::ck::Key::KP9
#define CK_KEY_KP_DECIMAL ::ck::Key::KPDecimal
#define CK_KEY_KP_DIVIDE ::ck::Key::KPDivide
#define CK_KEY_KP_MULTIPLY ::ck::Key::KPMultiply
#define CK_KEY_KP_SUBTRACT ::ck::Key::KPSubtract
#define CK_KEY_KP_ADD ::ck::Key::KPAdd
#define CK_KEY_KP_ENTER ::ck::Key::KPEnter
#define CK_KEY_KP_EQUAL ::ck::Key::KPEqual

#define CK_KEY_LEFT_SHIFT ::ck::Key::LeftShift
#define CK_KEY_LEFT_CONTROL ::ck::Key::LeftControl
#define CK_KEY_LEFT_ALT ::ck::Key::LeftAlt
#define CK_KEY_LEFT_SUPER ::ck::Key::LeftSuper
#define CK_KEY_RIGHT_SHIFT ::ck::Key::RightShift
#define CK_KEY_RIGHT_CONTROL ::ck::Key::RightControl
#define CK_KEY_RIGHT_ALT ::ck::Key::RightAlt
#define CK_KEY_RIGHT_SUPER ::ck::Key::RightSuper
#define CK_KEY_MENU ::ck::Key::Menu
