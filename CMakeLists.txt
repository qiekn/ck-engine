cmake_minimum_required(VERSION 3.30)

# - -----------------------------------------------------------------------------: Toolchain (must be before project())
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

include(${CMAKE_SOURCE_DIR}/cmake/EnableCxxImportStd.cmake)

project(ck-engine LANGUAGES C CXX)

set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Keep all executables at build/<preset>/ so run scripts and asset paths stay simple
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# - -----------------------------------------------------------------------------: Subdirectories
add_subdirectory(deps)
add_subdirectory(src/engine)
add_subdirectory(src/editor)
add_subdirectory(src/sandbox)