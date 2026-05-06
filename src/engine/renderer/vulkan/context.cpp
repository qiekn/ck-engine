#include "context.h"

#define VK_NO_PROTOTYPES
#include <volk.h>
#include <vulkan/vulkan.hpp>

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

struct Context::Impl {
  vk::Instance instance;
  vk::DebugUtilsMessengerEXT debug_messenger;
  vk::SurfaceKHR surface;
  vk::PhysicalDevice physical_device;
  vk::Device device;
  uint32_t graphics_family = ~0u;
  vk::Queue graphics_queue;

  ~Impl() {
    if (device) device.destroy();
    if (instance) {
      if (surface) instance.destroySurfaceKHR(surface);
      if (debug_messenger) instance.destroyDebugUtilsMessengerEXT(debug_messenger);
      instance.destroy();
    }
  }
};

Context::Context(Window& window) : impl_(CreateScope<Impl>()) {
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

  impl_->instance = vk::createInstance(ici);
  VULKAN_HPP_DEFAULT_DISPATCHER.init(impl_->instance);
  volkLoadInstance(impl_->instance);

  // 4. Debug messenger (only if validation layer is on)
  if (!layers.empty()) {
    using Sev = vk::DebugUtilsMessageSeverityFlagBitsEXT;
    using Type = vk::DebugUtilsMessageTypeFlagBitsEXT;
    vk::DebugUtilsMessengerCreateInfoEXT dmi{};
    dmi.messageSeverity = Sev::eVerbose | Sev::eInfo | Sev::eWarning | Sev::eError;
    dmi.messageType = Type::eGeneral | Type::eValidation | Type::ePerformance;
    dmi.pfnUserCallback = VkDebugCallback;
    impl_->debug_messenger = impl_->instance.createDebugUtilsMessengerEXT(dmi);
  }

  // 5. Surface (GLFW does the platform-specific call)
  VkSurfaceKHR raw_surface = VK_NULL_HANDLE;
  auto* glfw_window = static_cast<GLFWwindow*>(window.GetNativeWindow());
  if (glfwCreateWindowSurface(impl_->instance, glfw_window, nullptr, &raw_surface) != VK_SUCCESS) {
    CK_ENGINE_FATAL("glfwCreateWindowSurface failed");
    return;
  }
  impl_->surface = raw_surface;

  // 6. Physical device
  impl_->physical_device = PickPhysicalDevice(impl_->instance);
  if (!impl_->physical_device) return;
  auto props = impl_->physical_device.getProperties();
  CK_ENGINE_INFO("GPU: {}", static_cast<const char*>(props.deviceName));

  // 7. Queue family (single family, graphics + present)
  impl_->graphics_family = FindGraphicsFamily(impl_->physical_device, impl_->surface);
  if (impl_->graphics_family == ~0u) {
    CK_ENGINE_FATAL("No graphics+present queue family found");
    return;
  }
  CK_ENGINE_INFO("Queue family (graphics+present): {}", impl_->graphics_family);

  // 8. Logical device with Vulkan 1.3 dynamic rendering + sync2
  float queue_priority = 1.0f;
  vk::DeviceQueueCreateInfo qci{};
  qci.queueFamilyIndex = impl_->graphics_family;
  qci.queueCount = 1;
  qci.pQueuePriorities = &queue_priority;

  std::vector<const char*> device_exts = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  vk::PhysicalDeviceVulkan13Features features13{};
  features13.dynamicRendering = VK_TRUE;
  features13.synchronization2 = VK_TRUE;

  vk::DeviceCreateInfo dci{};
  dci.pNext = &features13;
  dci.queueCreateInfoCount = 1;
  dci.pQueueCreateInfos = &qci;
  dci.enabledExtensionCount = static_cast<uint32_t>(device_exts.size());
  dci.ppEnabledExtensionNames = device_exts.data();

  impl_->device = impl_->physical_device.createDevice(dci);
  VULKAN_HPP_DEFAULT_DISPATCHER.init(impl_->device);
  volkLoadDevice(impl_->device);

  impl_->graphics_queue = impl_->device.getQueue(impl_->graphics_family, 0);

  CK_ENGINE_INFO("Vulkan context ready");
}

Context::~Context() {
  CK_PROFILE_FUNCTION();
}

}  // namespace ck::vulkan