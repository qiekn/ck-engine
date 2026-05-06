#include "renderer/shader/slang_compiler.h"

#include <array>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

#include "core/log.h"
#include "debug/profiler.h"

namespace ck::vulkan {

namespace {

std::string SlurpFile(const std::filesystem::path& path) {
  std::ifstream f(path, std::ios::binary);
  if (!f) return {};
  std::stringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

}  // namespace

SlangCompiler::SlangCompiler() {
  CK_PROFILE_FUNCTION();

  if (SLANG_FAILED(slang::createGlobalSession(global_session_.writeRef()))) {
    CK_ENGINE_FATAL("[slang] createGlobalSession failed");
    return;
  }

  std::array<slang::TargetDesc, 1> targets{};
  targets[0].format = SLANG_SPIRV;
  targets[0].profile = global_session_->findProfile("spirv_1_4");

  // EmitSpirvDirectly skips the GLSL roundtrip and lets Slang generate SPIR-V
  // straight from its IR. Required for modern Slang features and faster.
  std::array<slang::CompilerOptionEntry, 1> options{};
  options[0].name = slang::CompilerOptionName::EmitSpirvDirectly;
  options[0].value.kind = slang::CompilerOptionValueKind::Int;
  options[0].value.intValue0 = 1;

  slang::SessionDesc desc{};
  desc.targets = targets.data();
  desc.targetCount = static_cast<SlangInt>(targets.size());
  desc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
  desc.compilerOptionEntries = options.data();
  desc.compilerOptionEntryCount = static_cast<uint32_t>(options.size());

  if (SLANG_FAILED(global_session_->createSession(desc, session_.writeRef()))) {
    CK_ENGINE_FATAL("[slang] createSession failed");
  }
}

SlangCompiler::~SlangCompiler() = default;

std::vector<uint32_t> SlangCompiler::CompileToSpirv(const std::filesystem::path& path) {
  CK_PROFILE_FUNCTION();
  if (!session_) return {};

  std::string source = SlurpFile(path);
  if (source.empty()) {
    CK_ENGINE_ERROR("[slang] failed to read shader: {}", path.string());
    return {};
  }

  std::string module_name = path.stem().string();
  std::string path_str = path.string();

  Slang::ComPtr<slang::IBlob> diagnostics;
  Slang::ComPtr<slang::IModule> module(session_->loadModuleFromSourceString(
      module_name.c_str(), path_str.c_str(), source.c_str(), diagnostics.writeRef()));
  if (diagnostics) {
    CK_ENGINE_ERROR("[slang] {}", static_cast<const char*>(diagnostics->getBufferPointer()));
  }
  if (!module) return {};

  Slang::ComPtr<slang::IBlob> spirv;
  Slang::ComPtr<slang::IBlob> link_diag;
  if (SLANG_FAILED(module->getTargetCode(0, spirv.writeRef(), link_diag.writeRef()))) {
    if (link_diag) {
      CK_ENGINE_ERROR("[slang] {}", static_cast<const char*>(link_diag->getBufferPointer()));
    }
    return {};
  }

  const auto* code = static_cast<const uint32_t*>(spirv->getBufferPointer());
  size_t words = spirv->getBufferSize() / sizeof(uint32_t);
  return std::vector<uint32_t>(code, code + words);
}

}  // namespace ck::vulkan