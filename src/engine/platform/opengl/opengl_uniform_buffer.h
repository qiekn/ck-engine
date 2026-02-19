#pragma once

#include "renderer/uniform_buffer.h"

namespace ck {

class OpenglUniformBuffer : public UniformBuffer {
public:
  OpenglUniformBuffer(uint32_t size, uint32_t binding);
  virtual ~OpenglUniformBuffer();

  virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

private:
  uint32_t renderer_id_ = 0;
};

}  // namespace ck
