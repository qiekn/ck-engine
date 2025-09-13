#pragma once

#include "events/application_event.h"
#include "events/event.h"
#include "events/key_event.h"
#include "events/mouse_event.h"
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
  bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& e);
  bool OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& e);
  bool OnMouseMoveEvent(MouseMoveEvent& e);
  bool OnMouseScrollEvent(MouseScrollEvent& e);
  bool OnKeyPressedEvent(KeyPressedEvent& e);
  bool OnKeyReleasedEvent(KeyReleasedEvent& e);
  bool OnKeyEvent(KeyEvent& e);
  bool OnWindowResizeEvent(WindowResizeEvent& e);

private:
  float time_{0.0f};
};
}  // namespace ck
