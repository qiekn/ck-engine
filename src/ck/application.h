#pragma once

namespace ck {
class Application {
public:
  Application();
  virtual ~Application();

  void Run();
};

// To be defined in CLIENT
extern Application* CreateApplication();
}  // namespace ck

#define MAKE_APPLICATION(ClassName)          \
  ck::Application* ck::CreateApplication() { \
    return new ClassName();                  \
  }
