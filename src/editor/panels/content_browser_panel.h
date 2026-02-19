#pragma once

#include <filesystem>

namespace ck {

class ContentBrowserPanel {
public:
  ContentBrowserPanel();

  void OnImGuiRender();

private:
  std::filesystem::path current_directory_;
};

}  // namespace ck
