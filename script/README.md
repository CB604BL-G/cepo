<h1 align="center">CEPO scripts</h1>

<p align="center">
Developer scripts for CEPO<br>
CEPO 的开发脚本
</p>

<p align="center">
  <a href="#english">English</a> · <a href="#简体中文">简体中文</a>
</p>

---

<a id="english"></a>

## English

Every Lua script of the project lives here. None of them needs anything but
`lua` 5.4; they drive `cmake` for the parts that have to be compiled.

The scripts resolve the project root from their own location, so they can be
started from any working directory. Running them from the repository root is
still the normal case and keeps the printed paths short:

```sh
lua script/test.lua
```

| Script | Purpose | Arguments |
|---|---|---|
| `test.lua` | Build `test/gen_lib/libcepo.a` once, then build and run test items | `--u<unit>`, `--c<item>` |
| `quick_test.lua` | Build and run the scratch program in `test/quick_test/` | `-b`, `-r`, `--` |
| `dependency_test.lua` | Build and run the downstream-consumption check in `test/dependency_test/` | `-b`, `-r`, `--` |
| `make_test_item.lua` | Create test item directories | `unit.item ...` |
| `remove_test_item.lua` | Delete test item directories | `unit.item ...` |
| `make_header.lua` | Create a new public header with its include guard | `path/to/header.hpp` |
| `update_db.lua` | Regenerate `compile_commands.json` for editors and language servers | none |

### `test.lua` — test runner

Builds the shared archive into `test/gen_lib/`, then builds and runs every
selected item under `test/units/<unit>/<item>/`. Without arguments it runs
everything.

```sh
lua script/test.lua
lua script/test.lua --uresult
lua script/test.lua --uresult --cslots --cfactory
```

| Code | Meaning |
|---|---|
| 0 | all selected items passed |
| 1 | at least one item failed |
| 2 | CLI error; nothing was built or run |
| 4 | `test/gen_lib/libcepo.a` failed to build |

The full workflow, the item layout, `metadata.txt` and the result rules are
described in [`../test/README.md`](../test/README.md).

### `quick_test.lua` — scratch check

Builds and runs `test/quick_test/`, which links CEPO through
`add_subdirectory` and therefore works on a working tree that was never
installed.

```sh
lua script/quick_test.lua             # build, then run
lua script/quick_test.lua -b          # build only
lua script/quick_test.lua -r          # run only (requires a prior build)
lua script/quick_test.lua -- --flag   # pass arguments to the program
```

Exit codes: 0 success, 1 build or run failed, 2 unknown argument.

### `dependency_test.lua` — installed-consumption check

Builds and runs `test/dependency_test/`, which consumes CEPO the way a
downstream project does: `include("$ENV{CEPO_CMAKE}")` and the `cepo::cepo`
alias. It therefore requires `CEPO_CMAKE` to point at a `lib.cmake`, which
`install/unix.lua -i` or `install/windows.lua -i` sets for you.

```sh
lua script/dependency_test.lua
```

The flags are the same as for `quick_test.lua`. Exit codes: 0 success,
1 build or run failed, 2 invalid argument or `CEPO_CMAKE` unset or not a file.

### `make_test_item.lua` — create items

Creates `test/units/<unit>/<item>/` with a generated `CMakeLists.txt` and an
empty `test.cpp`. The include and link paths inside the generated CMakeLists
depend on the layout, which is why items are not created by hand.

```sh
lua script/make_test_item.lua result.construction
lua script/make_test_item.lua result.construction result.unwrap
```

Names must match `^[a-z_][a-z0-9_]*$`, and the target directories must not
exist yet; all arguments are validated before anything is created. Exit codes:
0 success, 1 invalid argument or an existing target.

### `remove_test_item.lua` — delete items

The counterpart of `make_test_item.lua`, with the same argument syntax and the
opposite expectation: the directories must exist. All arguments are validated
first, so a typo in a list deletes nothing. A unit whose last item is removed
is deleted too, because an empty unit is not a valid state.

```sh
lua script/remove_test_item.lua result.construction
```

Exit codes: 0 success, 1 invalid argument or a missing target.

Item directories are created and deleted only by these two scripts; doing it
by any other means is undefined. See
[`../test/README.md`](../test/README.md) section 7.

### `make_header.lua` — create a header

Creates `include/cepo/<path>` containing the license header and an include
guard built from the path, and creates the intermediate directories. The
path may be given with or without the `include/cepo/` prefix.

```sh
lua script/make_header.lua result/new_component.hpp
lua script/make_header.lua include/cepo/result/new_component.hpp
```

The file must not exist yet. Exit codes: 0 success, 1 invalid path or an
existing file.

### `update_db.lua` — regenerate the compilation database

Configures the project with `CMAKE_EXPORT_COMPILE_COMMANDS=ON` into `.build/`
and copies the generated database to `compile_commands.json` in the project
root, where editors and language servers pick it up.

```sh
lua script/update_db.lua
```

Run it after touching any `CMakeLists.txt`, after adding a source file, or
whenever a language server reports stale flags. Both `.build/` and
`compile_commands.json` are gitignored local artefacts.

Exit codes: 0 success, 1 `cmake` failed or produced no database, 2 the file
could not be copied.

### Conventions

- Every script exits `0` on success and `1` on a user error, and uses `2` for
  a command-line problem where that is distinguishable.
- Arguments are never taken from the environment, except `CEPO_CMAKE` in
  `dependency_test.lua`.
- Paths are resolved from the script location through a small local helper,
  so no script depends on the current directory being the project root.

---

<a id="简体中文"></a>

## 简体中文

项目所有的 Lua 脚本都在这里。除 `lua` 5.4 之外不需要任何东西；需要编译的部分
由脚本调用 `cmake` 完成。

脚本根据自身位置推导项目根目录，因此可以在任意工作目录下启动。从仓库根目录
运行仍是最常见的方式，而且打印出来的路径最短：

```sh
lua script/test.lua
```

| 脚本 | 作用 | 参数 |
|---|---|---|
| `test.lua` | 先构建 `test/gen_lib/libcepo.a`，再构建并运行测试项 | `--u<unit>`、`--c<item>` |
| `quick_test.lua` | 构建并运行 `test/quick_test/` 里的草稿程序 | `-b`、`-r`、`--` |
| `dependency_test.lua` | 构建并运行 `test/dependency_test/` 里的下游接入检查 | `-b`、`-r`、`--` |
| `make_test_item.lua` | 创建测试项目录 | `unit.item ...` |
| `remove_test_item.lua` | 删除测试项目录 | `unit.item ...` |
| `make_header.lua` | 创建新公开头文件及其 include guard | `path/to/header.hpp` |
| `update_db.lua` | 为编辑器和语言服务器重新生成 `compile_commands.json` | 无 |

### `test.lua` —— 测试运行器

把共享归档构建到 `test/gen_lib/`，然后构建并运行 `test/units/<unit>/<item>/`
下所有被选中的测试项。不带参数时运行全部。

```sh
lua script/test.lua
lua script/test.lua --uresult
lua script/test.lua --uresult --cslots --cfactory
```

| 退出码 | 含义 |
|---|---|
| 0 | 所有被选中的测试项都通过 |
| 1 | 至少一个测试项失败 |
| 2 | 命令行错误，未构建、未运行任何内容 |
| 4 | `test/gen_lib/libcepo.a` 构建失败 |

完整流程、测试项的构成、`metadata.txt` 与结果判定规则见
[`../test/README.md`](../test/README.md)。

### `quick_test.lua` —— 临时验证

构建并运行 `test/quick_test/`，它通过 `add_subdirectory` 链接 CEPO，因此在
从未安装过的工作区里也能用。

```sh
lua script/quick_test.lua             # 先构建再运行
lua script/quick_test.lua -b          # 只构建
lua script/quick_test.lua -r          # 只运行（需要先前构建过）
lua script/quick_test.lua -- --flag   # 把参数传给程序
```

退出码：0 成功，1 构建或运行失败，2 参数无法识别。

### `dependency_test.lua` —— 验证安装后的接入

构建并运行 `test/dependency_test/`，它以下游项目的方式消费 CEPO：
`include("$ENV{CEPO_CMAKE}")` 加 `cepo::cepo` 别名。因此要求 `CEPO_CMAKE` 指向
某个 `lib.cmake`，这个变量由 `install/unix.lua -i` 或 `install/windows.lua -i`
设置。

```sh
lua script/dependency_test.lua
```

参数与 `quick_test.lua` 相同。退出码：0 成功，1 构建或运行失败，2 参数非法，
或 `CEPO_CMAKE` 未设置／不指向文件。

### `make_test_item.lua` —— 创建测试项

创建 `test/units/<unit>/<item>/`，内含生成的 `CMakeLists.txt` 与空的
`test.cpp`。生成的 CMakeLists 里的头文件与链接路径取决于目录层级，这正是
测试项不由手工创建的原因。

```sh
lua script/make_test_item.lua result.construction
lua script/make_test_item.lua result.construction result.unwrap
```

名称必须匹配 `^[a-z_][a-z0-9_]*$`，目标目录必须尚不存在；所有参数都校验通过
后才开始创建。退出码：0 成功，1 参数非法或目标已存在。

### `remove_test_item.lua` —— 删除测试项

`make_test_item.lua` 的对应物，参数语法相同而预期相反：目标目录必须存在。
所有参数先校验，因此一次传入多个 item 时只要有一个写错就什么也不删。若某个
unit 的最后一个 item 被删除，该 unit 目录也会被删除，因为空 unit 不是合法状态。

```sh
lua script/remove_test_item.lua result.construction
```

退出码：0 成功，1 参数非法或目标不存在。

测试项目录只由这两个脚本创建和删除；其它任何方式行为未定义。见
[`../test/README.md`](../test/README.md) 第 7 节。

### `make_header.lua` —— 创建头文件

创建 `include/cepo/<path>`，内容为版权声明与由路径生成的 include guard，并
自动创建中间目录。路径可以带或不带 `include/cepo/` 前缀。

```sh
lua script/make_header.lua result/new_component.hpp
lua script/make_header.lua include/cepo/result/new_component.hpp
```

文件必须尚不存在。退出码：0 成功，1 路径非法或文件已存在。

### `update_db.lua` —— 重新生成编译数据库

以 `CMAKE_EXPORT_COMPILE_COMMANDS=ON` 将工程配置到 `.build/`，再把生成的数据库
复制到项目根目录的 `compile_commands.json`，供编辑器与语言服务器使用。

```sh
lua script/update_db.lua
```

改动任何 `CMakeLists.txt`、新增源文件，或语言服务器报告编译参数过期时，重新
运行一次即可。`.build/` 与 `compile_commands.json` 都是被 gitignore 的本地产物。

退出码：0 成功，1 `cmake` 失败或未生成数据库，2 文件复制失败。

### 约定

- 所有脚本成功时退出 `0`，用户输入有误时退出 `1`；能区分开时，命令行问题
  使用 `2`。
- 除 `dependency_test.lua` 的 `CEPO_CMAKE` 外，脚本不读取环境变量。
- 路径通过一个小的本地辅助函数从脚本位置推导，因此没有脚本依赖当前工作目录
  必须是项目根目录。
