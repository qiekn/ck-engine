# User's Notes

## Code style

Basically using Google C++ code style, but line width can be 120 if necessary.

Git commit message must be simple.

Section comment uses this format:

```cpp
// - -----------------------------------------------------------------------------: context
```

# CLAUDE's Notes

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

ck-engine is a learning-oriented game engine, mirroring TheCherno's Hazel engine series. Branch `3d` is currently mid-Phase-6 (ImGui integration; 6.A.1 + 6.A.2 done):

- Build infrastructure: CMake 3.30, C++23, libc++, `import std`, presets — done (Phase 0/1)
- OpenGL backend dropped (Phase 2a, commit `0c76c3b`); engine reduced to a bare GLFW window with `GLFW_NO_API`
- Vulkan stack wired (Phase 2b, commit `91959cb`): volk (built from SDK source via `cmake/Vulkan.cmake`) + Vulkan-Hpp header + VMA + Slang from the SDK; imgui on ocornut docking upstream
- Vulkan bring-up done (Phase 3): `Renderer` drives BeginFrame/EndFrame on a Vulkan 1.3 dynamic-rendering loop with a time-cycled clear color; resize / out-of-date / suboptimal handled
- Hello-triangle done (Phase 4): runtime Slang→SPIR-V compile + dynamic-rendering pipeline.
- Renderer abstractions done (Phase 5): VMA-backed `Allocator` / `Buffer` / `Image` / `Sampler` / `DescriptorPool` / `DescriptorSetLayout` / `UniformBuffer<T>` / `Material` / engine-wide `vk::PipelineCache`; Hazel-style `Renderer2D` quad batcher with bindless `Sampler2D textures[]` (kMaxQuads=10000, kMaxTextures=32); public API switched from `#include <ck.h>` to `import ck;` (module at `src/engine/ck.cppm`).
- ImGui integration in progress (Phase 6.A): 6.A.1 added an offscreen `color_target_` + swapchain copy; 6.A.2 stood up `ck::ImGuiLayer` (imgui_impl_glfw + imgui_impl_vulkan via volk), added an optional imgui pass on the swapchain inside `Renderer::EndFrame`, and re-exported a minimal `ImGui::*` set through `ck.cppm`. 6.A.3 dropped the swapchain copy entirely: `color_target_` is now `Sampled` + transitions to `ShaderReadOnlyOptimal`, the editor opens a `DockSpaceOverViewport` plus a `Viewport` panel that hosts `color_target_` via `ImGui_ImplVulkan_AddTexture` → `ImTextureID`, and `Renderer` exposes a color-target lifecycle callback so `ImGuiLayer` rebinds the descriptor on every recreate.
- Scene/ECS started (Phase 6.B.1): `ck::Scene` wraps `entt::registry`; `ck::Entity` is a thin `(entt::entity, Scene*)` handle so clients add components via `entity.AddComponent<T>()` without seeing entt. Components live in `scene/components.h` (`TagComponent`, `TransformComponent`, `SpriteRendererComponent`). `Scene::OnUpdate` iterates the `(Transform, SpriteRenderer)` view and dispatches each to `Renderer2D::DrawQuad`. `ck.cppm` re-exports `Scene` / `Entity` / the three components.

Editor drives a single textured quad through `Scene` + `(Transform, SpriteRenderer)`; sandbox draws a 10×10 grid of textured quads (3 textures, 1 draw call) directly through `ck::Renderer2D::DrawQuad` from its layer.

## Current Learning Context

- Hazel 2024.1 Vulkan study source: `../Hazel`
- Personal Vulkan practice project: `../vulkan`
- Slang/Vulkan tutorial reference: `../how-to-vulkan`
- Current focus in this repository: rebuilding the renderer against Vulkan-Hpp + dynamic rendering

## Build & Run

CMake 3.30+, Clang 18+ with libc++, Ninja. Generator/toolchain are pinned via `CMakePresets.json`.

```powershell
cmake --preset debug              # configure
cmake --build --preset debug      # build
.\build\debug\editor.exe          # run
```

Or via the wrappers:

- MSYS2 UCRT64 / bash: `./run.sh` (use `./run.sh debug` to launch under gdb)
- PowerShell: `.\run.ps1`

The wrapper auto-runs the configure step on first invocation, then builds and launches `editor`.

Notes:

- Submodules required. Clone with `--recursive`, or `git submodule update --init --recursive` after the fact.
- First configure downloads `cmExperimental.cxx` from Kitware's GitHub to extract the `import std` GUID (~50 KB, cached afterwards).
- `import std` is experimental; CMake will warn at configure time. This is expected, not an error.
- `CMAKE_EXPORT_COMPILE_COMMANDS=ON`: clangd reads `build/debug/compile_commands.json`.
- No test target.

## CMake Layout

Top-level `CMakeLists.txt` is small (toolchain + `project()` + 4 `add_subdirectory`):

| Subdir              | Purpose                                                              |
| ------------------- | -------------------------------------------------------------------- |
| `cmake/`            | Helper modules (`EnableCxxImportStd.cmake`)                          |
| `deps/`             | Option overrides + `add_subdirectory` per dep + INTERFACE header libs |
| `src/engine/`       | `ck` STATIC lib. PCH at `pch.h` (PRIVATE), public module interface `ck.cppm`      |
| `src/editor/`       | `editor` exe (`ck_editor::Editor` Application + `ck_editor::EditorLayer` + `panels/` — `ck_editor::StatsPanel`, `ck_editor::ViewportPanel`, `ck_editor::SceneHierarchyPanel`) |
| `src/sandbox/`      | `sandbox` exe (standalone client example)                            |

All targets globbed with `CONFIGURE_DEPENDS`. `CMAKE_RUNTIME_OUTPUT_DIRECTORY` keeps `editor.exe`/`sandbox.exe` at `build/<preset>/` root.

Both executables write a one-line `int main(...) { return ck::EntryPoint(argc, argv); }`. `ck::EntryPoint` (in `src/engine/core/entry_point.cpp`) calls `Log::Init` then drives `ck::CreateApplication`, which the client provides (see `src/editor/editor.cpp` / `src/sandbox/sandbox.cpp`).

## Architecture (post-Phase-5, Renderer2D + module API)

The early-Hazel layered skeleton plus a thin Vulkan-only renderer:

- **Renderer** (`renderer/renderer.{h,cpp}`) — frontend; owns `vulkan::Context` / `vulkan::Allocator` / `vulkan::Swapchain` / offscreen `vulkan::Image color_target_` (sampled, not blitted) / `std::array<Frame, kFramesInFlight>` / `vulkan::SlangCompiler` / `Camera`. `BeginFrame` records cmdbuf begin + transition `color_target_` to `ColorAttachmentOptimal` + `beginRendering` (cleared time-cycled rgb) + `Renderer2D::BeginScene`; layers fill the batch via `Renderer2D::DrawQuad` in `OnUpdate`; `EndFrame` calls `Renderer2D::EndScene(cmd)`, `endRendering`, transitions `color_target_` to `ShaderReadOnlyOptimal`, then opens a fresh swapchain pass (`loadOp=Clear` black) where the registered `imgui_render_` callback draws the dockspace + ViewportPanel (which samples `color_target_`) + StatsPanel; finally swapchain → `PresentSrcKHR` + submit2 + present. `OnResize` defers swapchain + color_target recreate to next BeginFrame; `SetColorTargetCallback` lets `ImGuiLayer` re-register the descriptor whenever `color_target_` is (re)created.
- **`ImGuiLayer`** (`imgui/imgui_layer.{h,cpp}`) — engine-side overlay that owns the ImGui context + `imgui_impl_glfw` + `imgui_impl_vulkan` (dynamic rendering, `DescriptorPoolSize=128` so imgui owns the pool). DPI-scaled OpenSans font + Hazel dark palette (gamma-corrected for the sRGB swapchain). `Begin/End` brackets the per-frame `OnImGuiRender` pass; the actual cmdbuf record happens via the callback registered with `Renderer::SetImGuiRenderCallback`. Holds an `ImTextureID viewport_tex_` (descriptor for `color_target_`) refreshed via `Renderer::SetColorTargetCallback`; clients reach it through `viewport_texture_id()`. ImGui-C/Vulkan-C headers live only in the `.cpp`.
- **`Renderer2D`** (`renderer/renderer_2d.{h,cpp}`) — static-API quad batcher with bindless texture array (binding 0 UBO, binding 1 `Sampler2D[kMaxTextures]` with `PartiallyBound` + `VariableDescriptorCount`). `Init/Shutdown` lifetime owned by Renderer; `LoadTexture(path) -> TextureHandle` registers Image::FromFile into the bindless slot; per-frame host-visible vertex buffer + static index buffer; flushes one `drawIndexed` per `EndScene`.
- **`Scene`** / **`Entity`** / **components** (`scene/scene.{h,cpp}`, `scene/entity.h`, `scene/components.h`) — `Scene` owns an `entt::registry`; `Entity` is a thin `(entt::entity, Scene*)` handle so clients add components via `entity.AddComponent<T>()` without ever seeing entt. `Scene::OnUpdate` iterates the `(TransformComponent, SpriteRendererComponent)` view and dispatches each to `Renderer2D::DrawQuad`. Components: `TagComponent` (display name), `TransformComponent` (mat4 — will split into T/R/S when 6.B.3 wires PropertiesPanel), `SpriteRendererComponent` (TextureHandle + tint).
- **`Material`** (`renderer/material.{h,cpp}`) — Spec-driven shader+layout+pipeline+per-frame descriptor-set bundle. Built against the engine-wide `vk::PipelineCache`. Public abstraction; not used by Renderer's own quad path (Renderer2D drives that directly).
- **`Camera`** (`renderer/camera.{h,cpp}`) — orthographic camera with `SetViewport` / `SetPosition` / `view_projection`; Vulkan Y-down NDC handled internally.
- **`vulkan::Allocator`** (`renderer/vulkan/allocator.{h,cpp}`) — VMA wrapper + transient command pool/fence for `ImmediateSubmit` (one-shot uploads).
- **`vulkan::Buffer`** (`renderer/vulkan/buffer.{h,cpp}`) — RAII over (VkBuffer, VmaAllocation); `HostVisible` / `DeviceLocal` types; `CreateDeviceLocal` does staging upload.
- **`vulkan::Image`** (`renderer/vulkan/image.{h,cpp}`) — RAII over (VkImage, VkImageView, VmaAllocation); `FromFile` slurps via stb_image and lands in `ShaderReadOnlyOptimal`.
- **`vulkan::Sampler`** (`renderer/vulkan/sampler.{h,cpp}`) — RAII over `vk::Sampler`; default linear + repeat.
- **`vulkan::DescriptorPool`** / **`DescriptorSetLayout`** / **`DescriptorWriter`** (`renderer/vulkan/descriptor.{h,cpp}`) — RAII pool + layout, plus a chained writer with stable buffer/image-info storage.
- **`vulkan::UniformBuffer<T>`** (`renderer/vulkan/uniform_buffer.h`) — per-frame UBO ring; one HostVisible Buffer per frame in flight, persistently mapped.
- **`vulkan::Context`** (`renderer/vulkan/context.{h,cpp}`) — volk init, Vulkan-Hpp dynamic dispatcher, validation layer + debug messenger, GLFW surface, physical device pick, logical device with vk1.3 dynamic rendering + sync2 + vk1.1 shaderDrawParameters (Slang's SV_VertexID translation declares the DrawParameters capability).
- **`vulkan::Swapchain`** (`renderer/vulkan/swapchain.{h,cpp}`) — surface format / present mode (vsync via `Window::IsVSync`), HiDPI extent, per-image views; `Recreate()`.
- **`vulkan::Frame`** (`renderer/vulkan/frame.{h,cpp}`) — per-frame-in-flight: command pool/buffer + `image_available` semaphore + `in_flight` fence (signalled init). `render_finished` is per-swapchain-image and lives in `Renderer` (binary semaphore reuse rule).
- **`vulkan::SlangCompiler`** (`renderer/shader/slang_compiler.{h,cpp}`) — runtime Slang→SPIR-V compiler. Owns `slang::IGlobalSession` + a `slang::ISession` configured for SPIR-V 1.4 / EmitSpirvDirectly / column-major matrices. `CompileToSpirv(path)` slurps and compiles a `.slang` file with vert+frag entry points.
- **`vulkan::ShaderModule`** (`renderer/shader/shader_module.{h,cpp}`) — RAII over `vk::ShaderModule`; one module hosts both vert and frag entry points, pipeline picks via `(stage, "main")`.
- **`vulkan::GraphicsPipeline`** (`renderer/shader/graphics_pipeline.{h,cpp}`) — dynamic-rendering pipeline (no vertex input, no descriptors, dynamic viewport+scissor) wired to the swapchain color format via `vk::PipelineRenderingCreateInfo`.

Everything else survived the Phase 2a pruning — the early-Hazel layered skeleton, no scene/ECS or content systems yet:

- **Application** (`core/application.{h,cpp}`) — singleton (`Application::Get()`). Owns a `Window`, a `Renderer`, and a `LayerStack`; auto-pushes one `ImGuiLayer` overlay in its constructor and stores a raw observer pointer. Run loop ticks layers' `OnUpdate`, drives `imgui_layer_->Begin / iterate-OnImGuiRender / imgui_layer_->End`, then `renderer_->EndFrame()` and `window_->OnUpdate()`. `GetRenderer()` / `GetImGuiLayer()` exposed for clients and engine sub-systems.
- **Layer / LayerStack** (`core/layer.h`, `core/layer_stack.{h,cpp}`) — `OnAttach/OnDetach/OnUpdate(DeltaTime)/OnEvent/OnImGuiRender`. Overlays after regular layers; reverse iteration for events (stop on `IsHandled`).
- **Events** (`events/`) — `EventDispatcher::DispatchEvent<T>(fn)` + `CK_BIND_EVENT(method)` macro.
- **Window / Input** (`core/window.h`, `platform/windows/`) — GLFW-backed (`GLFW_NO_API`, no GL context). The `windows_*` directory name is GLFW path used on every OS, not Win32-specific.
- **Logging / profiling / math / utils** (`core/log.h`, `debug/profiler.h`, `math/`, `utils/`) — unchanged.

Deleted in Phase 2a (will be rebuilt against Vulkan): renderer abstractions (`Renderer`/`Renderer2D`/`RendererAPI`/`Shader`/`Buffer`/`VertexArray`/`FrameBuffer`/`UniformBuffer`/`Texture`), OpenGL impl (`platform/opengl/`), Scene + ECS + components (`scene/`, `components.h`), yaml-cpp scene serializer, ImGui integration, editor panels.

## Vulkan Migration Plan

Roadmap (one phase = one commit):

- **Phase 0** ✅ CMake 3.30 + C++23 + libc++ + `import std` + presets — `bf65295`
- **Phase 1** ✅ Split CMakeLists into per-target subdirectories — `e12282a`
- **Phase 2a** ✅ Drop OpenGL backend; engine reduced to bare window — `0c76c3b`
- **Phase 2b** ✅ Add Vulkan stack (volk + `Vulkan::cppm` + VMA + Slang); switch imgui to upstream docking — `91959cb`
- **Phase 3** ✅ Vulkan bring-up: instance/device/swapchain → time-based clear-color (dynamic rendering, sync2, 2 frames in flight, resize handling) — `b29641c`..`14ea163`
- **Phase 4** ✅ Slang runtime compile + first graphics pipeline + hello-triangle (RGB vertex-interp triangle on the time-cycled clear) — `36d816e`..`b52bc90`
- **Phase 5** ✅ Renderer abstractions + Renderer2D + module API — split into 5.1 (Allocator/Buffer/VBO+IBO triangle), 5.2 (Image/Sampler/DescriptorPool/UBO ring/textured quad), 5.3 (Material + engine-wide PipelineCache), 5.4 (Camera + Renderer2D bindless quad batcher), 5.5 (unified `ck::log` API + `import ck` module + umbrella header retired)
- **Phase 6.A** ✅ ImGui integration — 6.A.1 ✅ offscreen color target + swapchain blit (`e578740`); 6.A.2 ✅ `ck::ImGuiLayer` + optional imgui pass on swapchain + `ImGui::*` re-export; 6.A.3 ✅ drop swapchain copy + dockspace + ViewportPanel hosts color_target via `ImGui_ImplVulkan_AddTexture`; 6.A.3.5 ✅ panel-driven offscreen + camera resize via `Application::OnViewportResize`; 6.A.4 ✅ extract Stats/Viewport into `ck_editor::` panels under `src/editor/panels/`; 6.A.5 ✅ EditorLayer + Editor moved into `ck_editor::` namespace
- **Phase 6.B** in progress — scene/ECS + editor panels — 6.B.1 ✅ Scene/Entity/components scaffolding (entt-backed) + editor switched to Scene-driven path; 6.B.2 ✅ SceneHierarchyPanel (ck_editor::, lists entities, click-select, right-click create/delete) + Scene::GetAllEntities; 6.B.3 — PropertiesPanel (split TransformComponent into T/R/S); 6.B.4 — editor camera; 6.B.5 — yaml scene serializer (TBD)
- **Phase 6.C+** — 3D mesh renderer (TBD)

## Conventions

`.clang-format` (Google-based, 100-col, 2-space indent, pointer-left) and `.clang-tidy` enforce style. Identifier naming:

- namespaces `lower_case`; classes/structs/templates `CamelCase`
- functions `aNy_CasE` tolerated
- locals/params/vars `lower_case`; private/protected members trailing `_`
- enum constants, constexprs, global/member/static constants: `CamelCase` with `k` prefix (e.g. `kVulkan`)

Smart-pointer aliases (`core/core.h`):

- `ck::Scope<T>` ≡ `std::unique_ptr<T>`, factory `ck::CreateScope<T>(args...)`
- `ck::Ref<T>` ≡ `std::shared_ptr<T>`, factory `ck::CreateRef<T>(args...)`

Logging / asserts (`core/log.h`):

- `ck::log::trace/debug/info/warn/error/fatal(fmt, args...)` — variadic templates forwarding to a single spdlog logger; engine + client share it.
- `CK_ASSERT(cond, msg)` — gated by `CK_ENABLE_ASSERTS`.

Profiling (`debug/profiler.h`):

- `CK_PROFILE_FUNCTION()` / `CK_PROFILE_SCOPE("Label")`. Sessions framed by `CK_PROFILE_BEGIN_SESSION` / `CK_PROFILE_END_SESSION` (already wired in `entry_point.cpp`).
- Output: chrome://tracing JSON. View at <https://ui.perfetto.dev>. Files gitignored.

Public engine API: clients `import ck;` (module interface at `src/engine/ck.cppm`). Clients write `int main() { return ck::EntryPoint(argc, argv); }` and never #include engine headers directly; the umbrella `include/ck.h` is gone. Engine internals still use `#include`.

## Dependencies

Vendored as git submodules under `deps/`, wired in `deps/CMakeLists.txt`:

`spdlog`, `glfw`, `glm`, `imgui` (ocornut upstream, docking branch — `imgui` target inlined directly in `deps/CMakeLists.txt` because upstream ships no CMakeLists), `imguizmo`, `yaml-cpp`, `box2d`, in-tree `stb_image`. EnTT header-only at `deps/entt/include`. spdlog + EnTT exposed as INTERFACE targets `spdlog_headers` / `entt_headers`.

Vulkan stack lives outside `deps/` — pulled from the Vulkan SDK by `cmake/Vulkan.cmake`:

- `Vulkan::Vulkan` + `Vulkan::volk` (loader)
- `Vulkan::cppm` — alias of `VulkanCppModule`, the C++23 module built from `vulkan.cppm` / `vulkan_video.cppm`
- `Vulkan::vma` — INTERFACE wrapper around `vk_mem_alloc.h` shipped with the SDK
- `Slang::slang` — runtime shader compiler

imgui's `imgui_impl_vulkan` is built with `IMGUI_IMPL_VULKAN_USE_VOLK` so engine and imgui share one Vulkan loader.

## Documentation Strategy

Working context here is constrained — keep messages tight (~300k tokens). Externalize anything that risks growing the conversation:

- **`./docs/src/*.md`** — long-form notes (design rationale, API tours, Vulkan walkthroughs, learning notes). Independent mdBook on the `docs` orphan branch (separate `docs/.git`); commit there independently, not in engine commits.
- **`CLAUDE.md`** — durable project rules and conventions. Edit when something changes about *how* we work (style, layout, build, phase completion). Not for transient state.
- **Memory** (`~/.claude/projects/<project>/memory/`) — user/feedback/project/reference per the auto-memory system.

Default: if explaining something at length, write to `docs/src/` instead and link. Whenever phase status, dependency list, or architecture changes, update CLAUDE.md in the same commit.

## Commits

Auto-commit governed by `~/.claude/rules/git-workflow.md` (presence of `.claude/.no-autocommit` disables it). Use the `git-commit-message` skill for messages. Repository convention: prefixes `ep` / `feat` / `add` / `update` / `fix` / `docs` / `refactor` / `chore`.
