#pragma once

#include "application.h"
#include "log.h"

int main(int argc, char** argv) {  // NOLINT (clang-diagnostic-odr)
  ck::Log::Init();

  // Test Log System
  CK_CLIENT_TRACE("Initialized Log!");

  auto app = ck::CreateApplication();
  app->Run();
  delete app;
}
