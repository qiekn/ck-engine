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
    T& component = scene_->registry_.emplace<T>(entity_handle_, std::forward<Args>(args)...);
    // todo
    return component;
  }

  template <typename T>
  T& GetComponent() {
    CK_ENGINE_ASSERT(HasComponent<T>(), "Entity  does not have this component!");
    return scene_->registry_.get<T>(entity_handle_);
  }

  template <typename T>
  const T& GetComponent() const {
    CK_ENGINE_ASSERT(HasComponent<T>(), "Entity does not have this component!");
    return scene_->registry_.get<T>(entity_handle_);
  }

  template <typename T>
  bool HasComponent() const {
    return scene_->registry_.any_of<T>(entity_handle_);
  }

  template <typename T>
  void RemoveComponent() {
    CK_ENGINE_ASSERT(HasComponent<T>(), "Entity does not have this component!");
    scene_->registry_.remove<T>(entity_handle_);
  }

  explicit operator bool() const { return entity_handle_ != entt::null; }

  explicit operator entt::entity() const { return entity_handle_; }

  entt::entity GetID() const { return entity_handle_; }

  bool operator==(const Entity& other) const {
    return entity_handle_ == other.entity_handle_ && scene_ == other.scene_;
  }

  bool operator!=(const Entity& other) const { return !(*this == other); }

private:
  entt::entity entity_handle_{entt::null};
  Scene* scene_{nullptr};
};
}  // namespace ck
