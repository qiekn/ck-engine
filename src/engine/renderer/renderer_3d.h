#pragma once

#include <cstdint>
#include <filesystem>

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include "core/core.h"

namespace ck::vulkan {
class Context;
class Allocator;
class SlangCompiler;
}  // namespace ck::vulkan

namespace ck {

class Mesh;

// 3D mesh draw path. Single Lambert-lit pipeline (one descriptor set with
// the camera UBO at binding 0; per-draw model matrix + tint pushed via
// push constants). One drawIndexed per DrawMesh -- no instancing yet.
//
// Frame flow mirrors Renderer2D:
//   Renderer::BeginFrame() -> beginRendering -> Renderer3D::BeginScene
//   layers OnUpdate -> Renderer3D::DrawMesh...
//   Renderer::EndFrame() -> Renderer3D::EndScene(cmd, vp) -> Renderer2D::EndScene
class Renderer3D {
public:
  static void Init(vulkan::Context& ctx, vulkan::Allocator& alloc,
                   vulkan::SlangCompiler& compiler,
                   vk::Format color_format, vk::Format depth_format);
  static void Shutdown();

  static void BeginScene(uint32_t frame_index);
  static void DrawMesh(const Ref<Mesh>& mesh, const glm::mat4& transform,
                       const glm::vec3& tint = glm::vec3(1.0f));
  static void EndScene(vk::CommandBuffer cmd, const glm::mat4& view_projection);

  struct Stats {
    uint32_t mesh_count = 0;
    uint32_t draw_calls = 0;
  };
  static Stats stats();
};

}  // namespace ck
