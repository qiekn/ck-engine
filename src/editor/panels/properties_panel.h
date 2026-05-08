#pragma once

// Same `import ck;`-in-header pattern as SceneHierarchyPanel: needed
// because OnImGuiRender takes ck::Entity by value.
import ck;

namespace ck_editor {

// Inspects + edits components on the entity passed in each frame
// (typically `hierarchy_panel.SelectedEntity()`). Stateless: caller owns
// the selection. Renders Tag (rename), Transform (T/R/S as DragFloat3),
// SpriteRenderer (color picker).
class PropertiesPanel {
public:
  void OnImGuiRender(ck::Entity selected);
};

}  // namespace ck_editor
