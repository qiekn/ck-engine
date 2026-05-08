#include "renderer/renderer_3d.h"

#include <array>
#include <cstring>
#include <span>
#include <vector>

#include <volk.h>

#include "core/core.h"
#include "core/log.h"
#include "debug/profiler.h"
#include "renderer/mesh.h"
#include "renderer/shader/graphics_pipeline.h"
#include "renderer/shader/shader_module.h"
#include "renderer/shader/slang_compiler.h"
#include "renderer/vulkan/allocator.h"
#include "renderer/vulkan/buffer.h"
#include "renderer/vulkan/context.h"
#include "renderer/vulkan/frame.h"
#include "renderer/vulkan/uniform_buffer.h"

namespace ck {

namespace {

struct CameraUBOData {
  glm::mat4 view_proj;
};

// 64B mat4 + vec3 (padded to 16B). Match the shader's [[vk::push_constant]]
// PushBlock layout exactly so the bytes line up.
struct alignas(16) PushBlock {
  glm::mat4 model;
  glm::vec3 tint;
  float     _pad = 0.0f;
};
static_assert(sizeof(PushBlock) == 80, "PushBlock layout must match shader");

struct DrawCmd {
  Ref<Mesh> mesh;
  glm::mat4 transform;
  glm::vec3 tint;
};

struct State {
  vulkan::Context*  ctx = nullptr;
  vulkan::Allocator* alloc = nullptr;
  vk::Device        device;

  Scope<vulkan::ShaderModule>     shader;
  Scope<vulkan::GraphicsPipeline> pipeline;

  vk::DescriptorSetLayout         set_layout;
  vk::DescriptorPool              pool;
  std::array<vk::DescriptorSet, vulkan::kFramesInFlight> sets{};

  Scope<vulkan::UniformBuffer<CameraUBOData>> camera_ubo;

  std::vector<DrawCmd>            queue;
  uint32_t                        frame_index = 0;
  Renderer3D::Stats               last_stats{};
};

State* g_state = nullptr;

}  // namespace

void Renderer3D::Init(vulkan::Context& ctx, vulkan::Allocator& alloc,
                      vulkan::SlangCompiler& compiler,
                      vk::Format color_format, vk::Format depth_format) {
  CK_PROFILE_FUNCTION();
  CK_ASSERT(g_state == nullptr, "Renderer3D already initialized");

  g_state = new State();
  g_state->ctx = &ctx;
  g_state->alloc = &alloc;
  g_state->device = ctx.device();

  // Descriptor set layout: binding 0 = camera UBO (vertex stage only).
  vk::DescriptorSetLayoutBinding binding{};
  binding.binding = 0;
  binding.descriptorType = vk::DescriptorType::eUniformBuffer;
  binding.descriptorCount = 1;
  binding.stageFlags = vk::ShaderStageFlagBits::eVertex;

  vk::DescriptorSetLayoutCreateInfo layout_ci{};
  layout_ci.bindingCount = 1;
  layout_ci.pBindings = &binding;
  g_state->set_layout = g_state->device.createDescriptorSetLayout(layout_ci);

  // Pool sized for our per-frame UBO sets.
  vk::DescriptorPoolSize pool_size{vk::DescriptorType::eUniformBuffer,
                                   vulkan::kFramesInFlight};
  vk::DescriptorPoolCreateInfo pool_ci{};
  pool_ci.maxSets = vulkan::kFramesInFlight;
  pool_ci.poolSizeCount = 1;
  pool_ci.pPoolSizes = &pool_size;
  g_state->pool = g_state->device.createDescriptorPool(pool_ci);

  std::array<vk::DescriptorSetLayout, vulkan::kFramesInFlight> layouts;
  layouts.fill(g_state->set_layout);
  vk::DescriptorSetAllocateInfo alloc_info{};
  alloc_info.descriptorPool = g_state->pool;
  alloc_info.descriptorSetCount = vulkan::kFramesInFlight;
  alloc_info.pSetLayouts = layouts.data();
  auto allocated = g_state->device.allocateDescriptorSets(alloc_info);
  for (uint32_t i = 0; i < vulkan::kFramesInFlight; ++i) {
    g_state->sets[i] = allocated[i];
  }

  // Per-frame UBO + initial binding-0 writes.
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

  // Compile shader and build pipeline.
  auto spirv = compiler.CompileToSpirv("assets/shaders/renderer_3d_mesh.slang");
  CK_ASSERT(!spirv.empty(),
                   "Slang compile failed for renderer_3d_mesh.slang");
  g_state->shader = CreateScope<vulkan::ShaderModule>(ctx, std::span{spirv});

  vk::VertexInputBindingDescription vb_binding{};
  vb_binding.binding = 0;
  vb_binding.stride = sizeof(MeshVertex);
  vb_binding.inputRate = vk::VertexInputRate::eVertex;
  std::array<vk::VertexInputAttributeDescription, 2> vb_attrs{};
  vb_attrs[0].location = 0;
  vb_attrs[0].format = vk::Format::eR32G32B32Sfloat;
  vb_attrs[0].offset = offsetof(MeshVertex, position);
  vb_attrs[1].location = 1;
  vb_attrs[1].format = vk::Format::eR32G32B32Sfloat;
  vb_attrs[1].offset = offsetof(MeshVertex, normal);

  vulkan::VertexInput vi{
      .bindings = std::span{&vb_binding, 1},
      .attributes = std::span{vb_attrs.data(), vb_attrs.size()},
  };

  vk::PushConstantRange push_range{};
  push_range.stageFlags = vk::ShaderStageFlagBits::eVertex |
                          vk::ShaderStageFlagBits::eFragment;
  push_range.offset = 0;
  push_range.size = sizeof(PushBlock);

  vulkan::GraphicsPipelineSpec spec{};
  spec.color_format = color_format;
  spec.depth_format = depth_format;
  spec.depth_test_enable = true;
  spec.depth_write_enable = true;
  spec.depth_compare_op = vk::CompareOp::eLess;
  spec.cull_mode = vk::CullModeFlagBits::eBack;
  spec.front_face = vk::FrontFace::eCounterClockwise;
  spec.set_layouts = std::span{&g_state->set_layout, 1};
  spec.push_constants = std::span{&push_range, 1};

  g_state->pipeline = CreateScope<vulkan::GraphicsPipeline>(
      ctx, *g_state->shader, vi, spec, ctx.pipeline_cache());

  ck::log::info("Renderer3D ready");
}

void Renderer3D::Shutdown() {
  CK_PROFILE_FUNCTION();
  if (!g_state) return;
  if (g_state->set_layout) g_state->device.destroyDescriptorSetLayout(g_state->set_layout);
  if (g_state->pool) g_state->device.destroyDescriptorPool(g_state->pool);
  delete g_state;
  g_state = nullptr;
}

void Renderer3D::BeginScene(uint32_t frame_index) {
  CK_PROFILE_FUNCTION();
  g_state->frame_index = frame_index;
  g_state->queue.clear();
}

void Renderer3D::DrawMesh(const Ref<Mesh>& mesh, const glm::mat4& transform,
                          const glm::vec3& tint) {
  if (!mesh) return;
  g_state->queue.push_back({mesh, transform, tint});
}

void Renderer3D::EndScene(vk::CommandBuffer cmd, const glm::mat4& view_projection) {
  CK_PROFILE_FUNCTION();
  uint32_t fi = g_state->frame_index;
  uint32_t mc = static_cast<uint32_t>(g_state->queue.size());

  g_state->last_stats.mesh_count = mc;
  g_state->last_stats.draw_calls = mc;

  if (mc == 0) return;

  CameraUBOData ubo{};
  ubo.view_proj = view_projection;
  g_state->camera_ubo->Write(fi, ubo);

  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, g_state->pipeline->handle());
  cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                         g_state->pipeline->layout(), 0,
                         g_state->sets[fi], {});

  for (const auto& d : g_state->queue) {
    PushBlock pb{};
    pb.model = d.transform;
    pb.tint = d.tint;
    cmd.pushConstants(g_state->pipeline->layout(),
                      vk::ShaderStageFlagBits::eVertex |
                          vk::ShaderStageFlagBits::eFragment,
                      0, sizeof(PushBlock), &pb);

    vk::Buffer vb = d.mesh->vertex_buffer();
    vk::DeviceSize vb_offset = 0;
    cmd.bindVertexBuffers(0, 1, &vb, &vb_offset);
    cmd.bindIndexBuffer(d.mesh->index_buffer(), 0, vk::IndexType::eUint32);
    cmd.drawIndexed(d.mesh->index_count(), 1, 0, 0, 0);
  }
}

Renderer3D::Stats Renderer3D::stats() {
  return g_state ? g_state->last_stats : Stats{};
}

}  // namespace ck
