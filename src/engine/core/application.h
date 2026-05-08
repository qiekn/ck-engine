#pragma once

#include <chrono>
#include <string>

#include "core/layer_stack.h"
#include "core/window.h"
#include "deltatime.h"
#include "events/application_event.h"

namespace ck {

class Renderer;
class ImGuiLayer;

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

  // Forwarded to Renderer; clients (editor's ViewportPanel) call this with
  // ImGui::GetContentRegionAvail so the offscreen target + camera follow
  // the panel size instead of the swapchain's.
  void OnViewportResize(uint32_t width, uint32_t height);

  void PushLayer(Scope<Layer> layer);
  void PushOverlay(Scope<Layer> layer);

  inline auto& GetWindow() { return window_; }
  Renderer& GetRenderer() { return *renderer_; }
  ImGuiLayer& GetImGuiLayer() { return *imgui_layer_; }

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
  Scope<Renderer> renderer_;  // depends on window_; declared after so it dies first
  LayerStack layer_stack_;
  ImGuiLayer* imgui_layer_ = nullptr;  // raw observer; ownership lives in layer_stack_
  DeltaTime timestep_;
  std::chrono::time_point<std::chrono::steady_clock> last_frame_time_ =
      std::chrono::steady_clock::now();

  static Application* instance_;
};

// To be defined in CLIENT
extern Application* CreateApplication(ApplicationCommandLineArgs args);

// Engine-provided main-loop driver; clients call this from main().
int EntryPoint(int argc, char** argv);
}  // namespace ck

#define MAKE_APPLICATION(ClassName)                                             \
  ck::Application* ck::CreateApplication(ck::ApplicationCommandLineArgs args) { \
    return new ClassName(args);                                                 \
  }