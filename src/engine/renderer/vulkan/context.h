#pragma once

#include "core/core.h"

namespace ck {
class Window;
}

namespace ck::vulkan {

// Owns the Vulkan instance / debug messenger / surface / device / queue.
// All vk:: types are kept hidden behind Pimpl so engine consumers don't pull
// in <vulkan/vulkan.hpp>.
class Context {
public:
  explicit Context(Window& window);
  ~Context();

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;
  Context(Context&&) = delete;
  Context& operator=(Context&&) = delete;

private:
  struct Impl;
  Scope<Impl> impl_;
};

}  // namespace ck::vulkan