#pragma once

#include <cstdint>

#include "renderer/buffer.h"

namespace ck {
class OpenGLVertexBuffer : public VertexBuffer {
public:
  OpenGLVertexBuffer(float* vertices, uint32_t size);
  virtual ~OpenGLVertexBuffer();

  void Bind() const override;
  void Unbind() const override;

  const BufferLayout& Layout() const override { return layout_; }
  void SetLayout(const BufferLayout& layout) override { layout_ = layout; }

private:
  uint32_t renderer_id_;
  BufferLayout layout_;
};

class OpenGLIndexBuffer : public IndexBuffer {
public:
  OpenGLIndexBuffer(uint32_t* vertices, uint32_t count);
  ~OpenGLIndexBuffer();

  void Bind() const override;
  void Unbind() const override;

  uint32_t Count() const override { return count_; }

private:
  uint32_t renderer_id_;
  uint32_t count_;
};
}  // namespace ck
