#include "application.h"

#include <filesystem>

#include "core/deltatime.h"
#include "core/log.h"
#include "events/application_event.h"
#include "events/event.h"
#include "renderer/vulkan/context.h"

namespace ck {

Application* Application::instance_ = nullptr;

Application::Application(const ApplicationSpecification& spec) : specification_(spec) {
  CK_PROFILE_FUNCTION();
  CK_ENGINE_ASSERT(Application::instance_ == nullptr, "application already exists");
  instance_ = this;

  if (!specification_.working_directory.empty())
    std::filesystem::current_path(specification_.working_directory);

  window_ = Window::Create(WindowProps(specification_.name));
  window_->SetEventCallback(CK_BIND_EVENT(Application::OnEvent));

  vk_context_ = CreateScope<vulkan::Context>(*window_);
}

Application::~Application() {
  CK_PROFILE_FUNCTION();
}

void Application::Run() {
  CK_PROFILE_FUNCTION();
  while (running_) {
    CK_PROFILE_SCOPE("RunLoop");
    auto time = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration<float>(time - last_frame_time_).count();
    auto timestep = DeltaTime(diff);

    if (!minimized_) {
      CK_PROFILE_SCOPE("LayerStack OnUpdate");
      for (auto& layer : layer_stack_) {
        layer->OnUpdate(timestep);
      }
    }

    window_->OnUpdate();
    last_frame_time_ = time;
  }
}

void Application::Close() {
  running_ = false;
}

void Application::OnEvent(Event& e) {
  CK_PROFILE_FUNCTION();
  auto dispatcher = EventDispatcher(e);
  dispatcher.DispatchEvent<WindowCloseEvent>(CK_BIND_EVENT(Application::OnWindowCloseEvent));
  dispatcher.DispatchEvent<WindowResizeEvent>(CK_BIND_EVENT(Application::OnWindowResizeEvent));

  for (auto it = layer_stack_.rbegin(); it != layer_stack_.rend(); it++) {
    if (e.IsHandled()) break;
    (*it)->OnEvent(e);
  }
}

bool Application::OnWindowCloseEvent(WindowCloseEvent& e) {
  running_ = false;
  return true;
}

bool Application::OnWindowResizeEvent(WindowResizeEvent& e) {
  CK_PROFILE_FUNCTION();
  if (e.GetWindowHeight() == 0 || e.GetWindowWidth() == 0) {
    minimized_ = true;
    return false;
  }
  minimized_ = false;
  return false;
}

void Application::PushLayer(Scope<Layer> layer) {
  CK_PROFILE_FUNCTION();
  layer->OnAttach();
  layer_stack_.push_layer(std::move(layer));
}

void Application::PushOverlay(Scope<Layer> layer) {
  CK_PROFILE_FUNCTION();
  layer->OnAttach();
  layer_stack_.push_overlay(std::move(layer));
}

}  // namespace ck