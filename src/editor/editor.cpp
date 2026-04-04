#include <ck.h>

#include <memory>

#include "core/entry_point.h"  // IWYU pragma: keep
#include "editor_layer.h"

namespace ck {

class Editor : public ck::Application {
public:
  explicit Editor(const ApplicationSpecification& spec) : Application(spec) {
    PushLayer(std::make_unique<EditorLayer>());
  }

  ~Editor() {}
};
}  // namespace ck

ck::Application* ck::CreateApplication(ck::ApplicationCommandLineArgs args) {
  ck::ApplicationSpecification spec;
  spec.name = "CK Editor";
  spec.command_line_args = args;
  return new ck::Editor(spec);
}
