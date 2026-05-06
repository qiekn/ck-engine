#include "renderer/vulkan/image.h"

#include <cstring>

#include <stb_image.h>

#include "core/log.h"
#include "debug/profiler.h"
#include "renderer/vulkan/allocator.h"
#include "renderer/vulkan/buffer.h"
#include "renderer/vulkan/context.h"

namespace ck::vulkan {

namespace {

void TransitionImageLayout(vk::CommandBuffer cmd, vk::Image image,
                           vk::ImageLayout old_layout, vk::ImageLayout new_layout,
                           vk::PipelineStageFlags2 src_stage, vk::AccessFlags2 src_access,
                           vk::PipelineStageFlags2 dst_stage, vk::AccessFlags2 dst_access) {
  vk::ImageMemoryBarrier2 b{};
  b.srcStageMask = src_stage;
  b.srcAccessMask = src_access;
  b.dstStageMask = dst_stage;
  b.dstAccessMask = dst_access;
  b.oldLayout = old_layout;
  b.newLayout = new_layout;
  b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  b.image = image;
  b.subresourceRange = vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

  vk::DependencyInfo dep{};
  dep.imageMemoryBarrierCount = 1;
  dep.pImageMemoryBarriers = &b;
  cmd.pipelineBarrier2(dep);
}

}  // namespace

Image::Image(Context& ctx, Allocator& alloc, const CreateInfo& info)
    : device_(ctx.device()),
      vma_(alloc.handle()),
      format_(info.format),
      extent_(info.extent) {
  VkImageCreateInfo ici{};
  ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  ici.imageType = VK_IMAGE_TYPE_2D;
  ici.format = static_cast<VkFormat>(info.format);
  ici.extent = {info.extent.width, info.extent.height, 1};
  ici.mipLevels = info.mip_levels;
  ici.arrayLayers = 1;
  ici.samples = VK_SAMPLE_COUNT_1_BIT;
  ici.tiling = VK_IMAGE_TILING_OPTIMAL;
  ici.usage = static_cast<VkImageUsageFlags>(info.usage);
  ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  VmaAllocationCreateInfo aci{};
  aci.usage = VMA_MEMORY_USAGE_AUTO;

  VkImage raw{};
  if (vmaCreateImage(vma_, &ici, &aci, &raw, &alloc_handle_, nullptr) != VK_SUCCESS) {
    CK_ENGINE_FATAL("vmaCreateImage failed ({}x{})", info.extent.width, info.extent.height);
    return;
  }
  image_ = raw;

  vk::ImageViewCreateInfo vci{};
  vci.image = image_;
  vci.viewType = vk::ImageViewType::e2D;
  vci.format = format_;
  vci.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
  vci.subresourceRange.baseMipLevel = 0;
  vci.subresourceRange.levelCount = info.mip_levels;
  vci.subresourceRange.baseArrayLayer = 0;
  vci.subresourceRange.layerCount = 1;
  view_ = device_.createImageView(vci);
}

Image::~Image() {
  if (device_ && view_) device_.destroyImageView(view_);
  if (vma_ && image_) {
    vmaDestroyImage(vma_, static_cast<VkImage>(image_), alloc_handle_);
  }
}

Scope<Image> Image::FromFile(Context& ctx, Allocator& alloc,
                             const std::filesystem::path& path) {
  CK_PROFILE_FUNCTION();

  int w = 0;
  int h = 0;
  int channels = 0;
  stbi_uc* pixels = stbi_load(path.string().c_str(), &w, &h, &channels, STBI_rgb_alpha);
  if (!pixels) {
    CK_ENGINE_ERROR("stbi_load failed: {}", path.string());
    return nullptr;
  }
  vk::DeviceSize size = static_cast<vk::DeviceSize>(w) * static_cast<vk::DeviceSize>(h) * 4;

  Buffer staging(alloc, size, vk::BufferUsageFlagBits::eTransferSrc, BufferType::HostVisible);
  std::memcpy(staging.mapped(), pixels, size);
  stbi_image_free(pixels);

  CreateInfo info{};
  info.format = vk::Format::eR8G8B8A8Srgb;
  info.extent = vk::Extent2D{static_cast<uint32_t>(w), static_cast<uint32_t>(h)};
  info.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
  auto img = CreateScope<Image>(ctx, alloc, info);

  alloc.ImmediateSubmit([&](vk::CommandBuffer cmd) {
    TransitionImageLayout(cmd, img->handle(),
                          vk::ImageLayout::eUndefined,
                          vk::ImageLayout::eTransferDstOptimal,
                          vk::PipelineStageFlagBits2::eTopOfPipe, {},
                          vk::PipelineStageFlagBits2::eTransfer,
                          vk::AccessFlagBits2::eTransferWrite);

    vk::BufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = vk::Offset3D{0, 0, 0};
    region.imageExtent = vk::Extent3D{static_cast<uint32_t>(w), static_cast<uint32_t>(h), 1};
    cmd.copyBufferToImage(staging.handle(), img->handle(),
                          vk::ImageLayout::eTransferDstOptimal, region);

    TransitionImageLayout(cmd, img->handle(),
                          vk::ImageLayout::eTransferDstOptimal,
                          vk::ImageLayout::eShaderReadOnlyOptimal,
                          vk::PipelineStageFlagBits2::eTransfer,
                          vk::AccessFlagBits2::eTransferWrite,
                          vk::PipelineStageFlagBits2::eFragmentShader,
                          vk::AccessFlagBits2::eShaderRead);
  });

  return img;
}

}  // namespace ck::vulkan