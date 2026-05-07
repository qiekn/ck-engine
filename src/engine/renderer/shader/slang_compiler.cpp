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

// Slang's IBlob diagnostics carry a mix of warning / error / note lines in
// the format "path(line): severity NNNN: message" with continuation lines
// indented underneath. Walk the blob line-by-line, classify each diagnostic
// group by the first severity tag we see, and route to the matching ck::log
// channel. This stops benign warnings (e.g., profile auto-promotion) from
// showing up red in the console.
void LogSlangDiagnostics(slang::IBlob* blob) {
  if (!blob || blob->getBufferSize() == 0) return;
  std::string_view all(static_cast<const char*>(blob->getBufferPointer()),
                       blob->getBufferSize());

  enum class Severity { Info, Warn, Error };
  Severity current = Severity::Error;  // default for unrecognized output

  size_t pos = 0;
  while (pos <= all.size()) {
    size_t eol = all.find('\n', pos);
    if (eol == std::string_view::npos) eol = all.size();
    std::string_view line = all.substr(pos, eol - pos);
    if (!line.empty() && line.back() == '\r') line.remove_suffix(1);

    if (!line.empty()) {
      // Classify only on header lines (those that contain ": severity ").
      if (line.find(": warning ") != std::string_view::npos) {
        current = Severity::Warn;
      } else if (line.find(": error ") != std::string_view::npos) {
        current = Severity::Error;
      } else if (line.find(": note ") != std::string_view::npos) {
        current = Severity::Info;
      }
      // Continuation lines stay on |current|.

      switch (current) {
        case Severity::Info:  ck::log::info ("[slang] {}", line); break;
        case Severity::Warn:  ck::log::warn ("[slang] {}", line); break;
        case Severity::Error: ck::log::error("[slang] {}", line); break;
      }
    }

    if (eol == all.size()) break;
    pos = eol + 1;
  }
}

}  // namespace

SlangCompiler::SlangCompiler() {
  CK_PROFILE_FUNCTION();

  if (SLANG_FAILED(slang::createGlobalSession(global_session_.writeRef()))) {
    ck::log::fatal("[slang] createGlobalSession failed");
    return;
  }

  std::array<slang::TargetDesc, 1> targets{};
  targets[0].format = SLANG_SPIRV;
  // spirv_1_5 has SPV_EXT_descriptor_indexing capabilities promoted, so
  // Renderer2D's NonUniformResourceIndex usage no longer trips the
  // "auto-promoting profile" warning that fires under spirv_1_4.
  targets[0].profile = global_session_->findProfile("spirv_1_5");

  // EmitSpirvDirectly skips the GLSL roundtrip and lets Slang generate SPIR-V
  // straight from its IR. Required for modern Slang features and faster.
  // 41012 is the "auto-promoting profile" warning: Slang chains in capabilities
  // (image-query, gather-extended, sparse-residency, ...) for any fragment
  // shader regardless of whether they're used. We don't need the noise.
  const char* kDisable41012 = "41012";
  std::array<slang::CompilerOptionEntry, 2> options{};
  options[0].name = slang::CompilerOptionName::EmitSpirvDirectly;
  options[0].value.kind = slang::CompilerOptionValueKind::Int;
  options[0].value.intValue0 = 1;
  options[1].name = slang::CompilerOptionName::DisableWarning;
  options[1].value.kind = slang::CompilerOptionValueKind::String;
  options[1].value.stringValue0 = kDisable41012;

  slang::SessionDesc desc{};
  desc.targets = targets.data();
  desc.targetCount = static_cast<SlangInt>(targets.size());
  desc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
  desc.compilerOptionEntries = options.data();
  desc.compilerOptionEntryCount = static_cast<uint32_t>(options.size());

  if (SLANG_FAILED(global_session_->createSession(desc, session_.writeRef()))) {
    ck::log::fatal("[slang] createSession failed");
  }
}

SlangCompiler::~SlangCompiler() = default;

std::vector<uint32_t> SlangCompiler::CompileToSpirv(const std::filesystem::path& path) {
  CK_PROFILE_FUNCTION();
  if (!session_) return {};

  std::string source = SlurpFile(path);
  if (source.empty()) {
    ck::log::error("[slang] failed to read shader: {}", path.string());
    return {};
  }

  std::string module_name = path.stem().string();
  std::string path_str = path.string();

  Slang::ComPtr<slang::IBlob> diagnostics;
  Slang::ComPtr<slang::IModule> module(session_->loadModuleFromSourceString(
      module_name.c_str(), path_str.c_str(), source.c_str(), diagnostics.writeRef()));
  LogSlangDiagnostics(diagnostics);
  if (!module) return {};

  Slang::ComPtr<slang::IBlob> spirv;
  Slang::ComPtr<slang::IBlob> link_diag;
  if (SLANG_FAILED(module->getTargetCode(0, spirv.writeRef(), link_diag.writeRef()))) {
    LogSlangDiagnostics(link_diag);
    return {};
  }

  const auto* code = static_cast<const uint32_t*>(spirv->getBufferPointer());
  size_t words = spirv->getBufferSize() / sizeof(uint32_t);
  return std::vector<uint32_t>(code, code + words);
}

}  // namespace ck::vulkan