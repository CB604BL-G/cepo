# Test Workflow

This document describes the test workflow for the `cepo` project: how test
items are laid out on disk, how to generate new ones, how to run them, and
which conventions must be respected.

The workflow assumes:

- Unix-like system (Linux, macOS, Android/Termux, ...)
- `lua` **5.4**
- `cmake` **3.20+**
- You run every command from the **project root** (the directory that
  contains `test.lua`, `make_test_item.lua`, `include/`, `src/`, ...)

---

## 1. Directory Layout

```

<project-root>/
├── include/                     cepo public headers (added to each item's -I)
├── src/                         cepo sources
├── tests/
│   ├── CMakeLists.txt           builds tests/gen_lib/libcepo.a
│   ├── gen_lib/                 generated build tree (do not commit)
│   │   ├── libcepo.a
│   │   ├── CMakeCache.txt
│   │   ├── CMakeFiles/
│   │   └── cepo_build/
│   └── <unit>/
│       └── <item>/
│           ├── CMakeLists.txt   required
│           ├── test.cpp         the only source file
│           ├── metadata.txt     optional (defaults are used if missing)
│           └── .build/          generated build tree (do not commit)
├── test.lua                     test runner
├── make_test_item.lua           item generator
├── quick_test.lua               runner for quick_test/
├── quick_test/                  ad-hoc check, links cepo via add_subdirectory
├── dependency_test.lua          runner for dependency_test/
└── dependency_test/             downstream consumption check via CEPO_CMAKE

```

- A **unit** is a directory directly under `tests/` (excluding `gen_lib`).
- An **item** is a directory directly under a unit.
- Each item is compiled and run **independently**. Items do not share
  build directories.
- `quick_test/` and `dependency_test/` are single-program checks, not test
  units. They are driven by their own runners, described in section 9.

---

## 2. Building `libcepo.a`

The shared static library is built by `test.lua` **once** per run, before
any item is compiled, using `tests/CMakeLists.txt`:

```

cmake -S tests -B tests/gen_lib
cmake --build tests/gen_lib

```

The result is `tests/gen_lib/libcepo.a`. Every item links against this
single archive.

- The build is **incremental**. `tests/gen_lib/` keeps CMake's cache and
  object files on purpose. Do not delete it between runs unless you want a
  full rebuild.
- If the library fails to build, `test.lua` prints the error and exits
  with code **4** without running any item.

`tests/CMakeLists.txt` is expected to look like:

```cmake
cmake_minimum_required(VERSION 3.20)

project(cepo_test_build LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Place libcepo.a directly under tests/gen_lib/
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/gen_lib")

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/.." cepo_build)
```

---

## 3. Item Layout

Each item lives in tests/<unit>/<item>/ and must contain:

File Required Notes
CMakeLists.txt yes Builds an executable named after the item dir
test.cpp yes The single source file
metadata.txt no Defaults apply if missing

The generated CMakeLists.txt (see make_test_item.lua) follows this
template:

```cmake
cmake_minimum_required(VERSION 3.20)

project(<ITEM_UPPERCASE> LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(<item> test.cpp)

target_include_directories(<item> PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../../../include"
)

target_link_libraries(<item> PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../../gen_lib/libcepo.a"
)
```

Key conventions:

· Executable name equals the item directory name. The runner looks
  for tests/<unit>/<item>/.build/<item> after building.
· CWD at run time is the item directory (tests/<unit>/<item>/), not
  the build directory and not the project root.
· Include path ../../../include reaches <project-root>/include from
  the item directory.
· Link path ../../gen_lib/libcepo.a reaches the shared archive.
· Because the archive is referenced by absolute path, CMake tracks it as
  a link dependency and relinks the item automatically whenever
  libcepo.a is rebuilt.

---

## 4. `metadata.txt`

metadata.txt is optional. If present, it must follow a strict format:

· One key=value per line.
· No spaces anywhere.
· No blank lines.
· No comments.

Recognized keys:

Key Value Default
expect_compile_error true / false false
timeout non-negative integer, milliseconds 1000

Any other key, any malformed line, any invalid value causes the item to
be marked FAIL with the corresponding metadata error, and the item is
skipped (no build, no run).

Example:

```
expect_compile_error=true
timeout=5000
```

---

## 5. Result Rules

For each item, test.lua first builds it, then applies:

expect_compile_error = true

· Compilation fails → PASS (compile failed as expected)
· Compilation succeeds → FAIL (expected compile error, but compilation succeeded)
· The executable is never run in this mode.

expect_compile_error = false

· Compilation fails → FAIL (configure failed / build failed),
  the compiler output is printed.
· Compilation succeeds → the executable is run:
  · Exit code 0 → PASS
  · Any other exit code → FAIL (exit code N)
  · Exceeds timeout ms → FAIL (timeout after Nms)

The runner prints one [RUN] <unit>.<item> line before the executable
starts, then streams the executable's stdout and stderr verbatim. It then
prints [PASS] ... or [FAIL] ... (reason).

---

## 6. Running Tests

Always run from the project root.

Run everything

```sh
lua test.lua
```

Scans tests/ for all units (every non-hidden directory except
gen_lib), then every item inside each unit.

Run all items of one unit

```sh
lua test.lua --uresult
```

Run specific items of one unit

```sh
lua test.lua --uresult --cconstruction --cunwrap
```

Run several units

```sh
lua test.lua --uresult --cconstruction --uresult_bad --cunwrap
```

CLI rules (strict)

· --u<name> and --c<name> must be glued: no space between --u
  and the name, no space between --c and the name.
· The name after --u or --c must not be empty.
· --c<name> cannot appear before any --u<name>.
· Duplicated --u<name> is an error.
· Duplicated --c<name> within the same unit is an error.
· Any argument that is not a valid --u or --c is an error.
· If a --u<name> or --c<name> refers to a unit/item that does not
  exist on disk, that is recorded as an error, but parsing continues.

All CLI errors are collected first. If there is at least one error,
test.lua prints every error, exits with code 2, and does not
build or run anything.

Exit codes

Code Meaning
0 All selected items passed
1 At least one item failed
2 CLI error; nothing was built or run
4 tests/gen_lib/libcepo.a failed to build

A fatal error while building the shared library is reported separately
from test failures: code 4 means the run never reached any item, while
codes 1 and 2 both originate from a completed or rejected run.

Summary

At the end, test.lua prints:

```
Total: X units, Y items (passed: P, failed: F)
```

If any items failed, it then prints them grouped by unit:

```
Failed items:
  result:
    construction (exit code 1)
    unwrap (timeout after 1000ms)
  result_bad:
    compile_only (expected compile error, but compilation succeeded)
```

---

## 7. Generating New Items

```sh
lua make_test_item.lua <unit>.<item> [<unit>.<item> ...]
```

· Each argument is unit.item.
· unit and item names must match ^[a-z_][a-z0-9_]*$:
  · start with a lowercase letter or underscore,
  · followed by lowercase letters, digits, or underscores.
  · Digits are allowed but not as the first character.
· The unit name gen_lib is reserved and is rejected. test.lua excludes
  that name from unit scanning, so an item created there would never
  run. A reserved unit name aborts the whole invocation: if one of
  several arguments uses it, no item is created, including the
  arguments that are otherwise valid.
· The target directories must not exist yet. make_test_item.lua
  checks all targets first; if any exists, nothing is created.
· Several items can be created in one invocation.

For each item, the generator creates:

· tests/<unit>/<item>/CMakeLists.txt
  · project name = item name in uppercase
  · target / executable name = item name in lowercase
· tests/<unit>/<item>/test.cpp:
  ```cpp
  auto main() -> int
  {
  }
  ```
· metadata.txt is not generated; add it manually when you need
  expect_compile_error=true or a custom timeout.

Examples:

```sh
lua make_test_item.lua result.construction
lua make_test_item.lua result.construction result.unwrap
```

---

## 8. Notes and Gotchas

Incremental builds

· tests/gen_lib/ is a real CMake build tree. Keep it between runs.
  Deleting it forces a full rebuild of libcepo.a.
· Each item keeps its own .build/. Do not mix contents between items.
· If an item starts behaving strangely (for example, a stale executable
  from a previous successful build is reused after a failed rebuild),
  delete tests/<unit>/<item>/.build/ and rerun.

Temporary files during runs

For each run, test.lua writes three temporary files inside the item
directory:

```
.test_runner.sh
.test_run_exit
.test_run_timeout
```

· They are removed before the run starts and after it finishes.
· If a run was interrupted (Ctrl+C, kill, crash), leftovers may remain.
  The next run removes them at startup and proceeds normally.
· Do not create files with these names yourself.

Timeout implementation

· Timeout is implemented with a background sleep process that sends
  SIGKILL to the item executable when the deadline is reached. The two
  outcomes (natural exit vs. timeout) are distinguished via
  .test_run_timeout.
· The default timeout is 1000 ms. Override it per item with
  timeout=<ms> in metadata.txt.
· A very small race exists if the executable exits at the exact moment
  the killer checks it; in practice this never matters for real tests.

Item constraints

· test.cpp is the only source file expected. If you need more sources,
  the item is probably doing too much -- split it.
· The unit directory itself is only a container. Plain files placed
  directly under tests/<unit>/ are ignored.
· gen_lib is excluded from unit scanning by name. Do not create a unit
  named gen_lib.

---

## 9. Standalone Checks

The checks under `tests/` all build CEPO from the working tree. That answers
one question - does the code compile and behave correctly - but says nothing
about whether a consumer of an *installed* CEPO can find it. The two checks in
this section cover the missing questions.

| Runner | Directory | Purpose | CEPO is consumed via |
|---|---|---|---|
| `quick_test.lua` | `quick_test/` | Ad-hoc check while editing | `add_subdirectory` of the repository root |
| `dependency_test.lua` | `dependency_test/` | Verify an installation produced by `install/unix.lua -i` is usable | `include("$ENV{CEPO_CMAKE}")` |

### `quick_test` — checking code right after writing it

`quick_test/main.cpp` is a scratch file. The intent is to paste a snippet,
run `lua quick_test.lua`, and get an answer immediately, without going through
`make_test_item.lua`, metadata, or the unit/item scan. It links CEPO through
`add_subdirectory`, so it compiles against the working tree and needs neither
`CEPO_CMAKE` nor a prior installation.

Use it while developing. Because it is a scratch file, its content is expected
to change frequently and is not a permanent test.

### `dependency_test` — checking that an installation resolves correctly

This check exists to answer a different question: after running `install/unix.lua -i`,
can an unrelated CMake project find CEPO through `CEPO_CMAKE` and link
`cepo::cepo`?

It deliberately does **not** use `add_subdirectory` on the repository. It uses
the same two lines a downstream project would write:

```cmake
include("$ENV{CEPO_CMAKE}")
target_link_libraries(dependency_test PRIVATE cepo::cepo)
```

That makes it sensitive to everything the installation actually places on
disk - `lib.cmake`, `CMakeLists.txt`, `include/` and `src/` - and to the
environment variable itself. A broken `lib.cmake`, a missing header, or a
`CEPO_CMAKE` that still points at an old directory all show up here and
nowhere else.

Run it after installing or reinstalling:

```sh
lua install/unix.lua -i
exec $SHELL

# In a new shell
lua dependency_test.lua
```

The runner checks `CEPO_CMAKE` before doing anything else, because a missing or
invalid value would otherwise surface as an unrelated CMake configure error:

Exit codes

Code Meaning
0 Success
1 Build or run failed
2 Invalid argument, or CEPO_CMAKE unset or not pointing at a file

Both runners accept the same options:

```sh
lua quick_test.lua [-b|-r|--] [args...]
```

· `-b`, `--build` — configure and build only.
· `-r`, `--run` — run only; requires a prior build.
· no flag — configure, build, then run.
· `--` — stop parsing; the remaining arguments are passed to the executable.

The build directory is `<directory>/.build/` and is not committed.

---

<a id="简体中文"></a>

# 测试流程

本文档描述 `cepo` 项目的测试流程：测试项在磁盘上如何组织、如何生成新的
测试项、如何运行，以及需要遵守的约定。

本文档假设：

- Unix-like 系统（Linux、macOS、Android/Termux 等）
- `lua` **5.4**
- `cmake` **3.20** 或更高版本
- 所有命令均在**项目根目录**执行，即包含 `test.lua`、`make_test_item.lua`、
  `include/`、`src/` 的目录

---

## 1. 目录结构

```
<项目根目录>/
├── include/                     CEPO 公开头文件（加入每个测试项的 -I）
├── src/                         CEPO 源文件
├── tests/
│   ├── CMakeLists.txt           构建 tests/gen_lib/libcepo.a
│   ├── gen_lib/                 生成的构建目录（不提交）
│   │   ├── libcepo.a
│   │   ├── CMakeCache.txt
│   │   ├── CMakeFiles/
│   │   └── cepo_build/
│   └── <unit>/
│       └── <item>/
│           ├── CMakeLists.txt   必需
│           ├── test.cpp         唯一的源文件
│           ├── metadata.txt     可选（缺失时使用默认值）
│           └── .build/          生成的构建目录（不提交）
├── test.lua                     测试运行器
├── make_test_item.lua           测试项生成器
├── quick_test.lua               `quick_test/` 的运行器
├── quick_test/                  临时验证，通过 add_subdirectory 链接 cepo
├── dependency_test.lua          `dependency_test/` 的运行器
└── dependency_test/             通过 CEPO_CMAKE 验证下游接入
```

- **unit** 是 `tests/` 下一级目录（不含 `gen_lib`）。
- **item** 是 unit 下一级目录。
- 每个 item 独立编译、独立运行，item 之间不共享构建目录。
- `quick_test/` 与 `dependency_test/` 是单一程序的检查，不属于测试 unit，
  由各自的运行器驱动，见第 9 节。

---

## 2. 构建 `libcepo.a`

`test.lua` 在编译任何 item 之前，通过 `tests/CMakeLists.txt` **一次性**构建
共享静态库：

```
cmake -S tests -B tests/gen_lib
cmake --build tests/gen_lib
```

产物为 `tests/gen_lib/libcepo.a`，所有 item 都链接这个归档文件。

- 构建是**增量的**。`tests/gen_lib/` 会保留 CMake 缓存与目标文件，除非需要
  全量重建，否则不要在两次运行之间删除它。
- 若该库构建失败，`test.lua` 会输出错误并以退出码 **4** 终止，不运行任何 item。

`tests/CMakeLists.txt` 的内容应为：

```cmake
cmake_minimum_required(VERSION 3.20)

project(cepo_test_build LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 将 libcepo.a 直接放到 tests/gen_lib/ 下
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/gen_lib")

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/.." cepo_build)
```

---

## 3. item 的构成

每个 item 位于 `tests/<unit>/<item>/`，包含以下文件：

| 文件 | 必需 | 说明 |
|---|---|---|
| `CMakeLists.txt` | 是 | 构建与 item 目录同名的可执行文件 |
| `test.cpp` | 是 | 唯一的源文件 |
| `metadata.txt` | 否 | 缺失时使用默认值 |

生成的 `CMakeLists.txt`（见 `make_test_item.lua`）遵循以下模板：

```cmake
cmake_minimum_required(VERSION 3.20)

project(<ITEM_UPPERCASE> LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(<item> test.cpp)

target_include_directories(<item> PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../../../include"
)

target_link_libraries(<item> PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../../gen_lib/libcepo.a"
)
```

关键约定：

- 可执行文件名等于 item 目录名。运行器在构建后查找
  `tests/<unit>/<item>/.build/<item>`。
- 运行时的当前工作目录是 item 目录（`tests/<unit>/<item>/`），既不是构建
  目录也不是项目根目录。
- 头文件路径 `../../../include` 从 item 目录到达 `<项目根目录>/include`。
- 链接路径 `../../gen_lib/libcepo.a` 指向共享归档文件。
- 由于归档文件以绝对路径引用，CMake 将其视为链接依赖，`libcepo.a` 重建后
  item 会自动重新链接。

---

## 4. `metadata.txt`

`metadata.txt` 可选。若存在，必须遵循严格格式：

- 每行一个 `key=value`。
- 任何位置都不允许出现空格。
- 不允许空行。
- 不允许注释。

可识别的键：

| 键 | 取值 | 默认值 |
|---|---|---|
| `expect_compile_error` | `true` / `false` | `false` |
| `timeout` | 非负整数，单位为毫秒 | `1000` |

出现其它键、格式错误的行或非法取值，该 item 会被标记为 FAIL 并附带相应的
metadata 错误，随后跳过（不构建、不运行）。

示例：

```
expect_compile_error=true
timeout=5000
```

---

## 5. 结果判定规则

对每个 item，`test.lua` 先构建，再按以下规则判定：

`expect_compile_error = true`

- 编译失败 → PASS（编译如期失败）
- 编译成功 → FAIL（期望编译错误，但编译成功）
- 该模式下不会运行可执行文件。

`expect_compile_error = false`

- 编译失败 → FAIL（configure 失败 / build 失败），并输出编译器信息。
- 编译成功 → 运行可执行文件：
  - 退出码 0 → PASS
  - 其它退出码 → FAIL（退出码 N）
  - 超过 `timeout` 毫秒 → FAIL（超时）

运行器会在可执行文件启动前输出一行 `[RUN] <unit>.<item>`，随后原样转发
可执行文件的 stdout 与 stderr，最后输出 `[PASS] ...` 或
`[FAIL] ...（原因）`。

---

## 6. 运行测试

始终在项目根目录执行。

运行全部

```sh
lua test.lua
```

扫描 `tests/` 下的所有 unit（除 `gen_lib` 以外的非隐藏目录），再扫描每个
unit 下的所有 item。

运行某个 unit 的全部 item

```sh
lua test.lua --uresult
```

运行某个 unit 下的指定 item

```sh
lua test.lua --uresult --cconstruction --cunwrap
```

运行多个 unit

```sh
lua test.lua --uresult --cconstruction --uresult_bad --cunwrap
```

CLI 规则（严格）：

- `--u<name>` 与 `--c<name>` 必须紧接：`--u` 与名字之间、`--c` 与名字之间
  都不能有空格。
- `--u` 或 `--c` 后的名字不能为空。
- `--c<name>` 不能出现在任何 `--u<name>` 之前。
- 重复的 `--u<name>` 视为错误。
- 同一 unit 内重复的 `--c<name>` 视为错误。
- 任何不是合法 `--u` 或 `--c` 的参数视为错误。
- 若 `--u<name>` 或 `--c<name>` 指向磁盘上不存在的 unit/item，该情况会被
  记录为错误，但解析继续进行。

所有 CLI 错误会先被收集。只要存在至少一个错误，`test.lua` 会输出全部错误，
以退出码 **2** 终止，并且不构建、不运行任何内容。

退出码：

| 退出码 | 含义 |
|---|---|
| 0 | 所有选中的 item 均通过 |
| 1 | 至少有一个 item 失败 |
| 2 | CLI 错误；未构建、未运行任何内容 |
| 4 | `tests/gen_lib/libcepo.a` 构建失败 |

退出码 1 与 2 的区别在于：2 表示这次运行根本没有开始，属于调用方参数问题；
1 表示运行已经完成，确实存在失败的 item。构建共享库时的致命错误单独用 4
表示，即该次运行未进入到任何 item。

汇总输出：

运行结束时，`test.lua` 输出：

```
Total: X units, Y items (passed: P, failed: F)
```

若有 item 失败，随后按 unit 分组列出：

```
Failed items:
  result:
    construction (exit code 1)
    unwrap (timeout after 1000ms)
  result_bad:
    compile_only (expected compile error, but compilation succeeded)
```

---

## 7. 生成新的 item

```sh
lua make_test_item.lua <unit>.<item> [<unit>.<item> ...]
```

- 每个参数形式为 `unit.item`。
- unit 与 item 名称必须匹配 `^[a-z_][a-z0-9_]*$`：
  - 以小写字母或下划线开头，
  - 后接小写字母、数字或下划线，
  - 允许数字，但不能作为首字符。
- unit 名 `gen_lib` 为保留名，会被拒绝。`test.lua` 在扫描 unit 时排除了该
  名称，因此建在该 unit 下的 item 永远不会被执行。使用保留名会终止整个调用：
  若多个参数中有一个使用它，则不会创建任何 item，其余合法参数也不会生效。
- 目标目录必须尚不存在。`make_test_item.lua` 会先检查全部目标；只要有一个
  已存在，就不创建任何内容。
- 一次调用可以创建多个 item。

对每个 item，生成器创建：

- `tests/<unit>/<item>/CMakeLists.txt`
  - project 名称为 item 名的大写形式
  - target／可执行文件名称为 item 名的小写形式
- `tests/<unit>/<item>/test.cpp`：
  ```cpp
  auto main() -> int
  {
  }
  ```
- 不生成 `metadata.txt`；需要 `expect_compile_error=true` 或自定义超时时
  手动添加。

示例：

```sh
lua make_test_item.lua result.construction
lua make_test_item.lua result.construction result.unwrap
```

---

## 8. 注意事项

增量构建

- `tests/gen_lib/` 是真实的 CMake 构建目录，应在多次运行之间保留。删除它
  会强制全量重建 `libcepo.a`。
- 每个 item 保留自己的 `.build/`，不要在不同 item 之间混用内容。
- 若某个 item 行为异常（例如一次失败的重新构建之后仍复用了上一次成功构建
  留下的旧可执行文件），删除 `tests/<unit>/<item>/.build/` 后重试。

运行期间的临时文件

每次运行，`test.lua` 会在 item 目录内写入三个临时文件：

```
.test_runner.sh
.test_run_exit
.test_run_timeout
```

- 这些文件在运行开始前与结束后都会删除。
- 若运行被中断（Ctrl+C、kill、崩溃），可能残留。下次运行会在启动时删除
  它们并正常继续。
- 不要自行创建同名文件。

超时实现

- 超时由后台 sleep 进程实现：到达截止时间后向 item 可执行文件发送 SIGKILL。
  两种结果（自然退出与超时）通过 `.test_run_timeout` 区分。
- 默认超时为 1000 毫秒，可通过 `metadata.txt` 中的 `timeout=<ms>` 按 item
  覆盖。
- 若可执行文件恰好在 killer 检查的瞬间退出，存在极小的竞态；实际测试中
  不会产生影响。

item 的约束

- `test.cpp` 是唯一预期的源文件。若需要更多源文件，说明该 item 承担的职责
  过多，应拆分。
- unit 目录本身只是容器，直接放在 `tests/<unit>/` 下的普通文件会被忽略。
- `gen_lib` 按名称被排除在 unit 扫描之外，不要创建名为 `gen_lib` 的 unit。

---

## 9. 独立检查

`tests/` 下的检查全部从工作区源码构建 CEPO。它们能回答一个问题——代码是否
编译、行为是否正确——但对“使用已安装 CEPO 的项目能否找到它”没有任何说明。
本节的两个检查补上这部分。

| 运行器 | 目录 | 用途 | 接入 CEPO 的方式 |
|---|---|---|---|
| `quick_test.lua` | `quick_test/` | 写代码过程中的临时验证 | 对仓库根目录 `add_subdirectory` |
| `dependency_test.lua` | `dependency_test/` | 验证 `install/unix.lua -i` 安装结果可用 | `include("$ENV{CEPO_CMAKE}")` |

### `quick_test` —— 写完代码立即验证

`quick_test/main.cpp` 是一个草稿文件。它的用途是贴一段代码，执行
`lua quick_test.lua`，立刻得到结果，不必经过 `make_test_item.lua`、metadata
或 unit/item 扫描。它通过 `add_subdirectory` 链接 CEPO，因此直接针对工作区
源码编译，既不需要 `CEPO_CMAKE`，也不需要事先安装。

开发过程中使用它。作为草稿文件，它的内容会频繁变动，不作为长期测试。

### `dependency_test` —— 验证安装后能否正确找到依赖

这个检查回答的是另一个问题：执行 `install/unix.lua -i` 之后，一个无关的 CMake 工程
能否通过 `CEPO_CMAKE` 找到 CEPO 并链接 `cepo::cepo`？

它刻意**不**对仓库使用 `add_subdirectory`，而是使用下游项目会写的同样两行：

```cmake
include("$ENV{CEPO_CMAKE}")
target_link_libraries(dependency_test PRIVATE cepo::cepo)
```

因此它对安装实际落到磁盘上的内容都敏感——`lib.cmake`、`CMakeLists.txt`、
`include/`、`src/`——同时也对变量本身敏感。`lib.cmake` 损坏、头文件缺失、
`CEPO_CMAKE` 仍指向旧目录，这些问题只会在这里暴露，其它检查发现不了。

在安装或重装之后运行它：

```sh
lua install/unix.lua -i
exec $SHELL

# 在新 shell 中
lua dependency_test.lua
```

运行器在做任何事之前先检查 `CEPO_CMAKE`，否则变量缺失或非法时会以与问题无关
的 CMake configure 错误形式暴露：

| 退出码 | 含义 |
|---|---|
| 0 | 成功 |
| 1 | 构建或运行失败 |
| 2 | 参数非法，或 `CEPO_CMAKE` 未设置、未指向文件 |

两个运行器接受相同的选项：

```sh
lua quick_test.lua [-b|-r|--] [args...]
```

- `-b`、`--build` —— 只 configure 并构建。
- `-r`、`--run` —— 只运行，需要先前已构建。
- 不带参数 —— 构建后运行。
- `--` —— 停止解析，其余参数传给可执行文件。

构建目录为 `<目录>/.build/`，不提交。
