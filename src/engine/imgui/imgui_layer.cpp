#include "imgui_layer.h"

#include "GLFW/glfw3.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "core/application.h"
#include "core/layer.h"
#include "core/window.h"
#include "events/event.h"
#include "imgui.h"
#include "pch.h"

namespace ck {
ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {}

ImGuiLayer::~ImGuiLayer() {}

void ImGuiLayer::OnAttach() {
  CK_PROFILE_FUNCTION();
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();  // Setup Dear ImGui context

  ImGuiIO& io = ImGui::GetIO();
  (void)io;

  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Enable Multi-Viewport / Platform Windows

  // io.ConfigViewportsNoAutoMerge = true;
  // io.ConfigViewportsNoTaskBarIcon = true;

  ImGui::StyleColorsDark();

  ImGuiStyle& style = ImGui::GetStyle();
  style.WindowRounding = 8.0f;
  style.FrameRounding = 8.0f;

  // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look
  // identical to regular ones.
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  Application& app = Application::Get();
  GLFWwindow* window = (GLFWwindow*)(app.GetWindow()->GetNativeWindow());

  ImGui_ImplGlfw_InitForOpenGL(window, true);  // Platform binding
  ImGui_ImplOpenGL3_Init("#version 410");      // Renderer binding
}

void ImGuiLayer::OnDetach() {
  CK_PROFILE_FUNCTION();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void ImGuiLayer::OnEvent(Event& e) {
  if (is_block_events) {
    ImGuiIO& io = ImGui::GetIO();
    e.GetIsHandled() |= e.IsInCategorpy(EventCategoryMouse) & io.WantCaptureMouse;
    e.GetIsHandled() |= e.IsInCategorpy(EventCategoryKeyboard) & io.WantCaptureKeyboard;
  }
}

void ImGuiLayer::OnImGuiRender() {
  // static bool show = false;
  // ImGui::ShowDemoWindow(&show);
}

void ImGuiLayer::Begin() {
  CK_PROFILE_FUNCTION();
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void ImGuiLayer::End() {
  CK_PROFILE_FUNCTION();
  ImGuiIO& io = ImGui::GetIO();
  Application& app = Application::Get();
  io.DisplaySize = ImVec2((float)app.GetWindow()->GetWidth(), (float)app.GetWindow()->GetHeight());

  // RENDERING
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    GLFWwindow* backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);
  }
}
}  // namespace ck
