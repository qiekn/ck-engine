#pragma once

#include <chrono>
#include <string>

#include "core/layer_stack.h"
#include "core/window.h"
#include "deltatime.h"
#include "events/application_event.h"

namespace ck {

struct ApplicationCommandLineArgs {
  int count = 0;
  char** args = nullptr;

  const char* operator[](int index) const { return args[index]; }
};

struct ApplicationSpecification {
  std::string name = "CK Engine";
  std::string working_directory;
  ApplicationCommandLineArgs command_line_args;
};

class Application {
public:
  explicit Application(const ApplicationSpecification& spec);
  virtual ~Application();

  void Run();
  void Close();

  void OnEvent(Event& e);

  void PushLayer(Scope<Layer> layer);
  void PushOverlay(Scope<Layer> layer);

  inline auto& GetWindow() { return window_; }

  inline static Application& Get() { return *instance_; }

  const ApplicationSpecification& GetSpecification() const { return specification_; }

private:
  bool OnWindowCloseEvent(WindowCloseEvent& e);
  bool OnWindowResizeEvent(WindowResizeEvent& e);

private:
  ApplicationSpecification specification_;
  bool running_ = true;
  bool minimized_ = false;
  Scope<Window> window_;
  LayerStack layer_stack_;
  DeltaTime timestep_;
  std::chrono::time_point<std::chrono::steady_clock> last_frame_time_ =
      std::chrono::steady_clock::now();

  static Application* instance_;
};

// To be defined in CLIENT
extern Application* CreateApplication(ApplicationCommandLineArgs args);
}  // namespace ck

#define MAKE_APPLICATION(ClassName)                                             \
  ck::Application* ck::CreateApplication(ck::ApplicationCommandLineArgs args) { \
    return new ClassName(args);                                                 \
  }