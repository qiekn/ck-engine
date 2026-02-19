#include "content_browser_panel.h"

#include <imgui.h>

namespace ck {

// TODO: change this once we have a project system
static const std::filesystem::path kAssetPath = "assets";

ContentBrowserPanel::ContentBrowserPanel() : current_directory_(kAssetPath) {}

void ContentBrowserPanel::OnImGuiRender() {
  ImGui::Begin("Content Browser");

  if (current_directory_ != std::filesystem::path(kAssetPath)) {
    if (ImGui::Button("<-")) {
      current_directory_ = current_directory_.parent_path();
    }
  }

  for (auto& directory_entry : std::filesystem::directory_iterator(current_directory_)) {
    const auto& path = directory_entry.path();
    auto relative_path = std::filesystem::relative(path, kAssetPath);
    std::string filename_string = relative_path.filename().string();
    if (directory_entry.is_directory()) {
      if (ImGui::Button(filename_string.c_str())) {
        current_directory_ /= path.filename();
      }
    } else {
      if (ImGui::Button(filename_string.c_str())) {
      }
    }
  }

  ImGui::End();
}

}  // namespace ck
