#pragma once

#include "core/deltatime.h"
#include "scene/entity.h"

namespace ck {
class ScriptableEntity {
public:
  virtual ~ScriptableEntity() = default;

  template <typename T>
  T& GetComponent() {
    return entity_.GetComponent<T>();
  }

protected:
  virtual void OnCreate() {}

  virtual void OnDestroy() {}

  virtual void OnUpdate(DeltaTime dt) {}

private:
  Entity entity_;
  friend class Scene;
};
}  // namespace ck
