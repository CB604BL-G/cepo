<h1 align="center">CEPO</h1>

<p align="center">
Patches for C++11, and more than just patches<br>
为C++11打造的补丁，但不仅仅是补丁
</p>

<p align="center">
  <a href="#english">English</a> · <a href="#简体中文">简体中文</a>
</p>

---

<a id="english"></a>

## English

&emsp;&emsp;**CEPO**, short for **C++ Eleven Patches Open-source**, provides components that are implementable in C++11 yet absent from the standard—covering **new-standard components** (e.g., `cepo::make_unique`) and **extended-feature components** (e.g., `cepo::callable_traits`). CEPO targets C++11 exclusively and makes no guarantee of compatibility with later standards. For components that have a counterpart in the standard library, CEPO does not guarantee identical behavior.

### Requirements

- Lua 5.4 (recommended)
- CMake 3.20 or later, required to consume `lib.cmake`
- A Unix-like system (Linux, macOS, Android/Termux) or Windows

### Installation

The installation scripts live in [`install/`](install/README.md): `unix.lua` (`-i` install, `-u` uninstall) and `unix_copy.lua` on Unix, `windows.lua` and `windows_copy.lua` on Windows. [`install/README.md`](install/README.md) documents the prompts, the manifest and rollback behaviour, and the exit codes of every script.

### Usage from CMake

With either method installed, a downstream project includes `lib.cmake` through `CEPO_CMAKE`:

```cmake
cmake_minimum_required(VERSION 3.20)
project(my_app LANGUAGES CXX)

include("$ENV{CEPO_CMAKE}")

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE cepo::cepo)
```

`lib.cmake` calls `add_subdirectory` on the CEPO source tree with `EXCLUDE_FROM_ALL` and defines the alias target `cepo::cepo`. Its first statement, `if(TARGET cepo::cepo) return()`, makes additional inclusions in the same project no-ops.

Method A installations, as well as a plain clone of the repository, can instead reference the source tree directly:

```cmake
add_subdirectory(/path/to/cepo cepo_build EXCLUDE_FROM_ALL)
target_link_libraries(my_app PRIVATE cepo::cepo)
```

### Building

```sh
cmake -S . -B .build
cmake --build .build
```

The result is `libcepo.a`. The test workflow is described in
[test/README.md](test/README.md), and the developer scripts (test runner,
item generator, public header generator) in
[script/README.md](script/README.md).

---

<a id="简体中文"></a>

## 简体中文

&emsp;&emsp;**CEPO**，全称为 **C++ Eleven Patches Open-source**，为 C++11 提供一批“C++11 能实现、但标准未包含”的组件，包括**新标准组件**（如 `cepo::make_unique`）和**扩展特性组件**（如 `cepo::callable_traits`）。CEPO 仅面向 C++11，对更高版本标准的兼容性不作保证；对于标准库中已有对应实现的组件，CEPO 不保证其行为与标准库完全一致。

### 环境要求

- Lua 5.4（推荐）
- CMake 3.20 或更高版本，接入 `lib.cmake` 时必需
- Unix-like 系统（Linux、macOS、Android/Termux）或 Windows

### 安装

安装脚本位于 [`install/`](install/README.md)：Unix 下为 `unix.lua`（`-i` 安装、`-u` 卸载）与 `unix_copy.lua`，Windows 下为 `windows.lua` 与 `windows_copy.lua`。交互流程、清单与回滚行为、各脚本的退出码见 [`install/README.md`](install/README.md)。

### 在 CMake 中使用

无论采用哪种方式安装，下游项目都通过 `CEPO_CMAKE` 接入 `lib.cmake`：

```cmake
cmake_minimum_required(VERSION 3.20)
project(my_app LANGUAGES CXX)

include("$ENV{CEPO_CMAKE}")

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE cepo::cepo)
```

`lib.cmake` 以 `EXCLUDE_FROM_ALL` 方式对 CEPO 源码目录调用 `add_subdirectory`，并定义别名目标 `cepo::cepo`。其首条语句 `if(TARGET cepo::cepo) return()` 使同一工程中的重复接入不产生额外效果。

采用方式 A 安装时，或直接使用仓库副本时，也可以不经过环境变量，直接引用源码目录：

```cmake
add_subdirectory(/path/to/cepo cepo_build EXCLUDE_FROM_ALL)
target_link_libraries(my_app PRIVATE cepo::cepo)
```

### 构建

```sh
cmake -S . -B .build
cmake --build .build
```

产物为 `libcepo.a`。测试流程见 [test/README.md](test/README.md)，开发脚本
（测试运行器、测试项生成器、公开头文件生成器）见
[script/README.md](script/README.md)。
