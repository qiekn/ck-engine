#pragma once

#include <optional>
#include <string>

namespace ck {
class FileDialogs {
public:
  // These return empty strings if cancelled
  static std::optional<std::string> OpenFile(const char* filter);
  static std::optional<std::string> SaveFile(const char* filter);
};
}  // namespace ck
