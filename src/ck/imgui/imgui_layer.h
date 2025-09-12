#pragma once

#include "events/event.h"
#include "layer.h"

namespace ck {
class ImGuiLayer : public Layer {
public:
  ImGuiLayer();
  virtual ~ImGuiLayer();

  void OnAttach() override;
  void OnDetach() override;
  void OnUpdate() override;
  void OnEvent(Event& e) override;

private:
  float time_{0.0f};
};
}  // namespace ck
