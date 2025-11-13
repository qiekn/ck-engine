#pragma once

#include <memory>
#include <vector>

#include "renderer/vertex_array.h"

namespace ck {
class OpenglVertexArray : public VertexArray {
public:
  OpenglVertexArray();
  virtual ~OpenglVertexArray();

  void Bind() const override;
  void Unbind() const override;

  void AddVertexBuffer(Ref<VertexBuffer>) override;
  void SetIndexBuffer(Ref<IndexBuffer>) override;

  const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const override {
    return vertex_buffers_;
  }

  const Ref<IndexBuffer>& GetIndexBuffer() const override { return index_buffer_; }

private:
  uint32_t renderer_id_;

  std::vector<Ref<VertexBuffer>> vertex_buffers_;
  Ref<IndexBuffer> index_buffer_;
};
}  // namespace ck
