#pragma once

#include "renderer/buffer.h"

namespace ck {
class VertexArray {
public:
  virtual ~VertexArray() {}

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  virtual void AddVertexBuffer(std::shared_ptr<VertexBuffer>) = 0;
  virtual void SetIndexBuffer(std::shared_ptr<IndexBuffer>) = 0;

  virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const = 0;
  ;
  virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const = 0;

  static std::unique_ptr<VertexArray> Create();
};
}  // namespace ck
