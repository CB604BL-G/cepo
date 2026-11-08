-- Remove a test item from test/units/<unit>/<item>/.
--
-- Usage:
--   lua remove_test_item.lua unit.item [unit.item ...]
--
-- The arguments are parsed exactly like make_test_item.lua: each one is
-- "unit.item", unit and item names must consist only of lowercase ASCII
-- letters, numbers and underscores, and duplicates are rejected.
--
-- Unlike make_test_item.lua, the target directories must exist: every argument
-- is validated before anything is deleted, so a typo cannot leave a half-applied
-- removal behind.
--
-- An item directory is owned by this script and by make_test_item.lua. When the
-- last item of a unit is removed, the unit directory goes too, because a unit
-- without items is not a valid state for test.lua.
--
-- Exit codes:
--   0  all requested items were removed
--   1  invalid argument, or an item directory does not exist

local UNITS_DIR = "test/units"

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

-- Subdirectories of `path`, dotfiles excluded. An empty table means either an
-- empty directory or a missing one; callers only use it on existing units.
local function list_subdirs(path)
	local p = io.popen("ls -1 " .. sh_quote(path) .. " 2>/dev/null")
	if not p then return {} end
	local names = {}
	for line in p:lines() do
		if line ~= "" and line:sub(1, 1) ~= "." then names[#names + 1] = line end
	end
	p:close()
	local dirs = {}
	for _, n in ipairs(names) do
		if is_dir(path .. "/" .. n) then dirs[#dirs + 1] = n end
	end
	return dirs
end

local function rm_rf(path)
	local ok, why, code = os.execute("rm -rf " .. sh_quote(path))
	if ok then return true end
	local reason = (why == "signal") and ("killed by signal " .. tostring(code))
		or ("exited with code " .. tostring(code))
	fail("cannot remove " .. path .. " (" .. reason .. ")")
end

local function check_name(s, what)
	if s:match("^[a-z_][a-z0-9_]*$") == nil then
		fail(what .. " name '" .. s .. "' is invalid (must start with a lowercase letter or underscore, followed by lowercase letters, digits, or underscores)")
	end
end

if #arg == 0 then
	io.stderr:write("usage: lua remove_test_item.lua unit.item [unit.item ...]\n")
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

-- Validate everything before deleting anything.
for _, t in ipairs(tasks) do
	local idir = UNITS_DIR .. "/" .. t.unit .. "/" .. t.item
	if not is_dir(idir) then
		fail("item directory does not exist: " .. idir)
	end
end

-- Track which units were touched, so each one is inspected once at the end.
local touched = {}
local touched_order = {}

for _, t in ipairs(tasks) do
	local idir = UNITS_DIR .. "/" .. t.unit .. "/" .. t.item
	rm_rf(idir)
	print("removed " .. t.unit .. "." .. t.item)
	if not touched[t.unit] then
		touched[t.unit] = true
		touched_order[#touched_order + 1] = t.unit
	end
end

for _, unit in ipairs(touched_order) do
	local udir = UNITS_DIR .. "/" .. unit
	if #list_subdirs(udir) == 0 then
		rm_rf(udir)
		print("removed empty unit " .. unit)
	end
end
