<h1 align="center">CEPO installation</h1>

<p align="center">
Installation scripts for CEPO<br>
CEPO 的安装脚本
</p>

<p align="center">
  <a href="#english">English</a> · <a href="#简体中文">简体中文</a>
</p>

---

<a id="english"></a>

## English

`install/unix.lua` and `install/windows.lua` copy the library into a directory
given by the caller. Shared code is in `install/dependencies.lua`; running the
scripts requires Lua 5.4.

They are executed from the repository root and verify that `include/`, `src/`,
`lib.cmake`, `CMakeLists.txt` and `LICENSE` are present. If one is missing, the
script exits with code 2 and names it.

```sh
lua install/unix.lua
lua install\windows.lua
```

### Installed items

| Item | Content |
|---|---|
| `include/` | public headers |
| `src/` | sources of `libcepo.a` |
| `lib.cmake` | file referenced by `CEPO_CMAKE` |
| `CMakeLists.txt` | top-level build file |
| `LICENSE` | MIT license text |

### Interaction

The script requests two inputs.

1. **Installation path.** The default is `$HOME/.local/share/cepo`, or
   `%LOCALAPPDATA%\cepo` on Windows. An empty reply or `y` accepts the default;
   `n` requests an absolute path. Relative paths and paths beginning with `~`
   are rejected, and the script terminates with exit code 1.
2. **Existing items.** If any of the items above is present in the target
   directory, all such items are listed and deletion requires confirmation. The
   default reply is `N`; refusal terminates with exit code 1 and leaves the
   target directory unchanged. The check runs on every installation, including a
   reinstallation into a different directory.

Copying is the final operation, so a refusal at either prompt leaves the target
directory unchanged.

### CEPO_CMAKE

`CEPO_CMAKE` contains the absolute path of the installed `lib.cmake`. The script
prints the corresponding statements and does not apply them.

```sh
# bash, zsh
export CEPO_CMAKE="$HOME/.local/share/cepo/lib.cmake"

# fish
set -gx CEPO_CMAKE $HOME/.local/share/cepo/lib.cmake
```

```powershell
# PowerShell
$env:CEPO_CMAKE = "$env:LOCALAPPDATA\cepo\lib.cmake"
```

Persistence across sessions requires adding the statement to the shell
configuration.

### Uninstallation

Delete the installation directory:

```sh
rm -rf "$HOME/.local/share/cepo"
```

If `CEPO_CMAKE` was added to the shell configuration, that line is removed
separately.

### Exit codes

| Code | Condition |
|---|---|
| 0 | successful installation |
| 1 | invalid path, or deletion not confirmed |
| 2 | `$HOME` / `%LOCALAPPDATA%` unset, repository root incomplete, or a copy failed |

### Installations produced by earlier versions

Versions prior to this one accepted `-i` and `-u` and recorded the installation
in a manifest at `$XDG_DATA_HOME/cepo/manifest`, with an `export CEPO_CMAKE=...`
block delimited by `#[env-for-cepo-start]` and `#[env-for-cepo-end]` in the
startup script. An installation created by such a version is removed manually:
delete the delimited block from the startup script, delete the installation
directory, delete the manifest if present, and set `CEPO_CMAKE` where required.

---

<a id="简体中文"></a>

## 简体中文

`install/unix.lua` 与 `install/windows.lua` 将库复制到调用者指定的目录。共用代码
位于 `install/dependencies.lua`；运行脚本需要 Lua 5.4。

脚本在仓库根目录执行，并检查 `include/`、`src/`、`lib.cmake`、`CMakeLists.txt`、
`LICENSE` 是否存在。缺失时以退出码 2 终止并指明缺失项。

```sh
lua install/unix.lua
lua install\windows.lua
```

### 安装内容

| 条目 | 内容 |
|---|---|
| `include/` | 公开头文件 |
| `src/` | `libcepo.a` 的源文件 |
| `lib.cmake` | `CEPO_CMAKE` 所引用的文件 |
| `CMakeLists.txt` | 顶层构建文件 |
| `LICENSE` | MIT 许可证文本 |

### 交互流程

脚本请求两项输入。

1. **安装路径。** 默认为 `$HOME/.local/share/cepo`，Windows 上为
   `%LOCALAPPDATA%\cepo`。输入空行或 `y` 接受默认值；输入 `n` 后需提供绝对
   路径。相对路径与以 `~` 开头的路径视为非法，脚本以退出码 1 终止。
2. **已存在的条目。** 目标目录中存在上述任一条目时，脚本列出全部冲突项并要求
   确认删除。默认回复为 `N`，拒绝时以退出码 1 终止，目标目录保持不变。该检查在
   每次安装时执行，包括重装到另一个目录的情形。

复制为最后一个操作，因此任一处拒绝确认时，目标目录保持不变。

### CEPO_CMAKE

`CEPO_CMAKE` 的内容为已安装 `lib.cmake` 的绝对路径。脚本打印相应语句，但不代为
写入。

```sh
# bash、zsh
export CEPO_CMAKE="$HOME/.local/share/cepo/lib.cmake"

# fish
set -gx CEPO_CMAKE $HOME/.local/share/cepo/lib.cmake
```

```powershell
# PowerShell
$env:CEPO_CMAKE = "$env:LOCALAPPDATA\cepo\lib.cmake"
```

若需跨会话生效，将上述语句加入 shell 配置文件。

### 卸载

删除安装目录：

```sh
rm -rf "$HOME/.local/share/cepo"
```

若曾将 `CEPO_CMAKE` 写入 shell 配置文件，需另行删除该行。

### 退出码

| 退出码 | 条件 |
|---|---|
| 0 | 安装成功 |
| 1 | 路径非法，或未确认删除 |
| 2 | 未设置 `$HOME` / `%LOCALAPPDATA%`，仓库根目录不完整，或复制失败 |

### 旧版本产生的安装

此前版本的脚本提供 `-i` 与 `-u`，并在 `$XDG_DATA_HOME/cepo/manifest` 记录清单，
同时向启动脚本写入由 `#[env-for-cepo-start]` 与 `#[env-for-cepo-end]` 界定的
`export CEPO_CMAKE=...` 块。由该版本产生的安装需手动清理：删除启动脚本中被界定
的块、删除安装目录、删除可能存在的清单，并在需要的位置设置 `CEPO_CMAKE`。
