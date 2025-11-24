#pragma once

#include "application.h"
#include "debug/profiler.h"
#include "log.h"

int main(int argc, char** argv) {  // NOLINT (clang-diagnostic-odr)
  ck::Log::Init();

  CK_PROFILE_BEGIN_SESSION("Startup", "CkProfile-Startup.json");
  auto app = ck::CreateApplication();
  CK_PROFILE_END_SESSION();

  CK_PROFILE_BEGIN_SESSION("Runtime", "CkProfile-Runtime.json");
  app->Run();
  CK_PROFILE_END_SESSION();

  CK_PROFILE_BEGIN_SESSION("Showdown", "CkProfile-Shutdown.json");
  delete app;
  CK_PROFILE_END_SESSION();
}
