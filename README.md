# ck-engine

A learning-oriented 2D game engine, following the
[game engine](https://www.youtube.com/playlist?list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT)
series by [TheCherno](https://www.youtube.com/user/TheChernoProject).

<a href="https://qiekn.github.io/ck-engine/" target="_blank">WIP: docs</a>

## Quickstart

### Prerequisites

- C++20 compiler (clang, gcc, MSVC, etc.)
- CMake 3.24+
- Git

<details>
<summary>MSYS2 UCRT64 setup (Windows)</summary>

Install [MSYS2](https://www.msys2.org/), then open **UCRT64** terminal and run:

```bash
pacman -S --needed \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-clang \
  mingw-w64-ucrt-x86_64-make \
  mingw-w64-ucrt-x86_64-gdb \
  git
```

</details>

### Build & Run

```bash
git clone --recursive https://github.com/qiekn/ck-engine.git
cd ck-engine
```

**MSYS2 UCRT64 Terminal:**

```bash
./run.sh
```

**PowerShell:**

```powershell
.\run.ps1
```

The script will automatically configure the CMake build directory on first run, then build and launch the editor.

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
