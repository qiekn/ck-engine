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
  virtual int ReadPixel(uint32_t attachment_index, int x, int y) override;

  virtual void ClearAttachment(uint32_t attachment_index, int value) override;

  virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override {
    CK_ENGINE_ASSERT(index < color_attachments_.size(), "Color attachment index out of range");
    return color_attachments_[index];
  }

  virtual const FrameBufferSpecification& GetSpecification() const override {
    return specification_;
  }

private:
  uint32_t renderer_id_ = 0;
  FrameBufferSpecification specification_;

  std::vector<FrameBufferTextureSpecification> color_attachment_specifications_;
  FrameBufferTextureSpecification depth_attachment_specification_ = FrameBufferTextureFormat::None;

  std::vector<uint32_t> color_attachments_;
  uint32_t depth_attachment_ = 0;
};
}  // namespace ck
