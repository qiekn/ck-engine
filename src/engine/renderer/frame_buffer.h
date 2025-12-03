#pragma once

#include "core/core.h"
namespace ck {
// ----------------------------------------------------------------------------: data
struct FrameBufferSpecification {
  uint32_t width, height;
  uint32_t samples = 1;

  bool is_swap_chain_target = false;
};

// ----------------------------------------------------------------------------: class
class FrameBuffer {
public:
  virtual void Bind() = 0;
  virtual void Unbind() = 0;

  virtual uint32_t GetColorAttachmentRendererID() const = 0;

  virtual const FrameBufferSpecification& GetSpecification() const = 0;

  static Ref<FrameBuffer> Create(const FrameBufferSpecification& spec);
};

}  // namespace ck
