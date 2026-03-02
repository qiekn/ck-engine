cmake_minimum_required(VERSION 3.24...3.30)
project(ck-engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Clangd Lsp Support for nvim
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Debug symbols
if(NOT MSVC)
  set(CMAKE_BUILD_TYPE DEBUG)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -gdwarf-4")
endif()

# Default to a release build
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()


# Variables
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

# Mono (C# scripting runtime) — vendored in deps/mono/
set(MONO_DIR "${CMAKE_SOURCE_DIR}/deps/mono")
set(MONO_INCLUDE_DIR "${MONO_DIR}/include")
set(MONO_LIB_DIR "${MONO_DIR}/lib")
set(MONO_BIN_DIR "${MONO_DIR}/bin")

if(WIN32)
  if(MSVC)
    set(MONO_IMPORT_LIB "${MONO_LIB_DIR}/monosgen-2.0.lib")
  else()
    set(MONO_IMPORT_LIB "${MONO_LIB_DIR}/libmonosgen-2.0.dll.a")
  endif()
  set(MONO_SYSTEM_LIBS ws2_32 winmm version bcrypt)

  # Copy Mono runtime DLL to build directory
  file(COPY "${MONO_BIN_DIR}/libmonosgen-2.0.dll" DESTINATION "${CMAKE_BINARY_DIR}")
  if(MSVC)
    # MSVC multi-config generators put executables in build/<Config>/
    file(COPY "${MONO_BIN_DIR}/libmonosgen-2.0.dll" DESTINATION "${CMAKE_BINARY_DIR}/Debug")
    file(COPY "${MONO_BIN_DIR}/libmonosgen-2.0.dll" DESTINATION "${CMAKE_BINARY_DIR}/Release")
  endif()
elseif(APPLE)
  # macOS: use system-installed Mono from Homebrew or Mono framework
  find_library(MONO_IMPORT_LIB NAMES monosgen-2.0 REQUIRED)
  set(MONO_SYSTEM_LIBS pthread)
else()
  # Linux: use system-installed Mono
  find_library(MONO_IMPORT_LIB NAMES monosgen-2.0 REQUIRED)
  set(MONO_SYSTEM_LIBS pthread dl m)
endif()

#---------------------------------------------------------------------3
#                              CK Engine                              |
#---------------------------------------------------------------------3

# Define the ck-engine static library
file(GLOB_RECURSE ck_sources CONFIGURE_DEPENDS "${SRC_DIR}/engine/*.cpp")
add_library(ck STATIC
  ${ck_sources}
)

set_target_properties(ck PROPERTIES
  PUBLIC_HEADER "${SRC_DIR}/engine/include/ck.h"
)

target_compile_definitions(ck PRIVATE CK_PLATFORM_MAC)

# Include directories for ck
target_include_directories(ck
  PUBLIC
    "${SRC_DIR}/engine/include"
    "${SRC_DIR}/engine/"

  # Header-only libraries
  PUBLIC
    "${CMAKE_SOURCE_DIR}/deps/spdlog/include"
    "${CMAKE_SOURCE_DIR}/deps/entt/include"

  # Mono (C# scripting)
  PUBLIC
    "${MONO_INCLUDE_DIR}"
)

# Precompiled Headers
target_precompile_headers(ck PUBLIC ${SRC_DIR}/engine/pch.h)

# Dependencies
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glfw)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glad)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glm)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/imgui)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/imguizmo)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/stb_image)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/yaml-cpp)
option(BOX2D_SAMPLES "Build the Box2D samples" OFF)
option(BOX2D_UNIT_TESTS "Build the Box2D unit tests" OFF)
option(BOX2D_BENCHMARKS "Build the Box2D benchmarks" OFF)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/box2d)
target_link_libraries(ck PUBLIC
  glfw
  glad
  glm
  imgui
  stb_image
  yaml-cpp
  imguizmo
  box2d
  ${MONO_IMPORT_LIB}
  ${MONO_SYSTEM_LIBS}
)

#---------------------------------------------------------------------3
#                               Editor                                |
#---------------------------------------------------------------------3

set(EDITOR_NAME editor)
# Define sandbox application
file(GLOB_RECURSE editor_sources CONFIGURE_DEPENDS "${SRC_DIR}/editor/*.cpp")
add_executable(${EDITOR_NAME} ${editor_sources})

# Include directories for the sandbox application
target_include_directories(${EDITOR_NAME} PUBLIC
  "${SRC_DIR}/sandbox"
)

# Link sandbox with the ck library
target_link_libraries(${EDITOR_NAME} PRIVATE ck)

#---------------------------------------------------------------------3
#                               SandBox                               |
#---------------------------------------------------------------------3

# Define sandbox application
file(GLOB_RECURSE sandbox_sources CONFIGURE_DEPENDS "${SRC_DIR}/sandbox/*.cpp")
add_executable(sandbox ${sandbox_sources})

# Include directories for the sandbox application
target_include_directories(sandbox PUBLIC
  "${SRC_DIR}/sandbox"
)

# Link sandbox with the ck library
target_link_libraries(sandbox PRIVATE ck)

if (APPLE)
  target_link_libraries(sandbox PRIVATE "-framework OpenGL")
endif()
