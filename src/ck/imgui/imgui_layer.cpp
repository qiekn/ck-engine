#include "imgui_layer.h"

#include "application.h"
#include "events/event.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "layer.h"
#include "pch.h"
#include "platform/opengl/imgui_opengl_renderer.h"
#include "window.h"

namespace ck {
ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {}

ImGuiLayer::~ImGuiLayer() {}

void ImGuiLayer::OnAttach() {
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  ImGuiIO& io = ImGui::GetIO();
  io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
  io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

  ImGui_ImplOpenGL3_Init("#version 410");
}

void ImGuiLayer::OnDetach() {}

void ImGuiLayer::OnUpdate() {
  ImGuiIO& io = ImGui::GetIO();

  Application& app = Application::Get();
  const auto& window = app.GetWindow();
  io.DisplaySize = ImVec2(window->GetWidth(), window->GetHeight());
  io.DisplayFramebufferScale = ImVec2(window->GetScaleX(), window->GetScaleY());

  float time = (float)glfwGetTime();
  io.DeltaTime = time_ > 0.0 ? time - time_ : 1.0f / 60.0f;
  time_ = time;

  ImGui_ImplOpenGL3_NewFrame();
  ImGui::NewFrame();

  static bool show = true;
  ImGui::ShowDemoWindow(&show);

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLayer::OnEvent(Event& e) {
  auto dispatcher = EventDispatcher(e);
  dispatcher.DispatchEvent<MouseButtonPressedEvent>(
      CK_BIND_EVENT(ImGuiLayer::OnMouseButtonPressedEvent));
  dispatcher.DispatchEvent<MouseButtonReleasedEvent>(
      CK_BIND_EVENT(ImGuiLayer::OnMouseButtonReleasedEvent));
  dispatcher.DispatchEvent<MouseMoveEvent>(CK_BIND_EVENT(ImGuiLayer::OnMouseMoveEvent));
  dispatcher.DispatchEvent<MouseScrollEvent>(CK_BIND_EVENT(ImGuiLayer::OnMouseScrollEvent));
  dispatcher.DispatchEvent<KeyPressedEvent>(CK_BIND_EVENT(ImGuiLayer::OnKeyPressedEvent));
  // dispatcher.DispatchEvent<KeyTypedEvent>(CK_BIND_EVENT(ImGuiLayer::OnKeyType);
  dispatcher.DispatchEvent<KeyReleasedEvent>(CK_BIND_EVENT(ImGuiLayer::OnKeyReleasedEvent));
  dispatcher.DispatchEvent<WindowResizeEvent>(CK_BIND_EVENT(ImGuiLayer::OnWindowResizeEvent));
}

bool ImGuiLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& e) {
  ImGuiIO& io = ImGui::GetIO();
  io.MouseDown[e.GetMouseButton()] = true;

  // Let other layers to respond this input event.
  return false;
}

bool ImGuiLayer::OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& e) {
  ImGuiIO& io = ImGui::GetIO();
  io.MouseDown[e.GetMouseButton()] = false;
  return false;
}
bool ImGuiLayer::OnMouseMoveEvent(MouseMoveEvent& e) {
  ImGuiIO& io = ImGui::GetIO();
  io.MousePos = ImVec2(e.GetMouseX(), e.GetMouseY());
  return false;
}
bool ImGuiLayer::OnMouseScrollEvent(MouseScrollEvent& e) {
  ImGuiIO& io = ImGui::GetIO();
  io.MouseWheelH += e.GetMouseXOffset();
  io.MouseWheel += e.GetMouseYOffset();
  return false;
}
bool ImGuiLayer::OnKeyPressedEvent(KeyPressedEvent& e) {
  // ImGuiIO& io = ImGui::GetIO();
  // io.KeysDown[e.GetKeyCode()] = true;

  // io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
  // io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
  // io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
  // io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];

  return false;
}
bool ImGuiLayer::OnKeyReleasedEvent(KeyReleasedEvent& e) {
  // ImGuiIO& io = ImGui::GetIO();
  // io.KeysDown[e.GetKeyCode()] = false;
  return false;
}
bool ImGuiLayer::OnKeyEvent(KeyEvent& e) {
  ImGuiIO& io = ImGui::GetIO();
  int keycode = e.GetKeyCode();
  if (keycode > 0 && keycode < 0x10000) {
    io.AddInputCharacter((unsigned short)keycode);
  }

  return false;
}
bool ImGuiLayer::OnWindowResizeEvent(WindowResizeEvent& e) {
  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = ImVec2(e.GetWindowWidth(), e.GetWindowHeight());
  io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

  // TEMP
  glViewport(0, 0, e.GetWindowWidth(), e.GetWindowHeight());

  return false;
}
}  // namespace ck
