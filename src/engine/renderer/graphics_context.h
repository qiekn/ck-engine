#pragma once

namespace ck {
class GraphicContext {
public:
  GraphicContext() {}
  virtual ~GraphicContext() {}

  virtual void Init() = 0;
  virtual void SwapBuffers() = 0;
};
}  // namespace ck
