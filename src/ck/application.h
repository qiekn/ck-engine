#pragma once

#include "window.h"

namespace ck {
class Application {
public:
  Application();
  virtual ~Application();

  void Run();

private:
  std::unique_ptr<Window> window_;
  bool running_ = true;
};

// To be defined in CLIENT
extern Application* CreateApplication();
}  // namespace ck

#define MAKE_APPLICATION(ClassName) \
  ck::Application* ck::CreateApplication() { return new ClassName(); }
