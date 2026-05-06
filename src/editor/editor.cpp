#include <ck.h>

#include "core/entry_point.h"  // IWYU pragma: keep

namespace ck {

class Editor : public Application {
public:
  explicit Editor(const ApplicationSpecification& spec) : Application(spec) {}
  ~Editor() {}
};

}  // namespace ck

ck::Application* ck::CreateApplication(ck::ApplicationCommandLineArgs args) {
  ck::ApplicationSpecification spec;
  spec.name = "CK Editor";
  spec.command_line_args = args;
  return new ck::Editor(spec);
}