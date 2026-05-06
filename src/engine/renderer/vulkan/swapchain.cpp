#include "swapchain.h"

#include "context.h"

#include <volk.h>

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstdint>
#include <vector>

#include "core/log.h"
#include "core/window.h"
#include "debug/profiler.h"

namespace ck::vulkan {

namespace {

vk::SurfaceFormatKHR PickFormat(const std::vector<vk::SurfaceFormatKHR>& formats) {
  for (auto const& f : formats) {
    if (f.format == vk::Format::eB8G8R8A8Srgb &&
        f.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return f;
    }
  }
  return formats.front();
}

vk::PresentModeKHR PickPresentMode(const std::vector<vk::PresentModeKHR>& modes, bool vsync) {
  if (vsync) return vk::PresentModeKHR::eFifo;  // always supported
  for (auto m : modes) {
    if (m == vk::PresentModeKHR::eMailbox) return m;
  }
  return vk::PresentModeKHR::eFifo;
}

vk::Extent2D PickExtent(const vk::SurfaceCapabilitiesKHR& caps, GLFWwindow* glfw_win) {
  if (caps.currentExtent.width != UINT32_MAX) return caps.currentExtent;
  int w = 0, h = 0;
  glfwGetFramebufferSize(glfw_win, &w, &h);
  vk::Extent2D extent;
  extent.width = std::clamp<uint32_t>(static_cast<uint32_t>(w),
                                      caps.minImageExtent.width, caps.maxImageExtent.width);
  extent.height = std::clamp<uint32_t>(static_cast<uint32_t>(h),
                                       caps.minImageExtent.height, caps.maxImageExtent.height);
  return extent;
}

}  // namespace

Swapchain::Swapchain(Context& ctx, Window& window) : ctx_(ctx), window_(window) {
  CK_PROFILE_FUNCTION();
  Create();
}

Swapchain::~Swapchain() {
  CK_PROFILE_FUNCTION();
  Destroy();
}

void Swapchain::Recreate() {
  CK_PROFILE_FUNCTION();
  ctx_.device().waitIdle();
  Destroy();
  Create();
}

void Swapchain::Create() {
  vk::PhysicalDevice pd = ctx_.physical_device();
  vk::Device dev = ctx_.device();
  vk::SurfaceKHR surface = ctx_.surface();

  auto caps = pd.getSurfaceCapabilitiesKHR(surface);
  auto formats = pd.getSurfaceFormatsKHR(surface);
  auto modes = pd.getSurfacePresentModesKHR(surface);

  auto* glfw_win = static_cast<GLFWwindow*>(window_.GetNativeWindow());
  vk::SurfaceFormatKHR sf = PickFormat(formats);
  vk::PresentModeKHR pm = PickPresentMode(modes, window_.IsVSync());
  extent_ = PickExtent(caps, glfw_win);
  format_ = sf.format;

  uint32_t image_count = caps.minImageCount + 1;
  if (caps.maxImageCount > 0 && image_count > caps.maxImageCount) {
    image_count = caps.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR sci{};
  sci.surface = surface;
  sci.minImageCount = image_count;
  sci.imageFormat = sf.format;
  sci.imageColorSpace = sf.colorSpace;
  sci.imageExtent = extent_;
  sci.imageArrayLayers = 1;
  sci.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
  sci.imageSharingMode = vk::SharingMode::eExclusive;  // single graphics+present queue
  sci.preTransform = caps.currentTransform;
  sci.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
  sci.presentMode = pm;
  sci.clipped = VK_TRUE;
  sci.oldSwapchain = nullptr;

  swapchain_ = dev.createSwapchainKHR(sci);
  images_ = dev.getSwapchainImagesKHR(swapchain_);

  image_views_.reserve(images_.size());
  for (auto img : images_) {
    vk::ImageViewCreateInfo ivi{};
    ivi.image = img;
    ivi.viewType = vk::ImageViewType::e2D;
    ivi.format = format_;
    ivi.components = vk::ComponentMapping{};
    ivi.subresourceRange = vk::ImageSubresourceRange{
        vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
    image_views_.push_back(dev.createImageView(ivi));
  }

  CK_ENGINE_INFO("Swapchain: {}x{}, {} images, format={}, present={}",
                 extent_.width, extent_.height, images_.size(),
                 vk::to_string(format_), vk::to_string(pm));
}

void Swapchain::Destroy() {
  vk::Device dev = ctx_.device();
  for (auto v : image_views_) dev.destroyImageView(v);
  image_views_.clear();
  images_.clear();
  if (swapchain_) {
    dev.destroySwapchainKHR(swapchain_);
    swapchain_ = nullptr;
  }
}

}  // namespace ck::vulkan