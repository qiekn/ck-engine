#pragma once

#include <filesystem>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

#include "core/core.h"

namespace ck::vulkan {

class Allocator;
class Context;

// RAII over (VkImage, VkImageView, VmaAllocation). Layout transitions are
// the caller's responsibility for raw construction; the FromFile helper
// finishes its upload by transitioning to ShaderReadOnlyOptimal.
class Image {
public:
  struct CreateInfo {
    vk::Format format;
    vk::Extent2D extent;
    vk::ImageUsageFlags usage;
    uint32_t mip_levels = 1;
  };

  Image(Context& ctx, Allocator& alloc, const CreateInfo& info);
  ~Image();

  Image(const Image&) = delete;
  Image& operator=(const Image&) = delete;
  Image(Image&&) = delete;
  Image& operator=(Image&&) = delete;

  vk::Image handle() const { return image_; }
  vk::ImageView view() const { return view_; }
  vk::Format format() const { return format_; }
  vk::Extent2D extent() const { return extent_; }

  // Load a 2D 8-bit-RGBA image from disk via stb_image; uploads bytes via
  // a staging Buffer + Allocator::ImmediateSubmit; final layout is
  // ShaderReadOnlyOptimal. Returns nullptr if stb_image fails.
  static Scope<Image> FromFile(Context& ctx, Allocator& alloc,
                               const std::filesystem::path& path);

private:
  vk::Device device_;
  VmaAllocator vma_ = VK_NULL_HANDLE;       // borrowed
  VmaAllocation alloc_handle_ = VK_NULL_HANDLE;
  vk::Image image_;
  vk::ImageView view_;
  vk::Format format_ = vk::Format::eUndefined;
  vk::Extent2D extent_;
};

}  // namespace ck::vulkan