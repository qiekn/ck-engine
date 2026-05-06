# - -----------------------------------------------------------------------------: ck (engine)
file(GLOB_RECURSE ck_sources CONFIGURE_DEPENDS "*.cpp")
add_library(ck STATIC ${ck_sources})

set_target_properties(ck PROPERTIES
  PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/ck.h"
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
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_options(ck PRIVATE $<$<CONFIG:Debug>:-g>)

target_precompile_headers(ck PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)

target_link_libraries(ck PUBLIC
  glfw
  glad
  glm
  imgui
  stb_image
  yaml-cpp
  imguizmo
  box2d
  spdlog_headers
  entt_headers
)