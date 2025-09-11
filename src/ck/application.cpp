#include "application.h"

#include "events/application_event.h"
#include "log.h"

namespace ck {
Application::Application() {}

Application::~Application() {}

void Application::Run() {
  WindowResizeEvent e(1280, 720);
  CK_ENGINE_TRACE(e.ToString());
  while (true);
}
}  // namespace ck
