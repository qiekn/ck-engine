#include "renderer/mesh.h"

#include <array>

#include "core/log.h"
#include "renderer/vulkan/buffer.h"

namespace ck {

Mesh::Mesh(vulkan::Allocator& alloc,
           std::span<const MeshVertex> vertices,
           std::span<const uint32_t> indices)
    : index_count_(static_cast<uint32_t>(indices.size())) {
  vertex_buffer_ = vulkan::Buffer::CreateDeviceLocal(
      alloc, vertices.data(), vertices.size_bytes(),
      vk::BufferUsageFlagBits::eVertexBuffer);
  index_buffer_ = vulkan::Buffer::CreateDeviceLocal(
      alloc, indices.data(), indices.size_bytes(),
      vk::BufferUsageFlagBits::eIndexBuffer);
}

Mesh::~Mesh() = default;

vk::Buffer Mesh::vertex_buffer() const { return vertex_buffer_->handle(); }
vk::Buffer Mesh::index_buffer()  const { return index_buffer_->handle(); }

Ref<Mesh> Mesh::CreateCube(vulkan::Allocator& alloc) {
  constexpr float h = 0.5f;
  // 6 faces * 4 verts each, listed CCW when viewed from outside.
  const std::array<MeshVertex, 24> verts{{
      // +X
      {{ h, -h, -h}, { 1, 0, 0}}, {{ h,  h, -h}, { 1, 0, 0}},
      {{ h,  h,  h}, { 1, 0, 0}}, {{ h, -h,  h}, { 1, 0, 0}},
      // -X
      {{-h, -h,  h}, {-1, 0, 0}}, {{-h,  h,  h}, {-1, 0, 0}},
      {{-h,  h, -h}, {-1, 0, 0}}, {{-h, -h, -h}, {-1, 0, 0}},
      // +Y
      {{-h,  h, -h}, { 0, 1, 0}}, {{-h,  h,  h}, { 0, 1, 0}},
      {{ h,  h,  h}, { 0, 1, 0}}, {{ h,  h, -h}, { 0, 1, 0}},
      // -Y
      {{-h, -h,  h}, { 0,-1, 0}}, {{-h, -h, -h}, { 0,-1, 0}},
      {{ h, -h, -h}, { 0,-1, 0}}, {{ h, -h,  h}, { 0,-1, 0}},
      // +Z
      {{-h, -h,  h}, { 0, 0, 1}}, {{ h, -h,  h}, { 0, 0, 1}},
      {{ h,  h,  h}, { 0, 0, 1}}, {{-h,  h,  h}, { 0, 0, 1}},
      // -Z
      {{ h, -h, -h}, { 0, 0,-1}}, {{-h, -h, -h}, { 0, 0,-1}},
      {{-h,  h, -h}, { 0, 0,-1}}, {{ h,  h, -h}, { 0, 0,-1}},
  }};

  std::array<uint32_t, 36> indices{};
  for (uint32_t face = 0; face < 6; ++face) {
    uint32_t base = face * 4;
    uint32_t out = face * 6;
    indices[out + 0] = base + 0;
    indices[out + 1] = base + 1;
    indices[out + 2] = base + 2;
    indices[out + 3] = base + 0;
    indices[out + 4] = base + 2;
    indices[out + 5] = base + 3;
  }

  return CreateRef<Mesh>(alloc,
                         std::span<const MeshVertex>{verts.data(), verts.size()},
                         std::span<const uint32_t>{indices.data(), indices.size()});
}

Ref<Mesh> Mesh::Load(const std::string& path, vulkan::Allocator& alloc) {
  if (path.empty() || path == "cube") return CreateCube(alloc);
  if (auto m = FromFile(path, alloc)) return m;
  ck::log::warn("Mesh::Load fallback to cube for '{}'", path);
  return CreateCube(alloc);
}

Ref<Mesh> Mesh::FromFile(const std::filesystem::path& /*path*/,
                         vulkan::Allocator& /*alloc*/) {
  // tinyobjloader wiring lands in phase 6.C.5; until then everything
  // off the "cube" token funnels through the warn+fallback path above.
  return nullptr;
}

}  // namespace ck
