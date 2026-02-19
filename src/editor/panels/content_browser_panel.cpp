#include "content_browser_panel.h"

#include <imgui.h>

namespace ck {

// TODO: change this once we have a project system
static const std::filesystem::path kAssetPath = "assets";

ContentBrowserPanel::ContentBrowserPanel() : current_directory_(kAssetPath) {
  directory_icon_ = Texture2D::Create("assets/icons/content_browser/folder.png");
  file_icon_ = Texture2D::Create("assets/icons/content_browser/file.png");
}

void ContentBrowserPanel::OnImGuiRender() {
  ImGui::Begin("Content Browser");

  if (current_directory_ != std::filesystem::path(kAssetPath)) {
    if (ImGui::Button("<-")) {
      current_directory_ = current_directory_.parent_path();
    }
  }

  static float padding = 16.0f;
  static float thumbnail_size = 128.0f;
  float cell_size = thumbnail_size + padding + ImGui::GetStyle().FramePadding.x * 2.0f;

  float panel_width = ImGui::GetContentRegionAvail().x;
  int column_count = (int)(panel_width / cell_size);
  if (column_count < 1) column_count = 1;

  ImGui::Columns(column_count, 0, false);

  for (auto& directory_entry : std::filesystem::directory_iterator(current_directory_)) {
    const auto& path = directory_entry.path();
    auto relative_path = std::filesystem::relative(path, kAssetPath);
    std::string filename_string = relative_path.filename().string();

    ImGui::PushID(filename_string.c_str());
    Ref<Texture2D> icon = directory_entry.is_directory() ? directory_icon_ : file_icon_;
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::ImageButton("##thumbnail", (ImTextureID)(uint64_t)icon->GetRendererID(),
                       {thumbnail_size, thumbnail_size}, {0, 1}, {1, 0});
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
      if (directory_entry.is_directory()) current_directory_ /= path.filename();
    }
    ImGui::TextWrapped("%s", filename_string.c_str());
    ImGui::PopStyleColor();
    ImGui::PopID();

    ImGui::NextColumn();
  }

  ImGui::Columns(1);

  ImGui::SliderFloat("Thumbnail Size", &thumbnail_size, 16, 512);
  ImGui::SliderFloat("Padding", &padding, 0, 32);

  // TODO: status bar
  ImGui::End();
}

}  // namespace ck
