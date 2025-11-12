#include "application.h"

#include <memory>

#include "core/deltatime.h"
#include "events/application_event.h"
#include "events/event.h"
#include "glad/gl.h"
#include "imgui/imgui_layer.h"
#include "log.h"

namespace ck {

Application* Application::instance_ = nullptr;

Application::Application() {
  CK_ENGINE_ASSERT(Application::instance_ == nullptr, "application already exists");
  instance_ = this;
  window_ = Window::Create();
  window_->SetEventCallback(CK_BIND_EVENT(Application::OnEvent));

  auto imgui_layer = std::make_unique<ImGuiLayer>();
  imgui_layer_ = imgui_layer.get();  // 只要我们不 PopOverlay, imgui_layer_ 就不会悬空
  PushOverlay(std::move(imgui_layer));
}

Application::~Application() {}

void Application::Run() {
  while (running_) {
    auto time = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration<float>(time - last_frame_time_).count();
    auto timestep = DeltaTime(diff);
    for (auto& layer : layer_stack_) {
      layer->OnUpdate(timestep);
    }

    imgui_layer_->Begin();  // this is dirty, but it works
    for (auto& layer : layer_stack_) {
      layer->OnImGuiRender();
    }
    imgui_layer_->End();

    window_->OnUpdate();
    last_frame_time_ = time;
  }
}

void Application::OnEvent(Event& e) {
  auto dispatcher = EventDispatcher(e);
  dispatcher.DispatchEvent<WindowCloseEvent>(CK_BIND_EVENT(Application::OnWindowCloseEvent));

  for (auto it = layer_stack_.end(); it != layer_stack_.begin();) {
    (*--it)->OnEvent(e);
    if (e.IsHandled()) {
      break;
    }
  }
}

bool Application::OnWindowCloseEvent(WindowCloseEvent& e) {
  running_ = false;
  return true;
}

void Application::PushLayer(std::unique_ptr<Layer> layer) {
  layer->OnAttach();
  layer_stack_.push_layer(std::move(layer));
}

void Application::PushOverlay(std::unique_ptr<Layer> layer) {
  layer->OnAttach();
  layer_stack_.push_overlay(std::move(layer));
}
}  // namespace ck
