#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

#include <slang-com-ptr.h>
#include <slang.h>

namespace ck::vulkan {

// Runtime Slang -> SPIR-V compiler. One instance per engine; the global Slang
// session is heavyweight (loads the core module) so we hold both the global
// session and a single SPIR-V target session for the lifetime of the renderer.
class SlangCompiler {
public:
  SlangCompiler();
  ~SlangCompiler();

  SlangCompiler(const SlangCompiler&) = delete;
  SlangCompiler& operator=(const SlangCompiler&) = delete;
  SlangCompiler(SlangCompiler&&) = delete;
  SlangCompiler& operator=(SlangCompiler&&) = delete;

  // Compile a single .slang file containing [shader("vertex")] / [shader("fragment")]
  // entry points into a SPIR-V word stream. Returns an empty vector on failure;
  // diagnostics are routed through CK_ENGINE_ERROR.
  std::vector<uint32_t> CompileToSpirv(const std::filesystem::path& path);

private:
  Slang::ComPtr<slang::IGlobalSession> global_session_;
  Slang::ComPtr<slang::ISession> session_;
};

}  // namespace ck::vulkan