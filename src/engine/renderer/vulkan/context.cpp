#include "context.h"

#include <volk.h>

#include <GLFW/glfw3.h>

#include <cstdint>
#include <string_view>
#include <vector>

#include "core/log.h"
#include "core/window.h"
#include "debug/profiler.h"

// Storage for the dynamic dispatcher. Must appear in exactly one TU.
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace ck::vulkan {

namespace {

#ifndef NDEBUG
constexpr bool kEnableValidation = true;
#else
constexpr bool kEnableValidation = false;
#endif

VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT /*type*/,
    const vk::DebugUtilsMessengerCallbackDataEXT* data,
    void* /*user_data*/) {
  using Sev = vk::DebugUtilsMessageSeverityFlagBitsEXT;
  switch (severity) {
    case Sev::eVerbose: CK_ENGINE_TRACE("[vk] {}", data->pMessage); break;
    case Sev::eInfo:    CK_ENGINE_INFO ("[vk] {}", data->pMessage); break;
    case Sev::eWarning: CK_ENGINE_WARN ("[vk] {}", data->pMessage); break;
    case Sev::eError:   CK_ENGINE_ERROR("[vk] {}", data->pMessage); break;
    default: break;
  }
  return VK_FALSE;
}

bool HasLayer(std::string_view name, const std::vector<vk::LayerProperties>& layers) {
  for (auto const& l : layers) {
    if (name == l.layerName.data()) return true;
  }
  return false;
}

uint32_t FindGraphicsFamily(vk::PhysicalDevice pd, vk::SurfaceKHR surface) {
  auto qfp = pd.getQueueFamilyProperties();
  for (uint32_t i = 0; i < qfp.size(); ++i) {
    bool gfx = static_cast<bool>(qfp[i].queueFlags & vk::QueueFlagBits::eGraphics);
    bool present = pd.getSurfaceSupportKHR(i, surface);
    if (gfx && present) return i;
  }
  return ~0u;
}

vk::PhysicalDevice PickPhysicalDevice(vk::Instance inst) {
  auto devices = inst.enumeratePhysicalDevices();
  if (devices.empty()) {
    CK_ENGINE_FATAL("No Vulkan-capable GPU found");
    return nullptr;
  }
  for (auto pd : devices) {
    if (pd.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu) return pd;
  }
  return devices.front();
}

}  // namespace

Context::Context(Window& window) {
  CK_PROFILE_FUNCTION();

  // 1. volk -> dispatcher
  if (volkInitialize() != VK_SUCCESS) {
    CK_ENGINE_FATAL("volkInitialize() failed - Vulkan loader not found");
    return;
  }
  VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

  // 2. Required instance extensions + layers
  uint32_t glfw_ext_count = 0;
  const char** glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_count);
  std::vector<const char*> instance_exts(glfw_exts, glfw_exts + glfw_ext_count);

  std::vector<const char*> layers;
  if (kEnableValidation) {
    auto avail = vk::enumerateInstanceLayerProperties();
    if (HasLayer("VK_LAYER_KHRONOS_validation", avail)) {
      layers.push_back("VK_LAYER_KHRONOS_validation");
      instance_exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    } else {
      CK_ENGINE_WARN("VK_LAYER_KHRONOS_validation not available");
    }
  }

  // 3. Instance
  vk::ApplicationInfo app_info{};
  app_info.pApplicationName = "ck-engine";
  app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
  app_info.pEngineName = "ck";
  app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
  app_info.apiVersion = VK_API_VERSION_1_3;

  vk::InstanceCreateInfo ici{};
  ici.pApplicationInfo = &app_info;
  ici.enabledLayerCount = static_cast<uint32_t>(layers.size());
  ici.ppEnabledLayerNames = layers.data();
  ici.enabledExtensionCount = static_cast<uint32_t>(instance_exts.size());
  ici.ppEnabledExtensionNames = instance_exts.data();

  instance_ = vk::createInstance(ici);
  VULKAN_HPP_DEFAULT_DISPATCHER.init(instance_);
  volkLoadInstance(instance_);

  // 4. Debug messenger (only if validation layer is on)
  if (!layers.empty()) {
    using Sev = vk::DebugUtilsMessageSeverityFlagBitsEXT;
    using Type = vk::DebugUtilsMessageTypeFlagBitsEXT;
    vk::DebugUtilsMessengerCreateInfoEXT dmi{};
    dmi.messageSeverity = Sev::eVerbose | Sev::eInfo | Sev::eWarning | Sev::eError;
    dmi.messageType = Type::eGeneral | Type::eValidation | Type::ePerformance;
    dmi.pfnUserCallback = VkDebugCallback;
    debug_messenger_ = instance_.createDebugUtilsMessengerEXT(dmi);
  }

  // 5. Surface (GLFW does the platform-specific call)
  VkSurfaceKHR raw_surface = VK_NULL_HANDLE;
  auto* glfw_window = static_cast<GLFWwindow*>(window.GetNativeWindow());
  if (glfwCreateWindowSurface(instance_, glfw_window, nullptr, &raw_surface) != VK_SUCCESS) {
    CK_ENGINE_FATAL("glfwCreateWindowSurface failed");
    return;
  }
  surface_ = raw_surface;

  // 6. Physical device
  physical_device_ = PickPhysicalDevice(instance_);
  if (!physical_device_) return;
  auto props = physical_device_.getProperties();
  CK_ENGINE_INFO("GPU: {}", static_cast<const char*>(props.deviceName));

  // 7. Queue family (single family, graphics + present)
  graphics_family_ = FindGraphicsFamily(physical_device_, surface_);
  if (graphics_family_ == ~0u) {
    CK_ENGINE_FATAL("No graphics+present queue family found");
    return;
  }
  CK_ENGINE_INFO("Queue family (graphics+present): {}", graphics_family_);

  // 8. Logical device with Vulkan 1.3 dynamic rendering + sync2
  float queue_priority = 1.0f;
  vk::DeviceQueueCreateInfo qci{};
  qci.queueFamilyIndex = graphics_family_;
  qci.queueCount = 1;
  qci.pQueuePriorities = &queue_priority;

  std::vector<const char*> device_exts = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  // Vulkan 1.1: shaderDrawParameters is required because Slang's SPIR-V backend
  // declares the DrawParameters capability when translating SV_VertexID.
  vk::PhysicalDeviceVulkan11Features features11{};
  features11.shaderDrawParameters = VK_TRUE;

  vk::PhysicalDeviceVulkan13Features features13{};
  features13.pNext = &features11;
  features13.dynamicRendering = VK_TRUE;
  features13.synchronization2 = VK_TRUE;

  vk::DeviceCreateInfo dci{};
  dci.pNext = &features13;
  dci.queueCreateInfoCount = 1;
  dci.pQueueCreateInfos = &qci;
  dci.enabledExtensionCount = static_cast<uint32_t>(device_exts.size());
  dci.ppEnabledExtensionNames = device_exts.data();

  device_ = physical_device_.createDevice(dci);
  VULKAN_HPP_DEFAULT_DISPATCHER.init(device_);
  volkLoadDevice(device_);

  graphics_queue_ = device_.getQueue(graphics_family_, 0);

  CK_ENGINE_INFO("Vulkan context ready");
}

Context::~Context() {
  CK_PROFILE_FUNCTION();
  if (device_) device_.destroy();
  if (instance_) {
    if (surface_) instance_.destroySurfaceKHR(surface_);
    if (debug_messenger_) instance_.destroyDebugUtilsMessengerEXT(debug_messenger_);
    instance_.destroy();
  }
}

}  // namespace ck::vulkan