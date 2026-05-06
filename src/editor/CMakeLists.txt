file(GLOB_RECURSE editor_sources CONFIGURE_DEPENDS "*.cpp")
add_executable(editor ${editor_sources})
target_include_directories(editor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(editor PRIVATE ck)