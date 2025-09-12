#pragma once

#include "events/application_event.h"
#include "layer.h"
#include "layer_stack.h"
#include "window.h"

namespace ck {
class Application {
public:
  Application();
  virtual ~Application();

  void Run();

  void OnEvent(Event& e);

  void PushLayer(std::unique_ptr<Layer> layer);
  void PushOverlay(std::unique_ptr<Layer> layer);

  inline auto& GetWindow() { return window_; }

  inline static Application& Get() { return *instance_; }

private:
  bool OnWindowCloseEvent(WindowCloseEvent& e);

private:
  std::unique_ptr<Window> window_;
  bool running_ = true;
  LayerStack layer_stack_;

  static Application* instance_;
};

// To be defined in CLIENT
extern Application* CreateApplication();
}  // namespace ck

#define MAKE_APPLICATION(ClassName) \
  ck::Application* ck::CreateApplication() { return new ClassName(); }
