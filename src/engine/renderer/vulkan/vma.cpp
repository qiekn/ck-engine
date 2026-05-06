// VMA implementation TU. VMA_IMPLEMENTATION must appear in exactly one
// translation unit before #include <vk_mem_alloc.h>.
//
// We pair VMA with volk: VMA_STATIC_VULKAN_FUNCTIONS=0 + VMA_DYNAMIC=1
// means VMA loads its own vulkan entry points via the
// vkGetInstanceProcAddr / vkGetDeviceProcAddr we hand it (volk-loaded).
//
// This file opts out of the engine PCH (see src/engine/CMakeLists.txt) to
// keep VMA's macro pollution out of the precompiled header.

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#include <volk.h>

#include <vk_mem_alloc.h>