#pragma once

#include <chrono>

#include "core/deltatime.h"
#include "events/application_event.h"
#include "imgui/imgui_layer.h"
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

  void PushLayer(Scope<Layer> layer);
  void PushOverlay(Scope<Layer> layer);

  inline auto& GetWindow() { return window_; }

  inline static Application& Get() { return *instance_; }

private:
  bool OnWindowCloseEvent(WindowCloseEvent& e);

private:
  bool running_ = true;
  Scope<Window> window_;
  ImGuiLayer* imgui_layer_;
  LayerStack layer_stack_;
  DeltaTime timestep_;
  std::chrono::time_point<std::chrono::steady_clock> last_frame_time_ =
      std::chrono::steady_clock::now();

  static Application* instance_;
};

// To be defined in CLIENT
extern Application* CreateApplication();
}  // namespace ck

#define MAKE_APPLICATION(ClassName) \
  ck::Application* ck::CreateApplication() { return new ClassName(); }
