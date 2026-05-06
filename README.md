# ck-engine

A learning-oriented game engine following the
[game engine](https://www.youtube.com/playlist?list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT)
series by [TheCherno](https://www.youtube.com/user/TheChernoProject).

> Currently migrating from OpenGL to Vulkan. Branch [`3d`](../../tree/3d).

<a href="https://qiekn.github.io/ck-engine/" target="_blank">WIP: docs</a>

## Compile

This project uses:

- CMake 3.30+
- Ninja
- Clang / Clang++ (with libc++)
- [Vulkan SDK](https://vulkan.lunarg.com/) (for `slangc`, validation layers)

### Windows PowerShell

Install dependencies with [Scoop](https://scoop.sh/):

```powershell
# Install Scoop package manager
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
Invoke-RestMethod -Uri https://get.scoop.sh | Invoke-Expression

scoop install llvm ninja cmake
```

Install the [Vulkan SDK](https://vulkan.lunarg.com/) separately.

Clone with submodules:

```powershell
git clone --recursive https://github.com/qiekn/ck-engine.git
cd ck-engine
```

Then run:

```powershell
.\run.ps1
```

Or manually:

```powershell
cmake --preset debug
cmake --build --preset debug
.\build\debug\editor.exe
```

### Linux / macOS / WSL / MSYS2

Install Clang (with libc++), Ninja, CMake 3.30+, and the Vulkan SDK from your package manager. Then:

```bash
./run.sh           # build and launch editor
./run.sh debug     # launch under gdb
```

## Commit Convention

| mark     | description                         |
| :------- | :---------------------------------- |
| ep       | episode                             |
| feat     | new feature                         |
| add      | add third party library             |
| update   | make some progress                  |
| fix      | fix a bug                           |
| docs     | documents (readme or comments)      |
| refactor | refactor code                       |
| chore    | small changes                       |