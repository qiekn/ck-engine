#pragma once

#include <memory>

#include "events/application_event.h"
#include "imgui/imgui_layer.h"
#include "layer.h"
#include "layer_stack.h"
#include "renderer/shader.h"
#include "renderer/vertex_array.h"
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
  bool running_ = true;
  std::unique_ptr<Window> window_;
  ImGuiLayer* imgui_layer_;
  LayerStack layer_stack_;

  std::shared_ptr<VertexArray> vertex_array_;
  std::unique_ptr<Shader> shader_;

  std::shared_ptr<VertexArray> square_va_;
  std::unique_ptr<Shader> blue_shader_;

  static Application* instance_;
};

// To be defined in CLIENT
extern Application* CreateApplication();
}  // namespace ck

#define MAKE_APPLICATION(ClassName) \
  ck::Application* ck::CreateApplication() { return new ClassName(); }
