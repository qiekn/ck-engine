#include "renderer_2d.h"

#include <array>
#include <cstring>
#include <span>
#include <vector>

#include <volk.h>

#include "core/core.h"
#include "core/log.h"
#include "debug/profiler.h"
#include "renderer/camera.h"
#include "renderer/shader/graphics_pipeline.h"
#include "renderer/shader/shader_module.h"
#include "renderer/shader/slang_compiler.h"
#include "renderer/vulkan/allocator.h"
#include "renderer/vulkan/buffer.h"
#include "renderer/vulkan/context.h"
#include "renderer/vulkan/frame.h"
#include "renderer/vulkan/image.h"
#include "renderer/vulkan/sampler.h"
#include "renderer/vulkan/uniform_buffer.h"

namespace ck {

namespace {

struct Vertex {
  glm::vec3 position;
  glm::vec4 color;
  glm::vec2 uv;
  uint32_t  texture_index;
};

struct CameraUBOData {
  glm::mat4 view_proj;
};

constexpr uint32_t kVerticesPerQuad = 4;
constexpr uint32_t kIndicesPerQuad  = 6;

// CCW from bottom-left, with UVs that put (0,0) at top-left so the
// existing checkerboard texture orientation matches phase 5.3.
constexpr glm::vec4 kQuadCorners[4] = {
    {-0.5f, -0.5f, 0.0f, 1.0f},
    { 0.5f, -0.5f, 0.0f, 1.0f},
    { 0.5f,  0.5f, 0.0f, 1.0f},
    {-0.5f,  0.5f, 0.0f, 1.0f},
};
constexpr glm::vec2 kQuadUVs[4] = {
    {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
};

struct State {
  vulkan::Context*   ctx = nullptr;
  vk::Device         device;

  Scope<vulkan::ShaderModule>     shader;
  Scope<vulkan::GraphicsPipeline> pipeline;

  vk::DescriptorSetLayout         set_layout;
  vk::DescriptorPool              pool;
  std::array<vk::DescriptorSet, vulkan::kFramesInFlight> sets{};

  Scope<vulkan::UniformBuffer<CameraUBOData>> camera_ubo;
  std::array<Scope<vulkan::Buffer>, vulkan::kFramesInFlight> vertex_buffers;
  Scope<vulkan::Buffer>           index_buffer;

  Scope<vulkan::Image>            white_fallback;
  Scope<vulkan::Sampler>          sampler;

  // Bindless texture array: index = slot, value = image view (already
  // registered into the descriptor array).
  std::vector<vk::ImageView>      registered_views;
  // Per-frame: how many of |registered_views| have been written into
  // sets[frame]. Driving rule: BeginScene catches sets up if registered
  // grew last frame.
  std::array<uint32_t, vulkan::kFramesInFlight> sync_count{};

  // Per-frame batch
  std::vector<Vertex>             cpu_buffer;
  uint32_t                        quad_count   = 0;
  uint32_t                        frame_index  = 0;

  // Last-flushed stats (read via Renderer2D::stats()).
  Renderer2D::Stats               last_stats{};
};

State* g_state = nullptr;

uint32_t RegisterImageView(vk::ImageView view) {
  for (uint32_t i = 0; i < g_state->registered_views.size(); ++i) {
    if (g_state->registered_views[i] == view) return i;
  }
  if (g_state->registered_views.size() >= Renderer2D::kMaxTextures) {
    return 0;  // overflow, white fallback
  }
  g_state->registered_views.push_back(view);
  return static_cast<uint32_t>(g_state->registered_views.size() - 1);
}

void SyncDescriptors(uint32_t frame_index) {
  uint32_t total = static_cast<uint32_t>(g_state->registered_views.size());
  uint32_t synced = g_state->sync_count[frame_index];
  if (synced >= total) return;

  std::vector<vk::DescriptorImageInfo> infos;
  infos.reserve(total - synced);
  std::vector<vk::WriteDescriptorSet> writes;
  writes.reserve(total - synced);
  for (uint32_t i = synced; i < total; ++i) {
    auto& info = infos.emplace_back();
    info.sampler = g_state->sampler->handle();
    info.imageView = g_state->registered_views[i];
    info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
  }
  for (uint32_t i = 0; i < total - synced; ++i) {
    auto& w = writes.emplace_back();
    w.dstSet = g_state->sets[frame_index];
    w.dstBinding = 1;
    w.dstArrayElement = synced + i;
    w.descriptorCount = 1;
    w.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    w.pImageInfo = &infos[i];
  }
  g_state->device.updateDescriptorSets(writes, {});
  g_state->sync_count[frame_index] = total;
}

void EmitQuad(const glm::mat4& transform, const glm::vec4& color, uint32_t tex_id) {
  if (g_state->quad_count >= Renderer2D::kMaxQuads) return;
  uint32_t base = g_state->quad_count * kVerticesPerQuad;
  for (uint32_t i = 0; i < kVerticesPerQuad; ++i) {
    Vertex& v = g_state->cpu_buffer[base + i];
    glm::vec4 world = transform * kQuadCorners[i];
    v.position = glm::vec3(world);
    v.color = color;
    v.uv = kQuadUVs[i];
    v.texture_index = tex_id;
  }
  ++g_state->quad_count;
}

}  // namespace

void Renderer2D::Init(vulkan::Context& ctx, vulkan::Allocator& alloc,
                      vulkan::SlangCompiler& compiler, vk::Format color_format) {
  CK_PROFILE_FUNCTION();
  CK_ENGINE_ASSERT(g_state == nullptr, "Renderer2D already initialized");

  g_state = new State();
  g_state->ctx = &ctx;
  g_state->device = ctx.device();

  // -- 1x1 white fallback texture (slot 0)
  vulkan::Image::CreateInfo white_ci{};
  white_ci.format = vk::Format::eR8G8B8A8Unorm;
  white_ci.extent = vk::Extent2D{1, 1};
  white_ci.usage = vk::ImageUsageFlagBits::eSampled |
                   vk::ImageUsageFlagBits::eTransferDst;
  g_state->white_fallback = CreateScope<vulkan::Image>(ctx, alloc, white_ci);
  // Upload one opaque-white pixel via staging.
  {
    constexpr uint32_t kWhite = 0xFFFFFFFFu;
    auto staging = CreateScope<vulkan::Buffer>(
        alloc, sizeof(kWhite), vk::BufferUsageFlagBits::eTransferSrc,
        vulkan::BufferType::HostVisible);
    std::memcpy(staging->mapped(), &kWhite, sizeof(kWhite));
    alloc.ImmediateSubmit([&](vk::CommandBuffer cmd) {
      vk::ImageMemoryBarrier2 to_dst{};
      to_dst.srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe;
      to_dst.dstStageMask = vk::PipelineStageFlagBits2::eCopy;
      to_dst.dstAccessMask = vk::AccessFlagBits2::eTransferWrite;
      to_dst.oldLayout = vk::ImageLayout::eUndefined;
      to_dst.newLayout = vk::ImageLayout::eTransferDstOptimal;
      to_dst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      to_dst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      to_dst.image = g_state->white_fallback->handle();
      to_dst.subresourceRange = vk::ImageSubresourceRange{
          vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
      vk::DependencyInfo dep{};
      dep.imageMemoryBarrierCount = 1;
      dep.pImageMemoryBarriers = &to_dst;
      cmd.pipelineBarrier2(dep);

      vk::BufferImageCopy region{};
      region.imageSubresource = vk::ImageSubresourceLayers{
          vk::ImageAspectFlagBits::eColor, 0, 0, 1};
      region.imageExtent = vk::Extent3D{1, 1, 1};
      cmd.copyBufferToImage(staging->handle(), g_state->white_fallback->handle(),
                            vk::ImageLayout::eTransferDstOptimal, region);

      vk::ImageMemoryBarrier2 to_read{};
      to_read.srcStageMask = vk::PipelineStageFlagBits2::eCopy;
      to_read.srcAccessMask = vk::AccessFlagBits2::eTransferWrite;
      to_read.dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader;
      to_read.dstAccessMask = vk::AccessFlagBits2::eShaderSampledRead;
      to_read.oldLayout = vk::ImageLayout::eTransferDstOptimal;
      to_read.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
      to_read.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      to_read.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      to_read.image = g_state->white_fallback->handle();
      to_read.subresourceRange = vk::ImageSubresourceRange{
          vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
      vk::DependencyInfo dep2{};
      dep2.imageMemoryBarrierCount = 1;
      dep2.pImageMemoryBarriers = &to_read;
      cmd.pipelineBarrier2(dep2);
    });
  }

  g_state->sampler = CreateScope<vulkan::Sampler>(ctx);

  // -- Bindless descriptor set layout
  std::array<vk::DescriptorSetLayoutBinding, 2> bindings{};
  bindings[0].binding = 0;
  bindings[0].descriptorType = vk::DescriptorType::eUniformBuffer;
  bindings[0].descriptorCount = 1;
  bindings[0].stageFlags = vk::ShaderStageFlagBits::eVertex;
  bindings[1].binding = 1;
  bindings[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
  bindings[1].descriptorCount = kMaxTextures;
  bindings[1].stageFlags = vk::ShaderStageFlagBits::eFragment;

  std::array<vk::DescriptorBindingFlags, 2> binding_flags{{
      vk::DescriptorBindingFlags{},
      vk::DescriptorBindingFlagBits::ePartiallyBound |
          vk::DescriptorBindingFlagBits::eVariableDescriptorCount,
  }};
  vk::DescriptorSetLayoutBindingFlagsCreateInfo flags_ci{};
  flags_ci.bindingCount = static_cast<uint32_t>(binding_flags.size());
  flags_ci.pBindingFlags = binding_flags.data();

  vk::DescriptorSetLayoutCreateInfo layout_ci{};
  layout_ci.bindingCount = static_cast<uint32_t>(bindings.size());
  layout_ci.pBindings = bindings.data();
  layout_ci.pNext = &flags_ci;
  g_state->set_layout = g_state->device.createDescriptorSetLayout(layout_ci);

  // -- Internal descriptor pool sized for our two per-frame sets.
  std::array<vk::DescriptorPoolSize, 2> pool_sizes{{
      {vk::DescriptorType::eUniformBuffer, vulkan::kFramesInFlight},
      {vk::DescriptorType::eCombinedImageSampler,
       vulkan::kFramesInFlight * kMaxTextures},
  }};
  vk::DescriptorPoolCreateInfo pool_ci{};
  pool_ci.maxSets = vulkan::kFramesInFlight;
  pool_ci.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
  pool_ci.pPoolSizes = pool_sizes.data();
  g_state->pool = g_state->device.createDescriptorPool(pool_ci);

  // -- Allocate per-frame sets with variable descriptor count = kMaxTextures.
  {
    std::array<vk::DescriptorSetLayout, vulkan::kFramesInFlight> layouts;
    layouts.fill(g_state->set_layout);
    std::array<uint32_t, vulkan::kFramesInFlight> counts;
    counts.fill(kMaxTextures);
    vk::DescriptorSetVariableDescriptorCountAllocateInfo var_ci{};
    var_ci.descriptorSetCount = vulkan::kFramesInFlight;
    var_ci.pDescriptorCounts = counts.data();

    vk::DescriptorSetAllocateInfo alloc_info{};
    alloc_info.descriptorPool = g_state->pool;
    alloc_info.descriptorSetCount = vulkan::kFramesInFlight;
    alloc_info.pSetLayouts = layouts.data();
    alloc_info.pNext = &var_ci;
    auto allocated = g_state->device.allocateDescriptorSets(alloc_info);
    for (uint32_t i = 0; i < vulkan::kFramesInFlight; ++i) {
      g_state->sets[i] = allocated[i];
    }
  }

  // -- Camera UBO ring + initial binding-0 writes.
  g_state->camera_ubo = CreateScope<vulkan::UniformBuffer<CameraUBOData>>(
      alloc, vulkan::kFramesInFlight);
  for (uint32_t i = 0; i < vulkan::kFramesInFlight; ++i) {
    vk::DescriptorBufferInfo bi{};
    bi.buffer = g_state->camera_ubo->Handle(i);
    bi.offset = 0;
    bi.range = sizeof(CameraUBOData);
    vk::WriteDescriptorSet w{};
    w.dstSet = g_state->sets[i];
    w.dstBinding = 0;
    w.descriptorCount = 1;
    w.descriptorType = vk::DescriptorType::eUniformBuffer;
    w.pBufferInfo = &bi;
    g_state->device.updateDescriptorSets(w, {});
  }

  // -- Register white fallback as slot 0 (across all frame sets).
  RegisterImageView(g_state->white_fallback->view());
  for (uint32_t i = 0; i < vulkan::kFramesInFlight; ++i) {
    SyncDescriptors(i);
  }

  // -- Per-frame dynamic vertex buffer (host-visible, persistently mapped).
  for (uint32_t i = 0; i < vulkan::kFramesInFlight; ++i) {
    g_state->vertex_buffers[i] = CreateScope<vulkan::Buffer>(
        alloc, kMaxQuads * kVerticesPerQuad * sizeof(Vertex),
        vk::BufferUsageFlagBits::eVertexBuffer,
        vulkan::BufferType::HostVisible);
  }

  // -- Static index buffer.
  {
    std::vector<uint32_t> indices(kMaxQuads * kIndicesPerQuad);
    for (uint32_t q = 0; q < kMaxQuads; ++q) {
      uint32_t base = q * kVerticesPerQuad;
      uint32_t out = q * kIndicesPerQuad;
      indices[out + 0] = base + 0;
      indices[out + 1] = base + 1;
      indices[out + 2] = base + 2;
      indices[out + 3] = base + 2;
      indices[out + 4] = base + 3;
      indices[out + 5] = base + 0;
    }
    g_state->index_buffer = vulkan::Buffer::CreateDeviceLocal(
        alloc, indices.data(), indices.size() * sizeof(uint32_t),
        vk::BufferUsageFlagBits::eIndexBuffer);
  }

  // -- Compile shader and build pipeline.
  auto spirv = compiler.CompileToSpirv("assets/shaders/renderer_2d_quad.slang");
  CK_ENGINE_ASSERT(!spirv.empty(),
                   "Slang compile failed for renderer_2d_quad.slang");
  g_state->shader = CreateScope<vulkan::ShaderModule>(ctx, std::span{spirv});

  vk::VertexInputBindingDescription vb_binding{};
  vb_binding.binding = 0;
  vb_binding.stride = sizeof(Vertex);
  vb_binding.inputRate = vk::VertexInputRate::eVertex;
  std::array<vk::VertexInputAttributeDescription, 4> vb_attrs{};
  vb_attrs[0].location = 0;
  vb_attrs[0].format = vk::Format::eR32G32B32Sfloat;
  vb_attrs[0].offset = offsetof(Vertex, position);
  vb_attrs[1].location = 1;
  vb_attrs[1].format = vk::Format::eR32G32B32A32Sfloat;
  vb_attrs[1].offset = offsetof(Vertex, color);
  vb_attrs[2].location = 2;
  vb_attrs[2].format = vk::Format::eR32G32Sfloat;
  vb_attrs[2].offset = offsetof(Vertex, uv);
  vb_attrs[3].location = 3;
  vb_attrs[3].format = vk::Format::eR32Uint;
  vb_attrs[3].offset = offsetof(Vertex, texture_index);

  vulkan::VertexInput vi{
      .bindings = std::span{&vb_binding, 1},
      .attributes = std::span{vb_attrs.data(), vb_attrs.size()},
  };
  g_state->pipeline = CreateScope<vulkan::GraphicsPipeline>(
      ctx, *g_state->shader, color_format, vi,
      std::span{&g_state->set_layout, 1}, ctx.pipeline_cache());

  g_state->cpu_buffer.resize(kMaxQuads * kVerticesPerQuad);

  CK_ENGINE_INFO("Renderer2D ready: max {} quads, max {} textures",
                 kMaxQuads, kMaxTextures);
}

void Renderer2D::Shutdown() {
  CK_PROFILE_FUNCTION();
  if (!g_state) return;
  if (g_state->set_layout) g_state->device.destroyDescriptorSetLayout(g_state->set_layout);
  if (g_state->pool) g_state->device.destroyDescriptorPool(g_state->pool);
  delete g_state;
  g_state = nullptr;
}

void Renderer2D::BeginScene(const Camera& camera, uint32_t frame_index) {
  CK_PROFILE_FUNCTION();
  g_state->frame_index = frame_index;
  g_state->quad_count = 0;

  // Ensure this frame's set has descriptors for any textures registered
  // since it was last used.
  SyncDescriptors(frame_index);

  CameraUBOData ubo{};
  ubo.view_proj = camera.view_projection();
  g_state->camera_ubo->Write(frame_index, ubo);
}

void Renderer2D::EndScene(vk::CommandBuffer cmd) {
  CK_PROFILE_FUNCTION();
  uint32_t qc = g_state->quad_count;
  uint32_t fi = g_state->frame_index;

  g_state->last_stats.quad_count = qc;
  g_state->last_stats.texture_count =
      static_cast<uint32_t>(g_state->registered_views.size());
  g_state->last_stats.draw_calls = qc > 0 ? 1 : 0;

  if (qc == 0) return;

  std::memcpy(g_state->vertex_buffers[fi]->mapped(),
              g_state->cpu_buffer.data(),
              qc * kVerticesPerQuad * sizeof(Vertex));

  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, g_state->pipeline->handle());
  cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                         g_state->pipeline->layout(), 0,
                         g_state->sets[fi], {});
  vk::Buffer vbo = g_state->vertex_buffers[fi]->handle();
  vk::DeviceSize vbo_offset = 0;
  cmd.bindVertexBuffers(0, 1, &vbo, &vbo_offset);
  cmd.bindIndexBuffer(g_state->index_buffer->handle(), 0, vk::IndexType::eUint32);
  cmd.drawIndexed(qc * kIndicesPerQuad, 1, 0, 0, 0);
}

void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color) {
  EmitQuad(transform, color, 0);
}

void Renderer2D::DrawQuad(const glm::mat4& transform, const vulkan::Image& texture,
                          const glm::vec4& tint) {
  uint32_t slot = RegisterImageView(texture.view());
  EmitQuad(transform, tint, slot);
}

Renderer2D::Stats Renderer2D::stats() {
  return g_state ? g_state->last_stats : Stats{};
}

}  // namespace ck
