#pragma once

#include "core/deltatime.h"
#include "events/event.h"
#include "pch.h"

namespace ck {
class Layer {
public:
  Layer(const std::string& debug_name = "Layer") : debug_name_(debug_name) {}
  virtual ~Layer() {}

  virtual void OnAttach() {}
  virtual void OnDetach() {}
  virtual void OnUpdate(DeltaTime) {}
  virtual void OnEvent(Event& e) {}
  virtual void OnImGuiRender() {}

  const std::string& GetName() const { return debug_name_; }

protected:
  std::string debug_name_;
};
}  // namespace ck
