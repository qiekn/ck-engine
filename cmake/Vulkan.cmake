# Centralised Vulkan stack discovery: vulkan loader + volk + Vulkan-Hpp module + VMA + Slang.
# Must be included AFTER project().

# - -----------------------------------------------------------------------------: Vulkan + volk
find_package(Vulkan REQUIRED COMPONENTS volk)

# FindVulkan only adds ${Vulkan_INCLUDE_DIRS} to Vulkan::volk, but the SDK ships
# volk.h under Include/Volk/. Make <volk.h> directly resolvable.
if(EXISTS "${Vulkan_INCLUDE_DIRS}/Volk/volk.h")
  set_property(TARGET Vulkan::volk APPEND PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}/Volk")
endif()

# - -----------------------------------------------------------------------------: Vulkan-Hpp C++ module (Vulkan::cppm)
add_library(VulkanCppModule)
add_library(Vulkan::cppm ALIAS VulkanCppModule)

target_compile_definitions(VulkanCppModule PUBLIC
  VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
  VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
  VULKAN_HPP_CXX_MODULE_EXPERIMENTAL_WARNING=1
)

target_include_directories(VulkanCppModule PRIVATE "${Vulkan_INCLUDE_DIRS}")
target_link_libraries(VulkanCppModule PUBLIC Vulkan::Vulkan)

target_sources(VulkanCppModule
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
    BASE_DIRS "${Vulkan_INCLUDE_DIRS}"
    FILES "${Vulkan_INCLUDE_DIRS}/vulkan/vulkan.cppm"
          "${Vulkan_INCLUDE_DIRS}/vulkan/vulkan_video.cppm"
)

# - -----------------------------------------------------------------------------: VMA (header-only, vendored in Vulkan SDK)
add_library(VulkanMemoryAllocator INTERFACE)
add_library(Vulkan::vma ALIAS VulkanMemoryAllocator)
target_include_directories(VulkanMemoryAllocator INTERFACE "${Vulkan_INCLUDE_DIRS}/vma")
# Define VMA_IMPLEMENTATION in exactly one .cpp before #include "vk_mem_alloc.h".

# - -----------------------------------------------------------------------------: Slang (runtime shader compiler, from Vulkan SDK)
find_library(Slang_LIBRARY
  NAMES slang
  HINTS "$ENV{VULKAN_SDK}/Lib" "$ENV{VULKAN_SDK}/lib"
  REQUIRED
)
find_path(Slang_INCLUDE_DIR
  NAMES slang.h
  HINTS "$ENV{VULKAN_SDK}/Include/slang" "$ENV{VULKAN_SDK}/include/slang"
  REQUIRED
)

add_library(Slang::slang INTERFACE IMPORTED)
set_target_properties(Slang::slang PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${Slang_INCLUDE_DIR}"
  INTERFACE_LINK_LIBRARIES      "${Slang_LIBRARY}"
)

message(STATUS "Vulkan ${Vulkan_VERSION} at ${Vulkan_INCLUDE_DIRS}")
message(STATUS "Slang library: ${Slang_LIBRARY}")