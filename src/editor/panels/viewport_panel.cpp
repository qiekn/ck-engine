#include "panels/viewport_panel.h"

import ck;

namespace ck_editor {

void ViewportPanel::OnImGuiRender() {
  // WindowPadding=0: the Image fills the panel content area edge-to-edge.
  // The default 8px padding (ScaleAllSizes(dpi=1.5) -> 12px) shows the
  // panel BG through as a "black border" around the rendering otherwise.
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("Viewport");

  ImVec2 size = ImGui::GetContentRegionAvail();
  if (size.x > 0.0f && size.y > 0.0f) {
    ck::Application::Get().OnViewportResize(static_cast<unsigned int>(size.x),
                                            static_cast<unsigned int>(size.y));
  }
  ImTextureID tex = ck::Application::Get().GetImGuiLayer().viewport_texture_id();
  if (tex) ImGui::Image(tex, size);

  ImGui::End();
  ImGui::PopStyleVar();
}

}  // namespace ck_editor
