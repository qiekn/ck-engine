#pragma once

#include "core/deltatime.h"
#include "entt.hpp"

namespace ck {
class Entity;

class Scene {
public:
  Scene();
  virtual ~Scene();

  Entity CreateEntity(const std::string& name = std::string());

  void OnUpdate(DeltaTime dt);
  void OnViewportResize(uint32_t width, uint32_t height);

private:
  entt::registry registry_;
  uint32_t viewport_width = 0, viewport_height = 0;

  friend class Entity;
};
}  // namespace ck
