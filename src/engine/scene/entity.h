#pragma once

#include <utility>
#include "entt.hpp"
#include "scene/scene.h"

namespace ck {
class Entity {
public:
  Entity() = default;

  Entity(entt::entity handle, Scene* scene) : entity_handle_(handle), scene_(scene) {}

  Entity(const Entity& other) = default;

  virtual ~Entity() = default;

  template <typename T, typename... Args>
  T& AddComponent(Args&&... args) {
    CK_ENGINE_ASSERT(!HasComponent<T>(), "Entity already has component!");
    return scene_->registry_.emplace<T>(entity_handle_, std::forward<Args>(args)...);
  }

  template <typename T>
  T& GetComponent() {
    CK_ENGINE_ASSERT(HasComponent<T>(), "Entity  does not have this component!");
    return scene_->registry_.get<T>(entity_handle_);
  }

  template <typename T>
  bool HasComponent() {
    return scene_->registry_.any_of<T>(entity_handle_);
  }

  template <typename T>
  void RemoveComponent() {
    CK_ENGINE_ASSERT(HasComponent<T>(), "Entity does not have this component!");
    scene_->registry_.remove<T>(entity_handle_);
  }

  explicit operator bool() const { return entity_handle_ != entt::null; }

private:
  entt::entity entity_handle_{0};
  Scene* scene_{nullptr};
};
}  // namespace ck
