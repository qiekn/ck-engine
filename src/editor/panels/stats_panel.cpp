#include "panels/stats_panel.h"

import ck;

namespace ck_editor {

void StatsPanel::OnImGuiRender() {
  ImGui::Begin("Stats");
  ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
  auto s = ck::Renderer2D::stats();
  ImGui::Text("Quads: %u", s.quad_count);
  ImGui::Text("Textures: %u", s.texture_count);
  ImGui::Text("Draw calls: %u", s.draw_calls);
  ImGui::End();
}

}  // namespace ck_editor
