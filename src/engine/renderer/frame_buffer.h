#pragma once

#include <vector>

#include "core/core.h"

namespace ck {
// ----------------------------------------------------------------------------: data
enum class FrameBufferTextureFormat {
  None = 0,

  // Color
  RGBA8,

  // Depth/stencil
  Depth24Stencil8,

  // Defaults
  Depth = Depth24Stencil8
};

struct FrameBufferTextureSpecification {
  FrameBufferTextureSpecification() = default;
  FrameBufferTextureSpecification(FrameBufferTextureFormat format) : texture_format(format) {}

  FrameBufferTextureFormat texture_format = FrameBufferTextureFormat::None;
  // TODO: filtering/wrap
};

struct FrameBufferAttachmentSpecification {
  FrameBufferAttachmentSpecification() = default;
  FrameBufferAttachmentSpecification(
      std::initializer_list<FrameBufferTextureSpecification> attachments)
      : attachments(attachments) {}

  std::vector<FrameBufferTextureSpecification> attachments;
};

struct FrameBufferSpecification {
  uint32_t width = 0, height = 0;
  FrameBufferAttachmentSpecification attachments;
  uint32_t samples = 1;

  bool is_swap_chain_target = false;
};

// ----------------------------------------------------------------------------: class
class FrameBuffer {
public:
  virtual void Bind() = 0;
  virtual void Unbind() = 0;

  virtual void Resize(uint32_t width, uint32_t height) = 0;

  virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;

  virtual const FrameBufferSpecification& GetSpecification() const = 0;

  static Ref<FrameBuffer> Create(const FrameBufferSpecification& spec);
};

}  // namespace ck
