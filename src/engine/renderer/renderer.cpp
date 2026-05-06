#include "renderer.h"

#include "vulkan/context.h"
#include "vulkan/swapchain.h"

#include <cmath>
#include <numbers>

#include <volk.h>

#include "core/log.h"
#include "core/window.h"
#include "debug/profiler.h"

namespace ck {

namespace {

void TransitionImage(vk::CommandBuffer cmd,
                     vk::Image image,
                     vk::ImageLayout old_layout,
                     vk::ImageLayout new_layout,
                     vk::PipelineStageFlags2 src_stage,
                     vk::AccessFlags2 src_access,
                     vk::PipelineStageFlags2 dst_stage,
                     vk::AccessFlags2 dst_access) {
  vk::ImageMemoryBarrier2 barrier{};
  barrier.srcStageMask = src_stage;
  barrier.srcAccessMask = src_access;
  barrier.dstStageMask = dst_stage;
  barrier.dstAccessMask = dst_access;
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange = vk::ImageSubresourceRange{
      vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

  vk::DependencyInfo dep{};
  dep.imageMemoryBarrierCount = 1;
  dep.pImageMemoryBarriers = &barrier;
  cmd.pipelineBarrier2(dep);
}

}  // namespace

Renderer::Renderer(Window& window) : window_(window) {
  CK_PROFILE_FUNCTION();
  start_time_ = std::chrono::steady_clock::now();
  context_ = CreateScope<vulkan::Context>(window_);
  swapchain_ = CreateScope<vulkan::Swapchain>(*context_, window_);
  for (auto& f : frames_) f = CreateScope<vulkan::Frame>(*context_);

  vk::SemaphoreCreateInfo sem_ci{};
  render_finished_.resize(swapchain_->image_count());
  for (auto& s : render_finished_) s = context_->device().createSemaphore(sem_ci);
}

Renderer::~Renderer() {
  CK_PROFILE_FUNCTION();
  if (context_ && context_->device()) {
    context_->device().waitIdle();
    for (auto s : render_finished_) context_->device().destroySemaphore(s);
  }
}

void Renderer::OnResize(uint32_t /*width*/, uint32_t /*height*/) {
  resize_pending_ = true;
}

void Renderer::RecreateSwapchain() {
  CK_PROFILE_FUNCTION();
  // Skip if window is minimized — Application's run loop already skips render in that case,
  // but acquire/present can still trigger us before the minimize event lands.
  if (window_.GetWidth() == 0 || window_.GetHeight() == 0) return;

  vk::Device dev = context_->device();
  dev.waitIdle();

  // Image count may change across recreate -> rebuild render_finished_.
  for (auto s : render_finished_) dev.destroySemaphore(s);
  render_finished_.clear();

  swapchain_->Recreate();

  vk::SemaphoreCreateInfo sem_ci{};
  render_finished_.resize(swapchain_->image_count());
  for (auto& s : render_finished_) s = dev.createSemaphore(sem_ci);

  resize_pending_ = false;
}

void Renderer::BeginFrame() {
  CK_PROFILE_FUNCTION();
  if (resize_pending_) RecreateSwapchain();

  vk::Device dev = context_->device();
  vulkan::Frame& fr = *frames_[current_frame_];

  // Wait for previous use of this slot to finish; do NOT reset fence yet, so we can
  // bail safely on OutOfDate without leaving the fence unsignalled.
  (void)dev.waitForFences(fr.in_flight(), VK_TRUE, UINT64_MAX);

  uint32_t image_index = 0;
  try {
    auto r = dev.acquireNextImageKHR(swapchain_->handle(), UINT64_MAX,
                                     fr.image_available(), nullptr);
    if (r.result == vk::Result::eSuboptimalKHR) resize_pending_ = true;
    image_index = r.value;
  } catch (const vk::OutOfDateKHRError&) {
    RecreateSwapchain();
    return;  // skip this frame; fence stays signalled
  }
  image_index_ = image_index;
  dev.resetFences(fr.in_flight());

  vk::CommandBuffer cmd = fr.command_buffer();
  cmd.reset();
  vk::CommandBufferBeginInfo begin{};
  begin.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
  cmd.begin(begin);

  TransitionImage(cmd, swapchain_->images()[image_index_],
                  vk::ImageLayout::eUndefined,
                  vk::ImageLayout::eColorAttachmentOptimal,
                  vk::PipelineStageFlagBits2::eTopOfPipe, {},
                  vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                  vk::AccessFlagBits2::eColorAttachmentWrite);

  // Time-based RGB cycle: each channel is a sine offset by 120 degrees.
  float t = std::chrono::duration<float>(
      std::chrono::steady_clock::now() - start_time_).count();
  constexpr float kTwoThirdsPi = 2.0f * std::numbers::pi_v<float> / 3.0f;
  vk::ClearColorValue clear_color{std::array<float, 4>{
      0.5f + 0.5f * std::sin(t),
      0.5f + 0.5f * std::sin(t + kTwoThirdsPi),
      0.5f + 0.5f * std::sin(t + 2.0f * kTwoThirdsPi),
      1.0f}};

  vk::RenderingAttachmentInfo color_att{};
  color_att.imageView = swapchain_->image_views()[image_index_];
  color_att.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
  color_att.loadOp = vk::AttachmentLoadOp::eClear;
  color_att.storeOp = vk::AttachmentStoreOp::eStore;
  color_att.clearValue.color = clear_color;

  vk::RenderingInfo rendering{};
  rendering.renderArea.offset = vk::Offset2D{0, 0};
  rendering.renderArea.extent = swapchain_->extent();
  rendering.layerCount = 1;
  rendering.colorAttachmentCount = 1;
  rendering.pColorAttachments = &color_att;
  cmd.beginRendering(rendering);

  frame_active_ = true;
}

void Renderer::EndFrame() {
  CK_PROFILE_FUNCTION();
  if (!frame_active_) return;
  frame_active_ = false;

  vulkan::Frame& fr = *frames_[current_frame_];
  vk::CommandBuffer cmd = fr.command_buffer();

  cmd.endRendering();
  TransitionImage(cmd, swapchain_->images()[image_index_],
                  vk::ImageLayout::eColorAttachmentOptimal,
                  vk::ImageLayout::ePresentSrcKHR,
                  vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                  vk::AccessFlagBits2::eColorAttachmentWrite,
                  vk::PipelineStageFlagBits2::eBottomOfPipe, {});
  cmd.end();

  vk::SemaphoreSubmitInfo wait_si{};
  wait_si.semaphore = fr.image_available();
  wait_si.stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;

  vk::SemaphoreSubmitInfo signal_si{};
  signal_si.semaphore = render_finished_[image_index_];
  signal_si.stageMask = vk::PipelineStageFlagBits2::eAllGraphics;

  vk::CommandBufferSubmitInfo cmd_si{};
  cmd_si.commandBuffer = cmd;

  vk::SubmitInfo2 submit{};
  submit.waitSemaphoreInfoCount = 1;
  submit.pWaitSemaphoreInfos = &wait_si;
  submit.commandBufferInfoCount = 1;
  submit.pCommandBufferInfos = &cmd_si;
  submit.signalSemaphoreInfoCount = 1;
  submit.pSignalSemaphoreInfos = &signal_si;

  context_->graphics_queue().submit2(submit, fr.in_flight());

  vk::PresentInfoKHR present{};
  present.waitSemaphoreCount = 1;
  vk::Semaphore wait_sem = render_finished_[image_index_];
  present.pWaitSemaphores = &wait_sem;
  present.swapchainCount = 1;
  vk::SwapchainKHR sc = swapchain_->handle();
  present.pSwapchains = &sc;
  present.pImageIndices = &image_index_;

  try {
    auto pres_res = context_->graphics_queue().presentKHR(present);
    if (pres_res == vk::Result::eSuboptimalKHR) resize_pending_ = true;
  } catch (const vk::OutOfDateKHRError&) {
    resize_pending_ = true;
  }

  current_frame_ = (current_frame_ + 1) % vulkan::kFramesInFlight;
}

}  // namespace ck