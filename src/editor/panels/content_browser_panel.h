#pragma once

#include <filesystem>

#include "renderer/texture.h"

namespace ck {

class ContentBrowserPanel {
public:
  ContentBrowserPanel();

  void OnImGuiRender();

private:
  std::filesystem::path current_directory_;

  Ref<Texture2D> directory_icon_;
  Ref<Texture2D> file_icon_;
};

}  // namespace ck
