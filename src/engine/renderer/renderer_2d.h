#pragma once

#include <cstdint>
#include <filesystem>

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace ck::vulkan {
class Context;
class Allocator;
class SlangCompiler;
class Image;
}  // namespace ck::vulkan

namespace ck {

// Hazel-style 2D quad batcher.
//
// One static instance, lifetime managed by Renderer (Init in ctor, Shutdown
// in dtor). All state lives in a file-static `State` struct in the .cpp.
//
// Bindless texture array (binding 1, variable descriptor count, partially
// bound) holds up to kMaxTextures images. Slot 0 is always a 1×1 opaque
// white fallback so DrawQuad(color) can sample uniformly through the
// shader path.
//
// Frame flow:
//   Renderer::BeginFrame() -> beginRendering -> Renderer2D::BeginScene
//   layers OnUpdate -> Renderer2D::DrawQuad...
//   Renderer::EndFrame() -> Renderer2D::EndScene(cmd) -> endRendering ->
//     submit/present
class Renderer2D {
public:
  static constexpr uint32_t kMaxQuads    = 10000;
  static constexpr uint32_t kMaxTextures = 32;

  static void Init(vulkan::Context& ctx, vulkan::Allocator& alloc,
                   vulkan::SlangCompiler& compiler, vk::Format color_format,
                   vk::Format depth_format);
  static void Shutdown();

  // Begin a frame's batch. |frame_index| is the frame-in-flight slot the
  // caller is currently recording for; used to pick the per-frame UBO and
  // vertex buffer. Newly-registered textures are written into this frame's
  // descriptor set here. Camera matrix is fed to EndScene so layers can
  // mutate the active camera in their OnUpdate without a frame of lag.
  static void BeginScene(uint32_t frame_index);

  // Flush the batch into |cmd| (must already be in a renderpass / dynamic
  // rendering scope with the matching color format). Writes |view_projection|
  // into this frame's UBO before recording bindPipeline + bindDescriptorSets
  // + bindVertex/IndexBuffers + drawIndexed.
  static void EndScene(vk::CommandBuffer cmd, const glm::mat4& view_projection);

  // Opaque handle into the bindless texture array. 0 is always the white
  // fallback; LoadTexture returns 1, 2, ... up to kMaxTextures - 1.
  using TextureHandle = uint32_t;
  static constexpr TextureHandle kWhiteTexture = 0;

  // Per-texture sampler choice. Linear = smooth (default, photos);
  // Nearest = crisp pixels (checkerboard / pixel art).
  enum class Filter { Linear, Nearest };

  // Load |path| via stb_image, register into the bindless array, and
  // return its slot. Idempotent on (path, filter): a path loaded twice
  // with the same filter gets the same slot; loading the same path with a
  // different filter allocates a fresh slot.
  // Lifetime is tied to Renderer2D itself (until Shutdown).
  static TextureHandle LoadTexture(const std::filesystem::path& path,
                                   Filter filter = Filter::Linear);

  // Solid-color quad. tex_id points at the white fallback (slot 0).
  static void DrawQuad(const glm::mat4& transform, const glm::vec4& color);

  // Textured quad. |texture| must come from LoadTexture (or kWhiteTexture).
  // |tint| multiplies the sampled color.
  static void DrawQuad(const glm::mat4& transform, TextureHandle texture,
                       const glm::vec4& tint = glm::vec4(1.0f));

  struct Stats {
    uint32_t quad_count = 0;
    uint32_t texture_count = 0;
    uint32_t draw_calls = 0;
  };
  static Stats stats();
};

}  // namespace ck
