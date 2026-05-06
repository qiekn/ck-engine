file(GLOB_RECURSE sandbox_sources CONFIGURE_DEPENDS "*.cpp")
add_executable(sandbox ${sandbox_sources})
target_include_directories(sandbox PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(sandbox PRIVATE ck)