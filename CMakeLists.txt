cmake_minimum_required(VERSION 3.24...3.30)
project(ck-engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Clangd Lsp Support for nvim
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to a release build
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Variables
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

#---------------------------------------------------------------------3
#                              CK Engine                              |
#---------------------------------------------------------------------3

# Define the ck-engine static library
file(GLOB_RECURSE ck_sources CONFIGURE_DEPENDS "${SRC_DIR}/ck/*.cpp")
add_library(ck STATIC
  ${ck_sources}
)

target_compile_definitions(ck PRIVATE CK_PLATFORM_MAC)

# Include directories for ck
target_include_directories(ck
  PUBLIC
    "${SRC_DIR}/ck/include"
    "${SRC_DIR}/ck/"
    "${CMAKE_SOURCE_DIR}/deps/spdlog/include"
)

# Precompiled Headers
target_precompile_headers(ck PUBLIC ${SRC_DIR}/ck/pch.h)

# Dependencies
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glfw)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glad)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glm)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/imgui)
target_link_libraries(ck PUBLIC
  glfw
  glad
  glm
  imgui
)

#---------------------------------------------------------------------3
#                               SandBox                               |
#---------------------------------------------------------------------3

# Define sandbox application
file(GLOB_RECURSE sandbox_sources CONFIGURE_DEPENDS "${SRC_DIR}/sandbox/*.cpp") # source files
add_executable(sandbox ${sandbox_sources})

# Include directories for the sandbox application
target_include_directories(sandbox PUBLIC "${SRC_DIR}/sandbox")

# Link sandbox with the ck library
target_link_libraries(sandbox PUBLIC ck)

if (APPLE)
  target_link_libraries(sandbox PRIVATE "-framework OpenGL")
endif()
