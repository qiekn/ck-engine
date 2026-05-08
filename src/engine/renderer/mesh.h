#pragma once

#include <cstdint>
#include <filesystem>
#include <span>
#include <string>

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include "core/core.h"

namespace ck::vulkan {
class Allocator;
class Buffer;
}  // namespace ck::vulkan

namespace ck {

// Per-vertex inputs consumed by the Renderer3D shader. Position + normal is
// enough for Lambert lighting; texture / tangent space follow in 6.D.
struct MeshVertex {
  glm::vec3 position;
  glm::vec3 normal;
};

// Device-local vertex + index buffer pair. Shared via Ref<Mesh> from
// MeshComponent.
class Mesh {
public:
  Mesh(vulkan::Allocator& alloc,
       std::span<const MeshVertex> vertices,
       std::span<const uint32_t> indices);
  ~Mesh();

  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;
  Mesh(Mesh&&) = delete;
  Mesh& operator=(Mesh&&) = delete;

  vk::Buffer vertex_buffer() const;
  vk::Buffer index_buffer() const;
  uint32_t   index_count()   const { return index_count_; }

  // Procedural unit cube centered at the origin with face normals (no
  // sharing across faces -> 24 verts, 36 indices). CCW winding on the
  // outside; pair with cull_back.
  static Ref<Mesh> CreateCube(vulkan::Allocator& alloc);

  // Path-driven loader -- the persistent identifier carried by
  // MeshComponent. Special tokens:
  //   "cube"  (or empty) -> CreateCube
  //   anything else      -> FromFile (currently logs a warn + cube
  //                          fallback; tinyobjloader lands in 6.C.5)
  static Ref<Mesh> Load(const std::string& path, vulkan::Allocator& alloc);

  // .obj loader (tinyobjloader). Returns nullptr on parse failure.
  static Ref<Mesh> FromFile(const std::filesystem::path& path,
                            vulkan::Allocator& alloc);

private:
  Scope<vulkan::Buffer> vertex_buffer_;
  Scope<vulkan::Buffer> index_buffer_;
  uint32_t index_count_ = 0;
};

}  // namespace ck
