#include <ck.h>

#include "core/entry_point.h"  // IWYU pragma: keep

class Sandbox : public ck::Application {
public:
  explicit Sandbox(const ck::ApplicationSpecification& spec) : Application(spec) {}
  ~Sandbox() {}
};

ck::Application* ck::CreateApplication(ck::ApplicationCommandLineArgs args) {
  ck::ApplicationSpecification spec;
  spec.name = "Sandbox";
  spec.command_line_args = args;
  return new Sandbox(spec);
}