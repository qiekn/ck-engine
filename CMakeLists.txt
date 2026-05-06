cmake_minimum_required(VERSION 3.30)

# - -----------------------------------------------------------------------------: Toolchain (must be before project())
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

include(${CMAKE_SOURCE_DIR}/cmake/EnableCxxImportStd.cmake)

project(ck-engine LANGUAGES C CXX)

set(CMAKE_CXX_MODULE_STD ON)

# Clangd / nvim
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# - -----------------------------------------------------------------------------: Variables
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

# - -----------------------------------------------------------------------------: Dependencies
# Disable extras BEFORE add_subdirectory; FORCE so upstream defaults don't win.
# EXCLUDE_FROM_ALL keeps deps' install/test targets out of `all`.
set(GLFW_BUILD_EXAMPLES   OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS      OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS       OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL          OFF CACHE BOOL "" FORCE)

set(GLM_TEST_ENABLE       OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_TESTS       OFF CACHE BOOL "" FORCE)

set(YAML_CPP_BUILD_TESTS  OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS  OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(YAML_CPP_INSTALL      OFF CACHE BOOL "" FORCE)
set(YAML_CPP_FORMAT_SOURCE OFF CACHE BOOL "" FORCE)

set(BOX2D_SAMPLES         OFF CACHE BOOL "" FORCE)
set(BOX2D_UNIT_TESTS      OFF CACHE BOOL "" FORCE)
set(BOX2D_BENCHMARKS      OFF CACHE BOOL "" FORCE)
set(BOX2D_DOCS            OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glfw      EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glad      EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glm       EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/imgui     EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/imguizmo  EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/stb_image EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/yaml-cpp  EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/box2d     EXCLUDE_FROM_ALL)

# - -----------------------------------------------------------------------------: ck (engine)
file(GLOB_RECURSE ck_sources CONFIGURE_DEPENDS "${SRC_DIR}/engine/*.cpp")
add_library(ck STATIC ${ck_sources})

set_target_properties(ck PROPERTIES
  PUBLIC_HEADER "${SRC_DIR}/engine/include/ck.h"
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
    "${SRC_DIR}/engine/include"
    "${SRC_DIR}/engine/"
    "${CMAKE_SOURCE_DIR}/deps/spdlog/include"
    "${CMAKE_SOURCE_DIR}/deps/entt/include"
)

target_compile_options(ck PRIVATE $<$<CONFIG:Debug>:-g>)

target_precompile_headers(ck PUBLIC ${SRC_DIR}/engine/pch.h)

target_link_libraries(ck PUBLIC
  glfw
  glad
  glm
  imgui
  stb_image
  yaml-cpp
  imguizmo
  box2d
)

# - -----------------------------------------------------------------------------: editor
file(GLOB_RECURSE editor_sources CONFIGURE_DEPENDS "${SRC_DIR}/editor/*.cpp")
add_executable(editor ${editor_sources})
target_include_directories(editor PRIVATE "${SRC_DIR}/editor")
target_link_libraries(editor PRIVATE ck)

# - -----------------------------------------------------------------------------: sandbox
file(GLOB_RECURSE sandbox_sources CONFIGURE_DEPENDS "${SRC_DIR}/sandbox/*.cpp")
add_executable(sandbox ${sandbox_sources})
target_include_directories(sandbox PRIVATE "${SRC_DIR}/sandbox")
target_link_libraries(sandbox PRIVATE ck)

if(APPLE)
  target_link_libraries(sandbox PRIVATE "-framework OpenGL")
endif()