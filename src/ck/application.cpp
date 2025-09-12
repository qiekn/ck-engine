#include "application.h"

#include "events/application_event.h"
#include "events/event.h"

namespace ck {
#define CK_BIND_EVENT(fn)                                   \
  [this](auto&&... args) -> decltype(auto) {                \
    return this->fn(std::forward<decltype(args)>(args)...); \
  }

Application::Application() {
  window_ = std::unique_ptr<Window>(Window::Create());
  window_->SetEventCallback(CK_BIND_EVENT(Application::OnEvent));
}

Application::~Application() {}

void Application::Run() {
  while (running_) {
    for (auto& layer : layer_stack_) {
      layer->OnUpdate();
    }
    window_->OnUpdate();
  }
}

void Application::OnEvent(Event& e) {
  auto dispatcher = EventDispatcher(e);
  dispatcher.DispatchEvent<WindowCloseEvent>(CK_BIND_EVENT(Application::OnWindowCloseEvent));

  for (auto it = layer_stack_.end(); it != layer_stack_.begin();) {
    (*--it)->OnEvent(e);
    if (e.Handled()) {
      break;
    }
  }
}

bool Application::OnWindowCloseEvent(WindowCloseEvent& e) {
  running_ = false;
  return true;
}

void Application::PushLayer(std::unique_ptr<Layer> layer) {
  layer_stack_.push_layer(std::move(layer));
}

void Application::PushOverlay(std::unique_ptr<Layer> layer) {
  layer_stack_.push_overlay(std::move(layer));
}
}  // namespace ck
