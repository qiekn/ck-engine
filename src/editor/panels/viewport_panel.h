#pragma once

namespace ck_editor {

// Hosts the engine's offscreen color_target as ImGui::Image and reports
// the panel's content-region size to Application::OnViewportResize so
// color_target_ + camera follow the panel (phase 6.A.3.5 wire). Owned by
// EditorLayer.
class ViewportPanel {
public:
  void OnImGuiRender();
};

}  // namespace ck_editor
