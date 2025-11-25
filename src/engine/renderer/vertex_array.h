#pragma once

#include "renderer/buffer.h"

namespace ck {
class VertexArray {
public:
  virtual ~VertexArray() {}

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  virtual void AddVertexBuffer(Ref<VertexBuffer>) = 0;
  virtual void SetIndexBuffer(Ref<IndexBuffer>) = 0;

  virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const = 0;

  virtual const Ref<IndexBuffer>& GetIndexBuffer() const = 0;

  static Scope<VertexArray> Create();
};
}  // namespace ck
