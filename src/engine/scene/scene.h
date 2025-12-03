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

private:
  entt::registry registry_;

  friend class Entity;
};
}  // namespace ck
