#pragma once

#include <utility>
#include <entt.hpp>

#include "core/log.h"
#include "scene/scene.h"

namespace ck {

// Thin wrapper around (entt::entity, Scene*) so client code can manipulate
// entities through ck:: types without seeing entt directly. Templates are
// inline so `entity.AddComponent<MyComponent>()` instantiates against the
// scene's registry at the call site.
class Entity {
public:
  Entity() = default;
  Entity(entt::entity handle, Scene* scene) : handle_(handle), scene_(scene) {}

  template <typename T, typename... Args>
  T& AddComponent(Args&&... args) {
    CK_ASSERT(!HasComponent<T>(), "Entity already has component");
    return scene_->registry_.emplace<T>(handle_, std::forward<Args>(args)...);
  }

  template <typename T>
  T& GetComponent() {
    CK_ASSERT(HasComponent<T>(), "Entity has no such component");
    return scene_->registry_.get<T>(handle_);
  }

  template <typename T>
  bool HasComponent() const {
    return scene_->registry_.all_of<T>(handle_);
  }

  template <typename T>
  void RemoveComponent() {
    CK_ASSERT(HasComponent<T>(), "Entity has no such component");
    scene_->registry_.remove<T>(handle_);
  }

  operator bool()         const { return handle_ != entt::null && scene_ != nullptr; }
  operator entt::entity() const { return handle_; }
  operator uint32_t()     const { return static_cast<uint32_t>(handle_); }

  bool operator==(const Entity& other) const {
    return handle_ == other.handle_ && scene_ == other.scene_;
  }
  bool operator!=(const Entity& other) const { return !(*this == other); }

private:
  entt::entity handle_ = entt::null;
  Scene*       scene_  = nullptr;
};

}  // namespace ck
