#include "application.h"

namespace ck {
Application::Application() { window_ = std::unique_ptr<Window>(Window::Create()); }

Application::~Application() {}

void Application::Run() {
  while (running_) {
    window_->OnUpdate();
  }
}
}  // namespace ck
