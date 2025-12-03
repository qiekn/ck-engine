#pragma once

#include "core/deltatime.h"
#include "entt.hpp"

namespace ck {
class Scene {
public:
  Scene();
  virtual ~Scene();

  entt::entity CreateEntity();

  // TEMP Getter
  entt::registry& Reg() { return registry_; }

  void OnUpdate(DeltaTime dt);

private:
  entt::registry registry_;
};
}  // namespace ck
