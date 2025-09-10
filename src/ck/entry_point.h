#pragma once

#include "application.h"
#include "log.h"

int main(int argc, char** argv) {  // NOLINT (clang-diagnostic-odr)
  ck::Log::Init();

  // Test Log System
  CK_ENGINE_WARN("Initialized Log!");
  int a = 3;
  CK_CLIENT_INFO("Hello! var={0}", a);

  auto app = ck::CreateApplication();
  app->Run();
  delete app;
}
