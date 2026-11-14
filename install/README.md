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

The scripts live under `install/`. Shared helpers are in `install/dependencies.lua`; nothing has to be installed to run them, Lua 5.4 is enough.

```text
install/
├── dependencies.lua    shared helpers, loaded by the four scripts below
├── unix.lua            -i install, -u uninstall
├── unix_copy.lua       plain copy
├── windows.lua         -i install, -u uninstall
└── windows_copy.lua    plain copy
```

Run an installation script from the repository root, so that `./include`, `./src`, `./lib.cmake` and `./CMakeLists.txt` are visible. Otherwise the script exits with code 2 and says what is missing. Uninstallation (`-u`) works from any directory.

Both methods install the same files: `include/`, `src/`, `lib.cmake` and `CMakeLists.txt`.

| | Script | Shell config | Uninstall |
|---|---|---|---|
| A | `install/unix_copy.lua`, `install/windows_copy.lua` | not modified | manual |
| B | `install/unix.lua`, `install/windows.lua` | `CEPO_CMAKE` appended | `-u` |

### A. `unix_copy.lua` / `windows_copy.lua`

Copies the files into a target directory. No shell configuration is read or written, and no environment variable is set. There is no corresponding uninstall script.

```sh
lua install/unix_copy.lua       # Unix
lua install\windows_copy.lua    # Windows
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

### B. `unix.lua` / `windows.lua`

Copies the files, appends `export CEPO_CMAKE=...` or `set -gx CEPO_CMAKE ...` to the startup script of the current shell, and records every change in a manifest so that `-u` can revert it.

```sh
lua install/unix.lua -i          # Unix
lua install\windows.lua -i       # Windows

lua install/unix.lua -h          # usage
```

`-i` performs the following steps:

1. Reads the manifest at `$XDG_DATA_HOME/cepo/manifest`, or `$HOME/.local/share/cepo/manifest` when `XDG_DATA_HOME` is unset. If it exists, the script asks whether to reinstall. The previous installation is only removed after step 3, so a mistyped path never costs a working installation.
2. If no manifest exists but `CEPO_CMAKE` is already set, the script exits with code 4 and asks for the variable to be removed manually, so that a variable it did not create is never overwritten. This holds on both platforms.
3. Asks for the installation path. The default is `$HOME/.local/share/cepo`. A custom path must be absolute; `~` is not expanded. The directory is created here, before anything is deleted.
4. Lists any of the four items that already exist in the target directory and asks before deleting them (default `N`); refusing exits with code 1. This is not limited to a fresh install: a reinstall into a *different* directory would otherwise merge into whatever is already there. Items the installation being replaced already owns are not listed, they are removed in step 5 anyway.
5. On a reinstall, removes the previous installation, then deletes the conflicting items from step 4. The copy that follows therefore cannot leave stale files behind, so headers deleted or renamed in the repository do not survive in the installation.
6. Appends a marked block to the startup script selected by `$SHELL`. Only `bash`, `zsh` and `fish` are supported; any other value exits with code 127.

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

7. Writes the manifest to `$XDG_DATA_HOME/cepo/manifest`, listing the installed files and the modified startup script. The manifest directory is set to mode 700 and the manifest to 600. The manifest is written *before* the startup script is touched, so a failure in step 6 still leaves an installation that `-u` can clean up; in that case the script exits with code 2 and says so.

If an existing `#[env-for-cepo-start]` marker has no matching `#[env-for-cepo-end]`, the startup script is left untouched and the script exits with code 2 instead of silently dropping everything after the marker.

The variable is read from the startup script at shell startup, so an existing shell does not see it until a new one is started:

```sh
exec $SHELL
```

`unix.lua -u` / `windows.lua -u` read the manifest and, in order, removes the marked block from the startup script, deletes the installed files, deletes the manifest, and removes the directories if they are empty. If the startup script cannot be reverted, the installed files and the manifest are kept so that the uninstall can be retried once the script is fixed; the script then exits with code 2.

```sh
lua install/unix.lua -u          # Unix
lua install\windows.lua -u       # Windows
```

After that, `CEPO_CMAKE` is still present in the current shell: a new shell would inherit the variable from the current process, while the line that used to override it has been removed. `exec $SHELL` therefore does not clear it, and the variable has to be unset explicitly:

```sh
# bash, zsh
unset CEPO_CMAKE

# fish
set -e CEPO_CMAKE
```

### Exit codes

`unix.lua` and `windows.lua` behave the same; `windows.lua` reads `%LOCALAPPDATA%` where `unix.lua` reads `$HOME`.

| Script | Code | Meaning |
|---|---|---|
| `unix_copy.lua`, `windows_copy.lua` | 0 | success |
| | 1 | invalid path, or cancelled by the user |
| | 2 | `$HOME` / `%LOCALAPPDATA%` unset, the repository root is incomplete, or a copy failed |
| `unix.lua -i`, `windows.lua -i` | 0 | success |
| | 1 | invalid path, cancelled by the user, or an unknown argument |
| | 2 | a command failed, or the files, the manifest, the startup script or the environment variable could not be written |
| | 3 | reinstall cancelled by the user |
| | 4 | `CEPO_CMAKE` is set but no manifest exists |
| | 127 | `$HOME` unset, or unsupported shell |
| `unix.lua -u`, `windows.lua -u` | 0 | success, or no installation found |
| | 1 | unknown argument |
| | 2 | `$HOME` / `%LOCALAPPDATA%` unset, the startup script or the environment variable could not be reverted, or a file could not be removed |

---

<a id="简体中文"></a>

## 简体中文

脚本位于 `install/` 下，公共部分在 `install/dependencies.lua`。运行它们不需要额外安装任何东西，有 Lua 5.4 即可。

```text
install/
├── dependencies.lua    公共部分，由下面四个脚本加载
├── unix.lua            -i 安装，-u 卸载
├── unix_copy.lua       仅复制
├── windows.lua         -i 安装，-u 卸载
└── windows_copy.lua    仅复制
```

请在仓库根目录运行安装脚本，以保证 `./include`、`./src`、`./lib.cmake`、`./CMakeLists.txt` 可见；否则脚本以退出码 2 终止并指出缺少哪一项。卸载（`-u`）可在任意目录运行。

两种方式安装的文件相同：`include/`、`src/`、`lib.cmake`、`CMakeLists.txt`。

| | 脚本 | Shell 配置 | 卸载 |
|---|---|---|---|
| A | `install/unix_copy.lua`、`install/windows_copy.lua` | 不修改 | 手动 |
| B | `install/unix.lua`、`install/windows.lua` | 追加 `CEPO_CMAKE` | `-u` |

### A. `unix_copy.lua` / `windows_copy.lua`

将文件复制到目标目录。不读取也不修改任何 shell 配置，不设置环境变量，因此没有对应的卸载脚本。

```sh
lua install/unix_copy.lua       # Unix
lua install\windows_copy.lua    # Windows
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

### B. `unix.lua` / `windows.lua`

复制文件的同时，向当前 shell 的启动脚本追加 `export CEPO_CMAKE=...` 或 `set -gx CEPO_CMAKE ...`，并把所有改动记录到清单中，供 `-u` 回滚。

```sh
lua install/unix.lua -i          # Unix
lua install\windows.lua -i       # Windows

lua install/unix.lua -h          # usage
```

`-i` 的执行步骤如下：

1. 读取清单 `$XDG_DATA_HOME/cepo/manifest`；未设置 `XDG_DATA_HOME` 时读取 `$HOME/.local/share/cepo/manifest`。若清单存在，询问是否重装。原有安装要等到第 3 步之后才被删除，因此路径输错不会毁掉可用的安装。
2. 若清单不存在但 `CEPO_CMAKE` 已被设置，脚本以退出码 4 终止，并提示手动删除该变量；它不会覆盖不是由自己创建的变量。两个平台行为一致。
3. 询问安装路径，默认为 `$HOME/.local/share/cepo`。自定义路径必须是绝对路径，`~` 不会被展开。目录在此处创建，早于任何删除动作。
4. 若目标目录已存在四个条目中的任意一个，先列出并询问是否删除（默认 `N`），拒绝则以退出码 1 终止。这一步不限于全新安装：重装到**另一个**目录时同样会检查，否则就会与那里原有的内容合并。被替换的安装自身已有的条目不在列表内，它们会在第 5 步被删除。
5. 重装时先删除原有安装，再删除第 4 步列出的冲突条目。因此随后的复制不会留下陈旧文件，仓库中已删除或改名的头文件不会残留在安装目录。
6. 依据 `$SHELL` 向对应的启动脚本追加标记块。仅支持 `bash`、`zsh`、`fish`，其它取值以退出码 127 终止。

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

7. 写入清单 `$XDG_DATA_HOME/cepo/manifest`，记录已安装文件与被修改的启动脚本。清单目录权限为 700，清单文件权限为 600。清单在改动启动脚本**之前**写入：即使第 6 步失败，也已存在可供 `-u` 清理的安装记录，脚本会以退出码 2 终止并提示回滚命令。

若已存在的 `#[env-for-cepo-start]` 没有配对的 `#[env-for-cepo-end]`，启动脚本不会被改写，脚本以退出码 2 终止，而不是静默丢弃标记之后的所有内容。

该变量在 shell 启动时从启动脚本中读取，因此已打开的 shell 不会立即生效，需要启动新 shell：

```sh
exec $SHELL
```

`unix.lua -u` / `windows.lua -u` 读取清单后，依次从启动脚本中删除标记块、删除已安装文件、删除清单，若目录已空则一并删除。若启动脚本无法回滚，则保留已安装文件与清单，以便修好脚本后重新执行卸载；此情况以退出码 2 终止。

```sh
lua install/unix.lua -u          # Unix
lua install\windows.lua -u       # Windows
```

卸载完成后，当前 shell 中仍存在 `CEPO_CMAKE`：新 shell 会从当前进程继承该变量，而原先覆盖它的那一行已被删除，因此 `exec $SHELL` 无法清除它，需要显式删除：

```sh
# bash、zsh
unset CEPO_CMAKE

# fish
set -e CEPO_CMAKE
```

### 退出码

`unix.lua` 与 `windows.lua` 行为一致，只是后者用 `%LOCALAPPDATA%` 代替 `$HOME`。

| 脚本 | 退出码 | 含义 |
|---|---|---|
| `unix_copy.lua`、`windows_copy.lua` | 0 | 成功 |
| | 1 | 路径非法，或用户取消 |
| | 2 | 未设置 `$HOME` / `%LOCALAPPDATA%`，仓库根目录不完整，或复制失败 |
| `unix.lua -i`、`windows.lua -i` | 0 | 成功 |
| | 1 | 路径非法、用户取消，或参数无法识别 |
| | 2 | 命令执行失败，或文件、清单、启动脚本、环境变量写入失败 |
| | 3 | 用户取消重装 |
| | 4 | `CEPO_CMAKE` 已设置但清单不存在 |
| | 127 | 未设置 `$HOME`，或不支持的 shell |
| `unix.lua -u`、`windows.lua -u` | 0 | 成功，或未找到安装 |
| | 1 | 参数无法识别 |
| | 2 | 未设置 `$HOME` / `%LOCALAPPDATA%`，启动脚本或环境变量回滚失败，或某个文件无法删除 |
