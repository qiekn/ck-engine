#include "application.h"

#include "events/application_event.h"
#include "events/event.h"
#include "log.h"

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
    window_->OnUpdate();
  }
}

void Application::OnEvent(Event& e) {
  auto dispatcher = EventDispatcher(e);
  CK_CLIENT_TRACE(e.ToString());
  dispatcher.DispatchEvent<WindowCloseEvent>(CK_BIND_EVENT(Application::OnWindowCloseEvent));
}

bool Application::OnWindowCloseEvent(WindowCloseEvent& e) {
  running_ = false;
  return true;
}
}  // namespace ck
