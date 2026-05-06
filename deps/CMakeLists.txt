# - -----------------------------------------------------------------------------: Option overrides
# Set BEFORE add_subdirectory; FORCE so upstream defaults don't win.
set(GLFW_BUILD_EXAMPLES   OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS      OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS       OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL          OFF CACHE BOOL "" FORCE)

set(GLM_TEST_ENABLE       OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_TESTS       OFF CACHE BOOL "" FORCE)

set(YAML_CPP_BUILD_TESTS   OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS   OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(YAML_CPP_INSTALL       OFF CACHE BOOL "" FORCE)
set(YAML_CPP_FORMAT_SOURCE OFF CACHE BOOL "" FORCE)

set(BOX2D_SAMPLES         OFF CACHE BOOL "" FORCE)
set(BOX2D_UNIT_TESTS      OFF CACHE BOOL "" FORCE)
set(BOX2D_BENCHMARKS      OFF CACHE BOOL "" FORCE)
set(BOX2D_DOCS            OFF CACHE BOOL "" FORCE)

# - -----------------------------------------------------------------------------: imgui target (defined here because upstream ships no CMakeLists)
# Must precede imguizmo, which references the imgui target.
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
add_library(imgui STATIC
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_demo.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp

  ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
  ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)
target_include_directories(imgui PUBLIC
  ${IMGUI_DIR}
  ${IMGUI_DIR}/backends
)
# Use volk to load Vulkan symbols inside imgui_impl_vulkan instead of vulkan-1.dll directly.
# Keep the engine and imgui consistent on a single loader.
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_VULKAN_USE_VOLK)

# - -----------------------------------------------------------------------------: Library deps with own CMakeLists
add_subdirectory(glfw      EXCLUDE_FROM_ALL)
add_subdirectory(glm       EXCLUDE_FROM_ALL)
add_subdirectory(imguizmo  EXCLUDE_FROM_ALL)
add_subdirectory(stb_image EXCLUDE_FROM_ALL)
add_subdirectory(yaml-cpp  EXCLUDE_FROM_ALL)
add_subdirectory(box2d     EXCLUDE_FROM_ALL)

target_link_libraries(imgui PUBLIC glfw Vulkan::volk)

# - -----------------------------------------------------------------------------: Header-only deps as INTERFACE targets
add_library(spdlog_headers INTERFACE)
target_include_directories(spdlog_headers INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include)

add_library(entt_headers INTERFACE)
target_include_directories(entt_headers INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/entt/include)

# - -----------------------------------------------------------------------------: Don't scan deps for c++ modules (configure speedup)
foreach(dep IN ITEMS glfw glm imgui imguizmo stb_image yaml-cpp box2d)
  if(TARGET ${dep})
    set_target_properties(${dep} PROPERTIES CXX_SCAN_FOR_MODULES OFF)
  endif()
endforeach()