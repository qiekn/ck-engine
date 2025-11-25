#pragma once

#include <string>

#include "core/core.h"

namespace ck {
class Texture {
public:
  virtual ~Texture() = default;

  virtual uint32_t Width() const = 0;
  virtual uint32_t Height() const = 0;

  virtual void SetData(void* data, size_t size) const = 0;

  virtual void Bind(uint32_t slot = 0) const = 0;

  virtual bool operator==(const Texture& other) const = 0;
};

class Texture2D : public Texture {
public:
  virtual ~Texture2D() = default;

  static Scope<Texture2D> Create(const std::string& path);
  static Scope<Texture2D> Create(uint32_t width, uint32_t height);
};
}  // namespace ck
