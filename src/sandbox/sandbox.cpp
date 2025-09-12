#include <ck.h>

#include "log.h"

class ExampleLayer : public ck::Layer {
public:
  ExampleLayer() : Layer("Example") {}

  void OnEvent(ck::Event& event) override { CK_CLIENT_INFO(event.ToString()); }
};

class Sandbox : public ck::Application {
public:
  Sandbox() { PushLayer(std::make_unique<ExampleLayer>()); }
  ~Sandbox() {}
};

MAKE_APPLICATION(Sandbox)
