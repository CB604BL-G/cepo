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
└── make_test_item.lua           item generator

```

- A **unit** is a directory directly under `tests/` (excluding `gen_lib`).
- An **item** is a directory directly under a unit.
- Each item is compiled and run **independently**. Items do not share
  build directories.

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

3. Item Layout

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

4. metadata.txt

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

5. Result Rules

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

6. Running Tests

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
test.lua prints every error, exits with code 1, and does not
build or run anything.

Exit codes

Code Meaning
0 All selected items passed
1 CLI error, or at least one item failed
4 tests/gen_lib/libcepo.a failed to build

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

7. Generating New Items

```sh
lua make_test_item.lua <unit>.<item> [<unit>.<item> ...]
```

· Each argument is unit.item.
· unit and item names must match ^[a-z_][a-z0-9_]*$:
  · start with a lowercase letter or underscore,
  · followed by lowercase letters, digits, or underscores.
  · Digits are allowed but not as the first character.
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

8. Notes and Gotchas

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
