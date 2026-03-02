#include <ck.h>

#include <memory>

#include "core/entry_point.h"  // IWYU pragma: keep
#include "sandbox_2d.h"

class Sandbox : public ck::Application {
public:
  explicit Sandbox(const ck::ApplicationSpecification& spec) : Application(spec) {
    PushLayer(std::make_unique<Sandbox2D>());
  }
  ~Sandbox() {}
};

ck::Application* ck::CreateApplication(ck::ApplicationCommandLineArgs args) {
  ck::ApplicationSpecification spec;
  spec.name = "Sandbox";
  spec.command_line_args = args;
  return new Sandbox(spec);
}
