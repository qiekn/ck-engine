#pragma once

#include "core/layer.h"

namespace ck {
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
