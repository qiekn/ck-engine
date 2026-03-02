# ck-engine (Yet Another Hazel Engine)

A learning-oriented 2D game engine, following the
[game engine](https://www.youtube.com/playlist?list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT)
series by [TheCherno](https://www.youtube.com/user/TheChernoProject).

<a href="https://qiekn.github.io/ck-engine/" target="_blank">WIP: docs</a>

## Features

- OpenGL 4.6 renderer with batch rendering
- ECS scene system (EnTT)
- ImGui-based editor with scene hierarchy, properties panel, content browser
- Scene serialization (YAML)
- C# scripting via embedded Mono runtime

## Quickstart

### Prerequisites

- C++20 compiler (Clang, GCC, or MSVC)
- CMake 3.24+
- Git

All other dependencies (including the Mono runtime for C# scripting) are vendored in `deps/` and require no separate installation.

### Build & Run

```bash
git clone --recursive https://github.com/qiekn/ck-engine.git
cd ck-engine
```

<details>
<summary><b>MSYS2 UCRT64</b> (recommended for Windows)</summary>

Install [MSYS2](https://www.msys2.org/), then open **UCRT64** terminal:

```bash
# Install toolchain (first time only)
pacman -S --needed \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-clang \
  mingw-w64-ucrt-x86_64-make \
  mingw-w64-ucrt-x86_64-gdb \
  git

# Build and run
./run.sh
```

Or configure manually:

```bash
cmake -B build -G "MinGW Makefiles" \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++
cmake --build build
./build/editor.exe
```

</details>

<details>
<summary><b>Visual Studio</b> (MSVC)</summary>

1. Open the project folder in Visual Studio (File > Open > CMake...)
2. Visual Studio will auto-detect `CMakeLists.txt` and configure the project
3. Select `editor.exe` as the startup target and run

Or from **Developer Command Prompt**:

```cmd
cmake -B build
cmake --build build --config Debug
build\Debug\editor.exe
```

</details>

<details>
<summary><b>Linux</b></summary>

> not tested

```bash
# Install Mono (Ubuntu/Debian)
sudo apt install libmono-2.0-dev

cmake -B build
cmake --build build
./build/editor
```

</details>

<details>
<summary><b>macOS</b></summary>

> Apple deprecated OpenGL and caps support at **OpenGL 4.1**. The engine currently targets **OpenGL 4.6**. To run on macOS, you will need to downgrade the GLFW window hints to OpenGL 4.1 and replace any 4.2+ API calls in `src/engine/platform/opengl/` with their legacy equivalents.

Mono is also not bundled for macOS — install it separately:

```bash
brew install mono
cmake -B build
cmake --build build
./build/editor
```

</details>

### Build Targets

| Target   | Type       | Description          |
| :------- | :--------- | :------------------- |
| `ck`     | Static lib | Engine core          |
| `editor` | Executable | Scene editor (main)  |
| `sandbox`| Executable | Test/demo app        |

## Commit Convention

| mark     | description                         |
| :------- | :---------------------------------- |
| ep       | episode                             |
| feat     | new feature                         |
| add      | add third party library             |
| update   | make some progress                  |
| fix      | fix a bug                           |
| docs     | documents (readme or some comments) |
| refactor | refactor code                       |
| chores   | some little changes                 |
