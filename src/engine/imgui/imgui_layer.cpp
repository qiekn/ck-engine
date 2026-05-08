#include "imgui_layer.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <volk.h>
#include <GLFW/glfw3.h>

#include <cmath>

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

// Hazel-style dark theme — carried over from the OpenGL-era ImGuiLayer so the
// editor keeps the look the user is used to.
void ApplyDarkThemeColors() {
  auto& colors = ImGui::GetStyle().Colors;
  colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.105f, 0.11f, 1.0f};

  colors[ImGuiCol_Header]        = ImVec4{0.2f,  0.205f,  0.21f,  1.0f};
  colors[ImGuiCol_HeaderHovered] = ImVec4{0.3f,  0.305f,  0.31f,  1.0f};
  colors[ImGuiCol_HeaderActive]  = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

  colors[ImGuiCol_Button]        = ImVec4{0.2f,  0.205f,  0.21f,  1.0f};
  colors[ImGuiCol_ButtonHovered] = ImVec4{0.3f,  0.305f,  0.31f,  1.0f};
  colors[ImGuiCol_ButtonActive]  = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

  colors[ImGuiCol_FrameBg]        = ImVec4{0.2f,  0.205f,  0.21f,  1.0f};
  colors[ImGuiCol_FrameBgHovered] = ImVec4{0.3f,  0.305f,  0.31f,  1.0f};
  colors[ImGuiCol_FrameBgActive]  = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

  colors[ImGuiCol_Tab]                = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TabHovered]         = ImVec4{0.38f, 0.3805f, 0.381f, 1.0f};
  colors[ImGuiCol_TabActive]          = ImVec4{0.28f, 0.2805f, 0.281f, 1.0f};
  colors[ImGuiCol_TabUnfocused]       = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.2f,  0.205f,  0.21f,  1.0f};

  colors[ImGuiCol_TitleBg]          = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TitleBgActive]    = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
}

// The swapchain image is sRGB so the GPU does linear→sRGB encoding on every
// fragment write. ImGui colors above are perceptual sRGB values though
// (matched the OpenGL pipeline that wrote raw byte values to the framebuffer).
// Without compensation each component lands ~pow(c, 1/2.2) brighter than
// authored, so the dark Hazel palette renders washed out. Pre-applying
// pow(c, 2.2) cancels the hardware encoding and the displayed pixels match
// the authored values.
void GammaCorrectStyleColors() {
  auto& colors = ImGui::GetStyle().Colors;
  for (int i = 0; i < ImGuiCol_COUNT; ++i) {
    colors[i].x = std::pow(colors[i].x, 2.2f);
    colors[i].y = std::pow(colors[i].y, 2.2f);
    colors[i].z = std::pow(colors[i].z, 2.2f);
  }
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

  // Fonts + style sized for the monitor's DPI scale; with GLFW_SCALE_TO_MONITOR
  // the window is created at the scaled size, so io.DisplayFramebufferScale
  // stays at 1.0 and we have to inflate font + widget metrics ourselves.
  const float dpi = Window::s_high_dpi_scale_factor_;
  io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Bold.ttf", dpi * 18.0f);
  io.FontDefault = io.Fonts->AddFontFromFileTTF(
      "assets/fonts/opensans/OpenSans-Regular.ttf", dpi * 18.0f);

  ImGui::StyleColorsDark();
  ImGuiStyle& style = ImGui::GetStyle();
  style.WindowRounding = 8.0f;
  style.FrameRounding = 8.0f;
  style.ScaleAllSizes(dpi);
  ApplyDarkThemeColors();
  GammaCorrectStyleColors();

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

  ck::log::info("ImGuiLayer attached (dynamic rendering, format={}, dpi={})",
                static_cast<int>(color_format), dpi);
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
