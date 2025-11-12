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

  void AddVertexBuffer(std::shared_ptr<VertexBuffer>) override;
  void SetIndexBuffer(std::shared_ptr<IndexBuffer>) override;

  const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override {
    return vertex_buffers_;
  }

  const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const override { return index_buffer_; }

private:
  uint32_t renderer_id_;

  std::vector<std::shared_ptr<VertexBuffer>> vertex_buffers_;
  std::shared_ptr<IndexBuffer> index_buffer_;
};
}  // namespace ck
