#include "imgui_layer.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <volk.h>
#include <GLFW/glfw3.h>

#include "core/application.h"
#include "core/log.h"
#include "core/window.h"
#include "renderer/renderer.h"
#include "renderer/vulkan/context.h"
#include "renderer/vulkan/swapchain.h"

namespace ck {

namespace {

void CheckVkResult(VkResult err) {
  if (err == VK_SUCCESS) return;
  ck::log::error("[imgui-vk] VkResult = {}", static_cast<int>(err));
}

}  // namespace

ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {}
ImGuiLayer::~ImGuiLayer() = default;

void ImGuiLayer::OnAttach() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  // Multi-viewport intentionally off — see phase-6-plan.md §6.A decisions.

  ImGui::StyleColorsDark();

  Renderer& renderer = Application::Get().GetRenderer();
  vulkan::Context& ctx = renderer.context();
  vulkan::Swapchain& swapchain = renderer.swapchain();

  auto* glfw_window = static_cast<GLFWwindow*>(
      Application::Get().GetWindow()->GetNativeWindow());
  ImGui_ImplGlfw_InitForVulkan(glfw_window, true);

  // Dynamic-rendering pipeline create info: matches Renderer's swapchain pass.
  VkFormat color_format = static_cast<VkFormat>(swapchain.format());
  VkPipelineRenderingCreateInfoKHR pri{};
  pri.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
  pri.colorAttachmentCount = 1;
  pri.pColorAttachmentFormats = &color_format;

  ImGui_ImplVulkan_InitInfo info{};
  info.ApiVersion = VK_API_VERSION_1_3;
  info.Instance = static_cast<VkInstance>(ctx.instance());
  info.PhysicalDevice = static_cast<VkPhysicalDevice>(ctx.physical_device());
  info.Device = static_cast<VkDevice>(ctx.device());
  info.QueueFamily = ctx.graphics_family();
  info.Queue = static_cast<VkQueue>(ctx.graphics_queue());
  info.PipelineCache = static_cast<VkPipelineCache>(ctx.pipeline_cache());
  info.DescriptorPool = VK_NULL_HANDLE;
  info.DescriptorPoolSize = 128;  // imgui owns the pool internally
  info.MinImageCount = swapchain.image_count();
  info.ImageCount = swapchain.image_count();
  info.UseDynamicRendering = true;
  info.PipelineInfoMain.PipelineRenderingCreateInfo = pri;
  info.CheckVkResultFn = CheckVkResult;

  if (!ImGui_ImplVulkan_Init(&info)) {
    ck::log::error("ImGui_ImplVulkan_Init failed");
    return;
  }

  // Renderer drives the actual draw recording inside its EndFrame swapchain pass.
  renderer.SetImGuiRenderCallback([](vk::CommandBuffer cmd) {
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                    static_cast<VkCommandBuffer>(cmd));
  });

  ck::log::info("ImGuiLayer attached (dynamic rendering, format={})",
                static_cast<int>(color_format));
}

void ImGuiLayer::OnDetach() {
  Renderer& renderer = Application::Get().GetRenderer();
  renderer.SetImGuiRenderCallback({});
  renderer.context().device().waitIdle();

  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void ImGuiLayer::OnEvent(Event& e) {
  if (!block_events_) return;
  ImGuiIO& io = ImGui::GetIO();
  if (io.WantCaptureMouse && (e.GetCategoryFlags() & EventCategoryMouse)) {
    e.GetIsHandled() = true;
  }
  if (io.WantCaptureKeyboard && (e.GetCategoryFlags() & EventCategoryKeyboard)) {
    e.GetIsHandled() = true;
  }
}

void ImGuiLayer::Begin() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void ImGuiLayer::End() {
  ImGui::Render();
}

}  // namespace ck
