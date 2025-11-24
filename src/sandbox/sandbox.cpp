#include <ck.h>

#include <memory>

#include "core/entry_point.h"  // IWYU pragma: keep
#include "sandbox_2d.h"

class Sandbox : public ck::Application {
public:
  Sandbox() {
    // PushLayer(std::make_unique<ExampleLayer>());
    PushLayer(std::make_unique<Sandbox2D>());
  }
  ~Sandbox() {}
};

MAKE_APPLICATION(Sandbox)
