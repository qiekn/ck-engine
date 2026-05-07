# - -----------------------------------------------------------------------------: ck (engine)
file(GLOB_RECURSE ck_sources CONFIGURE_DEPENDS "*.cpp")
add_library(ck STATIC ${ck_sources})

# Public C++23 module interface (`import ck;`). Coexists with the legacy
# umbrella header at include/ck.h until phase 5.5.4 retires it.
target_sources(ck
  PUBLIC
    FILE_SET ck_modules TYPE CXX_MODULES
    BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/ck.cppm"
)

if(WIN32)
  target_compile_definitions(ck PRIVATE CK_PLATFORM_WINDOWS)
elseif(APPLE)
  target_compile_definitions(ck PRIVATE CK_PLATFORM_MAC)
elseif(UNIX)
  target_compile_definitions(ck PRIVATE CK_PLATFORM_LINUX)
endif()

target_include_directories(ck
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_options(ck PRIVATE $<$<CONFIG:Debug>:-g>)

# PCH must be PRIVATE: with import ck clients pulling std types through
# the module, exposing the engine PCH (which #includes <iostream> etc.)
# gives client TUs two conflicting definitions of std::basic_string et al.
target_precompile_headers(ck PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)

# VMA implementation TU stays out of the engine PCH so VMA's macro
# pollution (VMA_IMPLEMENTATION + dynamic-functions defines) doesn't leak
# into precompiled state. Also silence vk_mem_alloc.h's nullability
# annotations (clang -Wnullability-completeness fires on every PFN entry).
set_source_files_properties(
  "${CMAKE_CURRENT_SOURCE_DIR}/renderer/vulkan/vma.cpp"
  PROPERTIES
    SKIP_PRECOMPILE_HEADERS ON
    COMPILE_OPTIONS "-Wno-nullability-completeness;-Wno-nullability-extension")

target_link_libraries(ck PUBLIC
  glfw
  glm
  imgui
  stb_image
  yaml-cpp
  imguizmo
  box2d
  spdlog_headers
  entt_headers

  # Vulkan stack (consumers will start using these in Phase 3+)
  Vulkan::volk
  Vulkan::cppm
  Vulkan::vma
  Slang::slang
)