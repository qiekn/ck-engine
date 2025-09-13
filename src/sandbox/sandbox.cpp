#include <ck.h>

#include "application.h"
#include "imgui.h"
#include "log.h"

class ExampleLayer : public ck::Layer {
public:
  ExampleLayer() : Layer("Example") {}
  // void OnEvent(ck::Event& e) override { CK_ENGINE_INFO(e.ToString()); }
  void OnImGuiRender() override {
    ImGui::Begin("Test");
    ImGui::TextUnformatted("Hello");
    ImGui::End();
  }
};

class Sandbox : public ck::Application {
public:
  Sandbox() { PushLayer(std::make_unique<ExampleLayer>()); }
  ~Sandbox() {}
};

MAKE_APPLICATION(Sandbox)
