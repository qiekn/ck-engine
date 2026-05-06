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

- Build infrastructure (CMake 3.30, C++23, libc++, `import std`, presets) is already in
- OpenGL backend in `src/engine/platform/opengl/` is still present and still builds
- Vulkan backend, VMA, volk, slang shader pipeline are pending (Phase 2+)

EnTT-backed scene/component system, ImGui editor (`editor`), example client (`sandbox`).

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

## Architecture (current, OpenGL-era — being replaced)

The engine follows the early Hazel layered design:

- **Application** (`core/application.{h,cpp}`) — singleton (`Application::Get()`). Owns the `Window`, an `ImGuiLayer` overlay, and a `LayerStack`. Run loop ticks layers, runs `OnImGuiRender` between `imgui_layer_->Begin()`/`End()`, then `window_->OnUpdate()`.
- **Layer / LayerStack** (`core/layer.h`, `core/layer_stack.{h,cpp}`) — `OnAttach/OnDetach/OnUpdate(DeltaTime)/OnEvent/OnImGuiRender`. Overlays after regular layers; reverse iteration for events (stop on `IsHandled`).
- **Events** (`events/`) — `EventDispatcher::DispatchEvent<T>(fn)` + `CK_BIND_EVENT(method)` macro.
- **Window / Input** (`core/window.h`, `platform/windows/`) — GLFW-backed. The `windows_*` directory name is misleading: it is the GLFW path used on every platform.
- **Renderer** (`renderer/`) — API-agnostic interfaces (`Renderer`, `Renderer2D`, `RendererAPI`, `Shader`, `Buffer`, `VertexArray`, `FrameBuffer`, `UniformBuffer`, `Texture`) + OpenGL impl in `platform/opengl/`. **The `RendererAPI::Type` enum and the `kOpenGL` impl will be deleted in Phase 2.**
- **Scene / ECS** (`scene/`) — `Scene` wraps `entt::registry`. Components in `components.h`. Runtime/simulation/editor modes. Owns Box2D `b2WorldId`.
- **Serialization** — `scene/scene_serializer.{h,cpp}` (yaml-cpp). Scenes at `assets/scenes/*.scene`.
- **ImGui integration** — `imgui/imgui_layer.{h,cpp}` is pushed as the first overlay. `imgui_build.cpp` is the unity-style backend cpp (will be replaced by `imgui_impl_vulkan` + `imgui_impl_glfw` in Phase 2).

## Vulkan Migration Plan

Roadmap (one phase = one commit):

- **Phase 0** ✅ CMake 3.30 + C++23 + libc++ + `import std` + presets
- **Phase 1** ✅ Split CMakeLists into per-target subdirectories
- **Phase 2** — Drop OpenGL, add Vulkan SDK / VMA / volk / official imgui docking / `Vulkan::cppm`
- **Phase 3** — Vulkan instance / device / swapchain + clear-color milestone
- **Phase 4** — Slang runtime compile + first graphics pipeline + hello-triangle
- **Phase 5+** — Renderer / Material / RenderPass abstractions, Renderer2D rebuild, modules-based public API

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

Vendored as git submodules under `deps/`, added via `add_subdirectory` (centralized in `deps/CMakeLists.txt`):

`spdlog`, `glfw`, `glm`, `imgui` (qiekn fork — to be replaced by official docking branch), `imguizmo`, `yaml-cpp`, `box2d`, plus in-tree `glad` (Phase 2 deletion) and `stb_image`. EnTT header-only at `deps/entt/include`. Spdlog and EnTT exposed as INTERFACE targets `spdlog_headers` / `entt_headers`.

Pending additions in Phase 2: `deps/vma`, `deps/volk`, official `imgui` (docking branch), Slang runtime via `find_package(Vulkan COMPONENTS SLANG)`.

## Commits

Auto-commit governed by `~/.claude/rules/git-workflow.md` (presence of `.claude/.no-autocommit` disables it). Use the `git-commit-message` skill for messages. Repository convention: prefixes `ep` / `feat` / `add` / `update` / `fix` / `docs` / `refactor` / `chore`.