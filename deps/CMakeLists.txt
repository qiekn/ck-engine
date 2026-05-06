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

# - -----------------------------------------------------------------------------: Library deps with own CMakeLists
add_subdirectory(glfw      EXCLUDE_FROM_ALL)
add_subdirectory(glad      EXCLUDE_FROM_ALL)
add_subdirectory(glm       EXCLUDE_FROM_ALL)
add_subdirectory(imgui     EXCLUDE_FROM_ALL)
add_subdirectory(imguizmo  EXCLUDE_FROM_ALL)
add_subdirectory(stb_image EXCLUDE_FROM_ALL)
add_subdirectory(yaml-cpp  EXCLUDE_FROM_ALL)
add_subdirectory(box2d     EXCLUDE_FROM_ALL)

# - -----------------------------------------------------------------------------: Header-only deps as INTERFACE targets
add_library(spdlog_headers INTERFACE)
target_include_directories(spdlog_headers INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include)

add_library(entt_headers INTERFACE)
target_include_directories(entt_headers INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/entt/include)