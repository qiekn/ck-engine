#pragma once

#include <memory>

namespace ck {
class VertexBuffer {
public:
  virtual ~VertexBuffer() {}

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  static std::unique_ptr<VertexBuffer> Create(float* vertices, uint32_t size);
};

class IndexBuffer {
public:
  virtual ~IndexBuffer() {}

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  virtual uint32_t Count() const = 0;

  static std::unique_ptr<IndexBuffer> Create(uint32_t* indices, uint32_t count);
};
}  // namespace ck
