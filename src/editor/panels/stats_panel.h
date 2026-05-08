#pragma once

namespace ck_editor {

// Floating diagnostics panel: FPS + Renderer2D batcher counters. Owned by
// EditorLayer; OnImGuiRender is invoked between dockspace setup and the
// other panels each frame.
class StatsPanel {
public:
  void OnImGuiRender();
};

}  // namespace ck_editor
