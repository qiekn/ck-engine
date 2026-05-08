#pragma once

namespace ck_editor {

// Hosts the engine's offscreen color_target as ImGui::Image and reports
// the panel's content-region size to Application::OnViewportResize so
// color_target_ + camera follow the panel (phase 6.A.3.5 wire). Owned by
// EditorLayer.
//
// IsHovered / IsFocused expose the panel's last-frame ImGui state so
// EditorLayer can gate EditorCamera input (only pan/zoom when the user is
// actually pointing at the scene, not at the dockspace chrome).
class ViewportPanel {
public:
  void OnImGuiRender();

  bool IsHovered() const { return hovered_; }
  bool IsFocused() const { return focused_; }

private:
  bool hovered_ = false;
  bool focused_ = false;
};

}  // namespace ck_editor
