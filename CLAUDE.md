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

ck-engine is a learning-oriented game engine, mirroring TheCherno's Hazel engine series.
The codebase is **mid-migration from OpenGL to Vulkan** (branch `3d`):

- Build infrastructure: CMake 3.30, C++23, libc++, `import std`, presets — done (Phase 0/1)
- OpenGL backend dropped (Phase 2a, commit `0c76c3b`); engine reduced to a bare GLFW window with `GLFW_NO_API`
- Vulkan stack wired (Phase 2b, commit `91959cb`): volk (built from SDK source via `cmake/Vulkan.cmake`) + Vulkan-Hpp header + VMA + Slang from the SDK; imgui on ocornut docking upstream
- Vulkan bring-up done (Phase 3): `Renderer` drives BeginFrame/EndFrame on a Vulkan 1.3 dynamic-rendering loop with a time-cycled clear color; resize / out-of-date / suboptimal handled

Editor (`editor`) and sandbox (`sandbox`) open a window with the Vulkan clear-color loop running — no scene/ECS or 2D content yet (those will be rebuilt in Phase 5+).

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
| `src/engine/`       | `ck` STATIC lib. PCH at `pch.h`, public umbrella header `include/ck.h` |
| `src/editor/`       | `editor` exe (`EditorLayer` + `panels/`)                             |
| `src/sandbox/`      | `sandbox` exe (standalone client example)                            |

All targets globbed with `CONFIGURE_DEPENDS`. `CMAKE_RUNTIME_OUTPUT_DIRECTORY` keeps `editor.exe`/`sandbox.exe` at `build/<preset>/` root.

Both executables include `core/entry_point.h` exactly once. That header defines `main()`, calls `ck::Log::Init()`, then expects the client to provide `ck::CreateApplication(args)` (see `src/editor/editor.cpp` and `src/sandbox/sandbox.cpp`).

## Architecture (post-Phase-3, clear-color renderer)

The early-Hazel layered skeleton plus a thin Vulkan-only renderer:

- **Renderer** (`renderer/renderer.{h,cpp}`) — frontend; owns `vulkan::Context`, `vulkan::Swapchain`, and `std::array<Frame, kFramesInFlight>`. `BeginFrame` / `EndFrame` drive the dynamic-rendering frame loop; `OnResize` defers swapchain recreate to next BeginFrame.
- **`vulkan::Context`** (`renderer/vulkan/context.{h,cpp}`) — volk init, Vulkan-Hpp dynamic dispatcher, validation layer + debug messenger, GLFW surface, physical device pick, logical device with vk1.3 dynamic rendering + sync2.
- **`vulkan::Swapchain`** (`renderer/vulkan/swapchain.{h,cpp}`) — surface format / present mode (vsync via `Window::IsVSync`), HiDPI extent, per-image views; `Recreate()`.
- **`vulkan::Frame`** (`renderer/vulkan/frame.{h,cpp}`) — per-frame-in-flight: command pool/buffer + `image_available` semaphore + `in_flight` fence (signalled init). `render_finished` is per-swapchain-image and lives in `Renderer` (binary semaphore reuse rule).

Everything else survived the Phase 2a pruning — the early-Hazel layered skeleton, no scene/ECS or content systems yet:

- **Application** (`core/application.{h,cpp}`) — singleton (`Application::Get()`). Owns a `Window` and a `LayerStack`. Run loop ticks layers then `window_->OnUpdate()` (just `glfwPollEvents()` now — present is the future Vulkan renderer's job).
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
- **Phase 4** ⏳ Slang runtime compile + first graphics pipeline + hello-triangle
- **Phase 5+** — Renderer / Material / RenderPass abstractions, Renderer2D rebuild, modules-based public API (`import ck` replaces the umbrella header)

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

- `CK_ENGINE_TRACE/INFO/WARN/ERROR/FATAL(...)` and `CK_CLIENT_*` mirrors. spdlog format strings.
- `CK_ENGINE_ASSERT(cond, msg)` / `CK_CLIENT_ASSERT(cond, msg)` — gated by `CK_ENABLE_ASSERTS`.

Profiling (`debug/profiler.h`):

- `CK_PROFILE_FUNCTION()` / `CK_PROFILE_SCOPE("Label")`. Sessions framed by `CK_PROFILE_BEGIN_SESSION` / `CK_PROFILE_END_SESSION` (already wired in `entry_point.h`).
- Output: chrome://tracing JSON. View at <https://ui.perfetto.dev>. Files gitignored.

Public engine API: clients `#include <ck.h>` (umbrella in `src/engine/include/ck.h`). After Phase 4, this becomes `import ck;` and the umbrella is replaced by a `.cppm` module interface; engine internals still use `#include`.

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

Working context here is constrained — keep messages tight (~30k tokens). Externalize anything that risks growing the conversation:

- **`./docs/src/*.md`** — long-form notes (design rationale, API tours, Vulkan walkthroughs, learning notes). Independent mdBook on the `docs` orphan branch (separate `docs/.git`); commit there independently, not in engine commits.
- **`CLAUDE.md`** — durable project rules and conventions. Edit when something changes about *how* we work (style, layout, build, phase completion). Not for transient state.
- **Memory** (`~/.claude/projects/<project>/memory/`) — user/feedback/project/reference per the auto-memory system.

Default: if explaining something at length, write to `docs/src/` instead and link. Whenever phase status, dependency list, or architecture changes, update CLAUDE.md in the same commit.

## Commits

Auto-commit governed by `~/.claude/rules/git-workflow.md` (presence of `.claude/.no-autocommit` disables it). Use the `git-commit-message` skill for messages. Repository convention: prefixes `ep` / `feat` / `add` / `update` / `fix` / `docs` / `refactor` / `chore`.