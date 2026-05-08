#pragma once

// Unlike StatsPanel/ViewportPanel, this panel needs ck::Ref / ck::Scene /
// ck::Entity in its public surface (context_ + selected_ are by-value
// members). `import ck;` here keeps the engine-internal scene/* headers
// out of the editor target's include search path.
import ck;

namespace ck_editor {

// Lists every entity in the bound Scene as a Selectable. Click selects;
// PropertiesPanel (6.B.3) will read SelectedEntity() to decide what to
// inspect. Right-click context menus cover create + delete.
class SceneHierarchyPanel {
public:
  SceneHierarchyPanel() = default;
  explicit SceneHierarchyPanel(const ck::Ref<ck::Scene>& scene) { SetContext(scene); }

  void SetContext(const ck::Ref<ck::Scene>& scene) {
    context_ = scene;
    selected_ = {};
  }

  void OnImGuiRender();

  ck::Entity SelectedEntity() const { return selected_; }
  void SetSelectedEntity(ck::Entity entity) { selected_ = entity; }

private:
  void DrawEntityNode(ck::Entity entity);

  ck::Ref<ck::Scene> context_;
  ck::Entity selected_;
};

}  // namespace ck_editor
