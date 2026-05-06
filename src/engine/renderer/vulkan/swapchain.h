#pragma once

#include <span>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_to_string.hpp>

#include "core/core.h"

namespace ck {
class Window;
}

namespace ck::vulkan {

class Context;

class Swapchain {
public:
  Swapchain(Context& ctx, Window& window);
  ~Swapchain();

  Swapchain(const Swapchain&) = delete;
  Swapchain& operator=(const Swapchain&) = delete;
  Swapchain(Swapchain&&) = delete;
  Swapchain& operator=(Swapchain&&) = delete;

  // Tear down + recreate. Caller must ensure no in-flight work touches the old swapchain.
  void Recreate();

  vk::SwapchainKHR             handle()      const { return swapchain_; }
  vk::Format                   format()      const { return format_; }
  vk::Extent2D                 extent()      const { return extent_; }
  std::span<const vk::Image>     images()      const { return images_; }
  std::span<const vk::ImageView> image_views() const { return image_views_; }
  uint32_t                     image_count() const { return static_cast<uint32_t>(images_.size()); }

private:
  void Create();
  void Destroy();

  Context& ctx_;
  Window& window_;

  vk::SwapchainKHR swapchain_;
  std::vector<vk::Image> images_;
  std::vector<vk::ImageView> image_views_;
  vk::Format format_ = vk::Format::eUndefined;
  vk::Extent2D extent_;
};

}  // namespace ck::vulkan