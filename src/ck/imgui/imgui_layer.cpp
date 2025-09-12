#include "imgui_layer.h"

#include "application.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "layer.h"
#include "log.h"
#include "pch.h"
#include "platform/opengl/imgui_opengl_renderer.h"

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
  io.DisplaySize = ImVec2(app.GetWindow()->GetWidth(), app.GetWindow()->GetHeight());

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

void ImGuiLayer::OnEvent(Event& e) {}
}  // namespace ck
