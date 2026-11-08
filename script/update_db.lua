-- Generate compile_commands.json in the project root.
--
-- Usage:
--   lua script/update_db.lua
--
-- Editors and language servers read compile_commands.json to find the exact
-- compiler invocation of every translation unit. The file is produced by CMake
-- into the build tree, so this script configures the project with
-- CMAKE_EXPORT_COMPILE_COMMANDS=ON and copies the result next to the sources.
--
-- The build directory (.build/) and compile_commands.json are gitignored; they
-- are local artefacts, regenerate them whenever the build files change.
--
-- Exit codes:
--   0  compile_commands.json was written
--   1  cmake failed, or it did not produce compile_commands.json
--   2  the file could not be copied

-- The scripts live in <project-root>/script/, so the project root is the
-- parent of this file. Running them from the project root is the normal case
-- and keeps the paths short: ROOT collapses to "." then.
local script_dir = (arg[0] or ""):match("^(.*)[/\\][^/\\]*$") or "."
local ROOT = (script_dir == "script") and "." or (script_dir .. "/..")

-- Path of `rel` relative to the project root.
local function at(rel)
	return (ROOT == ".") and rel or (ROOT .. "/" .. rel)
end

local function fail(msg, code)
	io.stderr:write("error: " .. msg .. "\n")
	os.exit(code or 1)
end

local function sh_quote(s)
	return "'" .. s:gsub("'", "'\\''") .. "'"
end

local BUILD_DIR = at(".build")
local OUT_FILE = at("compile_commands.json")
local GENERATED = BUILD_DIR .. "/compile_commands.json"

-- 1. Configure the project, asking CMake for the compilation database.
local cmd = table.concat({
	"cmake",
	" -S " .. sh_quote(at(".")),
	" -B " .. sh_quote(BUILD_DIR),
	" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
})
local ok, why, code = os.execute(cmd)
if not (ok == true or ok == 0) then
	local reason = (why == "signal") and ("killed by signal " .. tostring(code))
		or ("exited with code " .. tostring(code))
	fail("cmake failed (" .. reason .. ")", 1)
end

-- 2. Copy the generated database to the project root.
local src = io.open(GENERATED, "rb")
if not src then
	fail(GENERATED .. " was not produced; is CMAKE_EXPORT_COMPILE_COMMANDS supported?", 1)
end
local content = src:read("*a")
src:close()

local dst, err = io.open(OUT_FILE, "wb")
if not dst then
	fail("cannot write " .. OUT_FILE .. ": " .. tostring(err), 2)
end
dst:write(content)
dst:close()

print("created compile_commands.json in the project root")
