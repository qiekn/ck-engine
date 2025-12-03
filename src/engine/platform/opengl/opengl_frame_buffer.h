#pragma once

#include "renderer/frame_buffer.h"

namespace ck {
class OpenglFrameBuffer : public FrameBuffer {
public:
  explicit OpenglFrameBuffer(const FrameBufferSpecification& spec);
  virtual ~OpenglFrameBuffer();

  void Invalidate();

  virtual void Bind() override;
  virtual void Unbind() override;

  virtual void Resize(uint32_t width, uint32_t height) override;

  virtual uint32_t GetColorAttachmentRendererID() const override { return color_attachment_; }

  virtual const FrameBufferSpecification& GetSpecification() const override {
    return specification_;
  }

private:
  uint32_t renderer_id_ = 0;
  uint32_t color_attachment_ = 0, depth_attachment_ = 0;
  FrameBufferSpecification specification_;
};
}  // namespace ck
