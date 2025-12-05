
#include "GLFW/glfw3.h"
#include "core/application.h"
#include "core/input.h"

namespace ck {

bool Input::IsKeyPressed(const KeyCode key) {
  auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow()->GetNativeWindow());
  auto state = glfwGetKey(window, static_cast<int32_t>(key));
  return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::IsMouseButtonPressed(const MouseCode button) {
  auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow()->GetNativeWindow());
  auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
  return state == GLFW_PRESS;
}

glm::vec2 Input::GetMousePos() {
  auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow()->GetNativeWindow());
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  return {(float)x, (float)y};
}

float Input::GetMouseX() {
  return GetMousePos().x;
}

float Input::GetMouseY() {
  return GetMousePos().y;
}

}  // namespace ck
