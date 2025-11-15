#pragma once

#include "core/layer.h"

namespace ck {
class ImGuiLayer : public Layer {
public:
  ImGuiLayer();
  virtual ~ImGuiLayer();

  void OnAttach() override;
  void OnDetach() override;
  void OnImGuiRender() override;

  void Begin();
  void End();
};
}  // namespace ck
