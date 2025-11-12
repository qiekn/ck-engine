#pragma once

#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

#include "log.h"

namespace ck {
enum class ShaderDataType {
  kNone = 0,
  kFloat,
  kFloat2,
  kFloat3,
  kFloat4,
  kMat3,
  kMat4,
  kInt,
  kInt2,
  kInt3,
  kInt4,
  kBool
};

static uint32_t ShaderDataTypeSize(ShaderDataType type) {
  switch (type) {
    case ShaderDataType::kNone:
      CK_ENGINE_ASSERT(false, "unknown ShaderDataType");
      return 0;

    case ShaderDataType::kFloat:
      return 4;
    case ShaderDataType::kFloat2:
      return 4 * 2;
    case ShaderDataType::kFloat3:
      return 4 * 3;
    case ShaderDataType::kFloat4:
      return 4 * 4;

    case ShaderDataType::kMat3:
      return 4 * 3 * 3;
    case ShaderDataType::kMat4:
      return 4 * 4 * 4;

    case ShaderDataType::kInt:
      return 4;
    case ShaderDataType::kInt2:
      return 4 * 2;
    case ShaderDataType::kInt3:
      return 4 * 3;
    case ShaderDataType::kInt4:
      return 4 * 4;

    case ShaderDataType::kBool:
      return 1;
  }
}

struct BufferElement {
  ShaderDataType type;
  std::string name;
  uint32_t size;
  uint32_t offset = 0;
  bool normalized = false;

  BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
      : type(type), name(name), size(ShaderDataTypeSize(type)), normalized(normalized) {}

  uint32_t ComponentCount() const {
    switch (type) {
      case ShaderDataType::kNone:
        CK_ENGINE_ASSERT(false, "unknown ShaderDataType");
        return 0;

      case ShaderDataType::kInt:
      case ShaderDataType::kFloat:
      case ShaderDataType::kBool:
        return 1;

      case ShaderDataType::kInt2:
      case ShaderDataType::kFloat2:
        return 2;

      case ShaderDataType::kInt3:
      case ShaderDataType::kFloat3:
        return 3;

      case ShaderDataType::kInt4:
      case ShaderDataType::kFloat4:
        return 4;

      case ShaderDataType::kMat3:
        return 3 * 3;
      case ShaderDataType::kMat4:
        return 4 * 4;
    }
  }
};

class BufferLayout {
public:
  BufferLayout() {}
  BufferLayout(const std::initializer_list<BufferElement>& elements) : elements_(elements) {
    CalculateOffsetsAndStride();
  }

  inline const auto& elements() const { return elements_; }

  auto begin() { return elements_.begin(); }
  auto end() { return elements_.end(); }

  auto begin() const { return elements_.begin(); }
  auto end() const { return elements_.end(); }

  uint32_t stride() const { return stride_; }

private:
  void CalculateOffsetsAndStride() {
    uint32_t offset = 0;
    stride_ = 0;

    for (auto& e : elements_) {
      e.offset = offset;
      offset += e.size;
      stride_ += e.size;
    }
  }

private:
  std::vector<BufferElement> elements_;
  uint32_t stride_ = 0;
};

class VertexBuffer {
public:
  virtual ~VertexBuffer() {}

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  virtual const BufferLayout& Layout() const = 0;
  virtual void SetLayout(const BufferLayout& layout) = 0;

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
