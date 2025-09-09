#include <ck.h>

class Sandbox : public ck::Application {
public:
  Sandbox() {}
  ~Sandbox() {}
};

MAKE_APPLICATION(Sandbox)
