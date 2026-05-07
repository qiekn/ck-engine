// Engine main-loop driver. Clients write a one-line main() that calls
// ck::EntryPoint(argc, argv); they no longer need #include "entry_point.h"
// or transitive std headers in their TU, which used to collide with
// `import ck;` (libc++ chrono / array template default args got
// redeclared between the GMF and the client TU).
#include "core/application.h"
#include "core/log.h"
#include "debug/profiler.h"

namespace ck {

int EntryPoint(int argc, char** argv) {
  Log::Init();

  CK_PROFILE_BEGIN_SESSION("Startup", "CkProfile-Startup.json");
  auto* app = CreateApplication({argc, argv});
  CK_PROFILE_END_SESSION();

  CK_PROFILE_BEGIN_SESSION("Runtime", "CkProfile-Runtime.json");
  app->Run();
  CK_PROFILE_END_SESSION();

  CK_PROFILE_BEGIN_SESSION("Shutdown", "CkProfile-Shutdown.json");
  delete app;
  CK_PROFILE_END_SESSION();
  return 0;
}

}  // namespace ck