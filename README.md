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

The repository provides two scripts. Both install the same files: `include/`, `src/`, `lib.cmake` and `CMakeLists.txt`.

| | Script | Shell config | Uninstall |
|---|---|---|---|
| A | `copy_install.lua` | not modified | manual |
| B | `install.lua`, `uninstall.lua` | `CEPO_CMAKE` appended | provided |

#### A. `copy_install.lua`

Copies the files into a target directory. No shell configuration is read or written, and no environment variable is set. There is no corresponding uninstall script.

```sh
lua copy_install.lua
```

The script prompts twice:

1. Installation path. The default is `$HOME/.local/share/cepo` on Unix and `%LOCALAPPDATA%\cepo` on Windows. Answer `y` or press Enter to accept it, or `n` to enter an absolute path.
2. If any of the four items already exists in the target directory, it is listed and the script asks for confirmation before deleting it and copying the new one. The default is `N`.

`CEPO_CMAKE` is not set, so it must be provided where needed:

```sh
# bash, zsh
export CEPO_CMAKE="$HOME/.local/share/cepo/lib.cmake"

# fish
set -gx CEPO_CMAKE $HOME/.local/share/cepo/lib.cmake
```

```powershell
# PowerShell
[Environment]::SetEnvironmentVariable("CEPO_CMAKE", "$env:LOCALAPPDATA\cepo\lib.cmake", "User")
```

To uninstall, remove the directory, and remove the variable from your shell configuration if you set it there:

```sh
rm -rf "$HOME/.local/share/cepo"
```

#### B. `install.lua` and `uninstall.lua`

Copies the files, appends `export CEPO_CMAKE=...` or `set -gx CEPO_CMAKE ...` to the startup script of the current shell, and records every change in a manifest so that `uninstall.lua` can revert it.

```sh
lua install.lua
```

`install.lua` performs the following steps:

1. Reads the manifest at `$XDG_DATA_HOME/cepo/manifest`, or `$HOME/.local/share/cepo/manifest` when `XDG_DATA_HOME` is unset. If it exists, the script asks whether to reinstall; on confirmation the previous installation is removed first.
2. If no manifest exists but `CEPO_CMAKE` is already set, the script exits with code 4 and asks for the variable to be removed manually, so that a variable it did not create is never overwritten.
3. Asks for the installation path. The default is `$HOME/.local/share/cepo`. A custom path must be absolute; `~` is not expanded.
4. Appends a marked block to the startup script selected by `$SHELL`. Only `bash`, `zsh` and `fish` are supported; any other value exits with code 127.

   | Shell | File | Line appended |
   |---|---|---|
   | bash | `~/.bashrc` | `export CEPO_CMAKE='<dir>/lib.cmake'` |
   | zsh | `~/.zshrc` | `export CEPO_CMAKE='<dir>/lib.cmake'` |
   | fish | `~/.config/fish/config.fish` | `set -gx CEPO_CMAKE '<dir>/lib.cmake'` |

   The block is delimited as follows. Existing blocks are removed before a new one is written, so repeated installations do not accumulate.

   ```sh
   #[env-for-cepo-start]
   export CEPO_CMAKE='/home/user/.local/share/cepo/lib.cmake'
   #[env-for-cepo-end]
   ```

5. Writes the manifest to `$XDG_DATA_HOME/cepo/manifest`, listing the installed files and the modified startup script. The manifest directory is set to mode 700 and the manifest to 600.

The variable is read from the startup script at shell startup, so an existing shell does not see it until a new one is started:

```sh
exec $SHELL
```

`uninstall.lua` reads the manifest and, in order, removes the marked block from the startup script, deletes the installed files, deletes the manifest, and removes the directories if they are empty.

```sh
lua uninstall.lua
```

After that, `CEPO_CMAKE` is still present in the current shell: a new shell would inherit the variable from the current process, while the line that used to override it has been removed. `exec $SHELL` therefore does not clear it, and the variable has to be unset explicitly:

```sh
# bash, zsh
unset CEPO_CMAKE

# fish
set -e CEPO_CMAKE
```

#### Exit codes

| Script | Code | Meaning |
|---|---|---|
| `copy_install.lua` | 0 | success |
| | 1 | invalid path, or cancelled by the user |
| | 2 | `$HOME` / `%LOCALAPPDATA%` unset, or a copy failed |
| `install.lua` | 0 | success |
| | 1 | invalid path, or cancelled by the user |
| | 2 | a command failed, or a file or the manifest could not be written |
| | 3 | reinstall cancelled by the user |
| | 4 | `CEPO_CMAKE` is set but no manifest exists |
| | 127 | `$HOME` unset, or unsupported shell |
| `uninstall.lua` | 0 | success, or no installation found |
| | 2 | `$HOME` unset, or the startup script could not be reverted |

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

The result is `libcepo.a`. The test workflow is described in [TEST_WORK_README.md](TEST_WORK_README.md).

---

<a id="简体中文"></a>

## 简体中文

&emsp;&emsp;**CEPO**，全称为 **C++ Eleven Patches Open-source**，为 C++11 提供一批“C++11 能实现、但标准未包含”的组件，包括**新标准组件**（如 `cepo::make_unique`）和**扩展特性组件**（如 `cepo::callable_traits`）。CEPO 仅面向 C++11，对更高版本标准的兼容性不作保证；对于标准库中已有对应实现的组件，CEPO 不保证其行为与标准库完全一致。

### 环境要求

- Lua 5.4（推荐）
- CMake 3.20 或更高版本，接入 `lib.cmake` 时必需
- Unix-like 系统（Linux、macOS、Android/Termux）或 Windows

### 安装

仓库提供两个安装脚本，两者安装的文件相同：`include/`、`src/`、`lib.cmake`、`CMakeLists.txt`。

| | 脚本 | Shell 配置 | 卸载 |
|---|---|---|---|
| A | `copy_install.lua` | 不修改 | 手动 |
| B | `install.lua`、`uninstall.lua` | 追加 `CEPO_CMAKE` | 提供 |

#### A. `copy_install.lua`

将文件复制到目标目录。不读取也不修改任何 shell 配置，不设置环境变量，因此没有对应的卸载脚本。

```sh
lua copy_install.lua
```

脚本会进行两次询问：

1. 安装路径。默认值为 Unix 下的 `$HOME/.local/share/cepo` 与 Windows 下的 `%LOCALAPPDATA%\cepo`。输入 `y` 或直接回车采用默认值，输入 `n` 则手动输入一个绝对路径。
2. 若目标目录中已存在上述四个条目中的任意一个，脚本会先列出，再询问是否删除并覆盖。此项默认值为 `N`。

脚本不设置 `CEPO_CMAKE`，需要时自行提供：

```sh
# bash、zsh
export CEPO_CMAKE="$HOME/.local/share/cepo/lib.cmake"

# fish
set -gx CEPO_CMAKE $HOME/.local/share/cepo/lib.cmake
```

```powershell
# PowerShell
[Environment]::SetEnvironmentVariable("CEPO_CMAKE", "$env:LOCALAPPDATA\cepo\lib.cmake", "User")
```

卸载时删除该目录；若曾在 shell 配置中自行添加该变量，一并删除对应行：

```sh
rm -rf "$HOME/.local/share/cepo"
```

#### B. `install.lua` 与 `uninstall.lua`

复制文件的同时，向当前 shell 的启动脚本追加 `export CEPO_CMAKE=...` 或 `set -gx CEPO_CMAKE ...`，并把所有改动记录到清单中，供 `uninstall.lua` 回滚。

```sh
lua install.lua
```

`install.lua` 的执行步骤如下：

1. 读取清单 `$XDG_DATA_HOME/cepo/manifest`；未设置 `XDG_DATA_HOME` 时读取 `$HOME/.local/share/cepo/manifest`。若清单存在，询问是否重装，确认后先卸载原有安装。
2. 若清单不存在但 `CEPO_CMAKE` 已被设置，脚本以退出码 4 终止，并提示手动删除该变量；它不会覆盖不是由自己创建的变量。
3. 询问安装路径，默认为 `$HOME/.local/share/cepo`。自定义路径必须是绝对路径，`~` 不会被展开。
4. 依据 `$SHELL` 向对应的启动脚本追加标记块。仅支持 `bash`、`zsh`、`fish`，其它取值以退出码 127 终止。

   | Shell | 文件 | 追加内容 |
   |---|---|---|
   | bash | `~/.bashrc` | `export CEPO_CMAKE='<dir>/lib.cmake'` |
   | zsh | `~/.zshrc` | `export CEPO_CMAKE='<dir>/lib.cmake'` |
   | fish | `~/.config/fish/config.fish` | `set -gx CEPO_CMAKE '<dir>/lib.cmake'` |

   标记块格式如下。写入前会先删除已有标记块，因此重复安装不会累积。

   ```sh
   #[env-for-cepo-start]
   export CEPO_CMAKE='/home/user/.local/share/cepo/lib.cmake'
   #[env-for-cepo-end]
   ```

5. 写入清单 `$XDG_DATA_HOME/cepo/manifest`，记录已安装文件与被修改的启动脚本。清单目录权限为 700，清单文件权限为 600。

该变量在 shell 启动时从启动脚本中读取，因此已打开的 shell 不会立即生效，需要启动新 shell：

```sh
exec $SHELL
```

`uninstall.lua` 读取清单后，依次从启动脚本中删除标记块、删除已安装文件、删除清单，若目录已空则一并删除。

```sh
lua uninstall.lua
```

卸载完成后，当前 shell 中仍存在 `CEPO_CMAKE`：新 shell 会从当前进程继承该变量，而原先覆盖它的那一行已被删除，因此 `exec $SHELL` 无法清除它，需要显式删除：

```sh
# bash、zsh
unset CEPO_CMAKE

# fish
set -e CEPO_CMAKE
```

#### 退出码

| 脚本 | 退出码 | 含义 |
|---|---|---|
| `copy_install.lua` | 0 | 成功 |
| | 1 | 路径非法，或用户取消 |
| | 2 | 未设置 `$HOME` / `%LOCALAPPDATA%`，或复制失败 |
| `install.lua` | 0 | 成功 |
| | 1 | 路径非法，或用户取消 |
| | 2 | 命令执行失败，或文件、清单写入失败 |
| | 3 | 用户取消重装 |
| | 4 | `CEPO_CMAKE` 已设置但清单不存在 |
| | 127 | 未设置 `$HOME`，或不支持的 shell |
| `uninstall.lua` | 0 | 成功，或未找到安装 |
| | 2 | 未设置 `$HOME`，或启动脚本回滚失败 |

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

产物为 `libcepo.a`。测试流程见 [TEST_WORK_README.md](TEST_WORK_README.md)。
