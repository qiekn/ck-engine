#include <ck.h>

#include <memory>

#include "core/entry_point.h"  // IWYU pragma: keep
#include "editor_layer.h"

namespace ck {

class Editor : public ck::Application {
public:
  Editor() { PushLayer(std::make_unique<EditorLayer>()); }

  ~Editor() {}
};
}  // namespace ck

MAKE_APPLICATION(Editor)
