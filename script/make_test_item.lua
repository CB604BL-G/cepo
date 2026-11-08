-- Create a new test item under test/units/<unit>/<item>/.
--
-- Usage:
--   lua script/make_test_item.lua unit.item [unit.item ...]
--
-- Each argument is "unit.item"; unit and item names must consist only of
-- lowercase ASCII letters, numbers and underscores. The target directories must
-- not exist yet.
--
-- An item directory is owned by this script and by remove_test_item.lua: it
-- carries a generated CMakeLists.txt whose include and link paths depend on the
-- layout above. Creating or deleting items by any other means is undefined.

-- The scripts live in <project-root>/script/, so the project root is the
-- parent of this file. Running them from the project root is the normal case
-- and keeps the paths short: ROOT collapses to "." then.
local script_dir = (arg[0] or ""):match("^(.*)[/\\][^/\\]*$") or "."
local ROOT = (script_dir == "script") and "." or (script_dir .. "/..")

-- Path of `rel` relative to the project root.
local function at(rel)
	return (ROOT == ".") and rel or (ROOT .. "/" .. rel)
end

local UNITS_DIR = at("test/units")

local function fail(msg)
	io.stderr:write("error: " .. msg .. "\n")
	os.exit(1)
end

local function sh_quote(s)
	return "'" .. s:gsub("'", "'\\''") .. "'"
end

local function is_dir(path)
	local fh = io.popen("test -d " .. sh_quote(path) .. " && echo yes || echo no")
	if not fh then return false end
	local r = fh:read("*a")
	fh:close()
	return r:find("yes") ~= nil
end

local function mkdir_p(path)
	os.execute("mkdir -p " .. sh_quote(path))
end

local function write_file(path, content)
	local fh, err = io.open(path, "w")
	if not fh then
		fail("cannot write " .. path .. ": " .. tostring(err))
	end
	fh:write(content)
	fh:close()
end

local function check_name(s, what)
	if s:match("^[a-z_][a-z0-9_]*$") == nil then
		fail(what .. " name '" .. s .. "' is invalid (must start with a lowercase letter or underscore, followed by lowercase letters, digits, or underscores)")
	end
end

if #arg == 0 then
	io.stderr:write("usage: lua script/make_test_item.lua unit.item [unit.item ...]\n")
	os.exit(1)
end

local tasks = {}
local seen = {}

for _, a in ipairs(arg) do
	local unit, item = a:match("^([^.]+)%.([^.]+)$")
	if not unit then
		fail("invalid argument '" .. a .. "': expected format unit.item")
	end
	check_name(unit, "unit")
	check_name(item, "item")
	local key = unit .. "." .. item
	if seen[key] then
		fail("duplicate argument '" .. a .. "'")
	end
	seen[key] = true
	tasks[#tasks + 1] = { unit = unit, item = item }
end

-- Make sure none of the target directories exist yet.
for _, t in ipairs(tasks) do
	local idir = UNITS_DIR .. "/" .. t.unit .. "/" .. t.item
	if is_dir(idir) then
		fail("item directory already exists: " .. idir)
	end
end

local function item_cmakelists(item)
	local project_name = item:upper()
	local target_name = item
	return table.concat({
		"cmake_minimum_required(VERSION 3.20)",
		"",
		"project(" .. project_name .. " LANGUAGES CXX)",
		"",
		"set(CMAKE_CXX_STANDARD 11)",
		"set(CMAKE_CXX_STANDARD_REQUIRED ON)",
		"set(CMAKE_CXX_EXTENSIONS OFF)",
		"",
		"add_executable(" .. target_name .. " test.cpp)",
		"",
		"target_include_directories(" .. target_name .. " PRIVATE",
		"\t\"${CMAKE_CURRENT_SOURCE_DIR}/../../../../include\"",
		")",
		"",
		"target_link_libraries(" .. target_name .. " PRIVATE",
		"\t\"${CMAKE_CURRENT_SOURCE_DIR}/../../../gen_lib/libcepo.a\"",
		")",
		"",
	}, "\n")
end

local TEST_CPP = "auto main() -> int\n{\n}\n"

for _, t in ipairs(tasks) do
	local idir = UNITS_DIR .. "/" .. t.unit .. "/" .. t.item
	mkdir_p(idir)
	write_file(idir .. "/CMakeLists.txt", item_cmakelists(t.item))
	write_file(idir .. "/test.cpp", TEST_CPP)
	print("created " .. t.unit .. "." .. t.item .. " at " .. idir)
end
