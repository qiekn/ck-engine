#pragma once

#include "core/layer.h"
#include "imgui_internal.h"

namespace ck {

inline void HideDockNodeTabBar() {
  if (ImGui::IsWindowDocked()) {
    ImGuiDockNode* node = ImGui::GetWindowDockNode();
    if (node) {
      if (node->Windows.Size == 1)
        node->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
      else
        node->LocalFlags &= ~ImGuiDockNodeFlags_NoTabBar;
    }
  }
}

class ImGuiLayer : public Layer {
public:
  ImGuiLayer();
  virtual ~ImGuiLayer();

  void OnAttach() override;
  void OnDetach() override;
  void OnEvent(Event& e) override;
  void OnImGuiRender() override;

  void Begin();
  void End();

  void BlockEvent(bool block) { is_block_events = block; }

  void SetDarkThemeColors();

private:
  bool is_block_events = true;
};
}  // namespace ck
