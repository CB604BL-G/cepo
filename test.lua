-- Test runner for the cepo project.
--
-- Usage:
--   lua test.lua
--   lua test.lua --u<unit> [--c<item> ...] [--u<unit> ...]
--
-- Units live in test/units/. The shared library is built into test/gen_lib/,
-- which is a sibling of test/units/ and therefore never scanned as a unit.
--
-- Exit codes:
--   0  all selected items passed
--   1  at least one item failed
--   2  CLI errors; nothing was built or run
--   4  test/gen_lib/libcepo.a failed to build

-- Everything the runner touches is derived from these two.
local UNITS_DIR = "test/units"
local GEN_LIB_DIR = "test/gen_lib"

local function sh_quote(s)
	return "'" .. s:gsub("'", "'\\''") .. "'"
end

-- Run a shell command, capture stdout+stderr, return (ok, output, exit_code).
local function capture(cmd)
	local fh = io.popen(cmd .. " 2>&1", "r")
	if not fh then
		return false, "", -1
	end
	local out = fh:read("*a")
	local a, b, c = fh:close()
	if a == true then
		return true, out, 0
	end
	if type(a) == "number" then
		-- Lua 5.1
		return a == 0, out, a
	end
	if b == "exit" then
		return false, out, c
	end
	return false, out, -1
end

local function is_dir(path)
	local fh = io.popen("test -d " .. sh_quote(path) .. " && echo yes || echo no")
	if not fh then return false end
	local r = fh:read("*a")
	fh:close()
	return r:find("yes") ~= nil
end

local function is_file(path)
	local fh = io.popen("test -f " .. sh_quote(path) .. " && echo yes || echo no")
	if not fh then return false end
	local r = fh:read("*a")
	fh:close()
	return r:find("yes") ~= nil
end

local function list_subdirs(path)
	local p = io.popen("ls -1 " .. sh_quote(path) .. " 2>/dev/null")
	if not p then return {} end
	local names = {}
	for line in p:lines() do
		if line ~= "" then names[#names + 1] = line end
	end
	p:close()
	local dirs = {}
	for _, n in ipairs(names) do
		if n:sub(1, 1) ~= "." and is_dir(path .. "/" .. n) then
			dirs[#dirs + 1] = n
		end
	end
	return dirs
end

--------------------------------------------------------------------------------
-- CLI parsing
--------------------------------------------------------------------------------

local function parse_cli(argv)
	local errors = {}
	local units = {}
	local unit_by_name = {}
	local current = nil

	for i, a in ipairs(argv) do
		local u = a:match("^%-%-u(.+)$")
		local c = a:match("^%-%-c(.+)$")
		if u then
			if u == "" then
				errors[#errors + 1] = "argument #" .. i .. ": empty unit name in '" .. a .. "'"
			elseif unit_by_name[u] then
				errors[#errors + 1] = "argument #" .. i .. ": duplicate unit '" .. u .. "'"
			else
				current = { name = u, items = {}, item_set = {} }
				units[#units + 1] = current
				unit_by_name[u] = current
			end
		elseif c then
			if c == "" then
				errors[#errors + 1] = "argument #" .. i .. ": empty item name in '" .. a .. "'"
			elseif not current then
				errors[#errors + 1] = "argument #" .. i .. ": '--c' used before any '--u'"
			elseif current.item_set[c] then
				errors[#errors + 1] = "argument #" .. i .. ": duplicate item '" .. c .. "' in unit '" .. current.name .. "'"
			else
				current.item_set[c] = true
				current.items[#current.items + 1] = c
			end
		else
			errors[#errors + 1] = "argument #" .. i .. ": unexpected argument '" .. a .. "'"
		end
	end

	for _, u in ipairs(units) do
		local udir = UNITS_DIR .. "/" .. u.name
		if not is_dir(udir) then
			errors[#errors + 1] = "unit not found: '" .. u.name .. "'"
		else
			for _, it in ipairs(u.items) do
				if not is_dir(udir .. "/" .. it) then
					errors[#errors + 1] = "item not found: '" .. u.name .. "." .. it .. "'"
				end
			end
		end
	end

	return units, errors
end

--------------------------------------------------------------------------------
-- Metadata
--------------------------------------------------------------------------------

local function parse_metadata(path)
	local meta = { expect_compile_error = false, timeout = 1000 }
	if not is_file(path) then
		return meta
	end
	local fh = io.open(path, "r")
	if not fh then
		return nil, "cannot open " .. path
	end
	local line_no = 0
	for line in fh:lines() do
		line_no = line_no + 1
		if line == "" then
			fh:close()
			return nil, "metadata.txt:" .. line_no .. ": empty line not allowed"
		end
		local key, val = line:match("^([%a_][%w_]*)=(.+)$")
		if not key then
			fh:close()
			return nil, "metadata.txt:" .. line_no .. ": invalid line"
		end
		if key == "expect_compile_error" then
			if val == "true" then
				meta.expect_compile_error = true
			elseif val == "false" then
				meta.expect_compile_error = false
			else
				fh:close()
				return nil, "metadata.txt:" .. line_no .. ": expect_compile_error must be true or false"
			end
		elseif key == "timeout" then
			local n = tonumber(val)
			if not n or n < 0 or n ~= math.floor(n) then
				fh:close()
				return nil, "metadata.txt:" .. line_no .. ": timeout must be a non-negative integer"
			end
			meta.timeout = n
		else
			fh:close()
			return nil, "metadata.txt:" .. line_no .. ": unknown key '" .. key .. "'"
		end
	end
	fh:close()
	return meta
end

--------------------------------------------------------------------------------
-- Build libcepo.a
--------------------------------------------------------------------------------

local function build_lib()
	local ok, out, code = capture("cmake -S test -B " .. GEN_LIB_DIR)
	if not ok then
		io.stderr:write("configure " .. GEN_LIB_DIR .. " failed (exit " .. tostring(code) .. ")\n")
		io.stderr:write(out)
		return false
	end
	ok, out, code = capture("cmake --build " .. GEN_LIB_DIR)
	if not ok then
		io.stderr:write("build libcepo.a failed (exit " .. tostring(code) .. ")\n")
		io.stderr:write(out)
		return false
	end
	if not is_file(GEN_LIB_DIR .. "/libcepo.a") then
		io.stderr:write("error: " .. GEN_LIB_DIR .. "/libcepo.a was not produced\n")
		return false
	end
	return true
end

--------------------------------------------------------------------------------
-- Build and run a single item
--------------------------------------------------------------------------------

local function build_item(idir)
	local bdir = idir .. "/.build"
	local ok, out, code = capture("cmake -S " .. sh_quote(idir) .. " -B " .. sh_quote(bdir))
	if not ok then
		return false, "configure", out
	end
	ok, out, code = capture("cmake --build " .. sh_quote(bdir))
	if not ok then
		return false, "build", out
	end
	return true
end

-- Run the item executable with a timeout.
-- Returns (true) on success, (nil, reason) on failure.
local function run_item(idir, exe_name, timeout_ms)
	local exit_file = idir .. "/.test_run_exit"
	local to_file = idir .. "/.test_run_timeout"
	local runner = idir .. "/.test_runner.sh"

	-- Clean up any leftovers from a previous interrupted run.
	os.remove(exit_file)
	os.remove(to_file)
	os.remove(runner)

	local timeout_s = string.format("%.3f", timeout_ms / 1000.0)
	local fh = io.open(runner, "w")
	if not fh then
		return nil, "cannot create runner script"
	end
	fh:write("#!/bin/sh\n")
	fh:write("cd \"$(dirname \"$0\")\" || exit 99\n")
	fh:write("./.build/" .. exe_name .. " &\n")
	fh:write("child=$!\n")
	fh:write("(\n")
	fh:write("  sleep " .. timeout_s .. "\n")
	fh:write("  if kill -0 $child 2>/dev/null; then\n")
	fh:write("    kill -9 $child 2>/dev/null\n")
	fh:write("    echo timeout > .test_run_timeout\n")
	fh:write("  fi\n")
	fh:write(") &\n")
	fh:write("killer=$!\n")
	fh:write("wait $child 2>/dev/null\n")
	fh:write("ec=$?\n")
	fh:write("if [ ! -f .test_run_timeout ]; then\n")
	fh:write("  echo $ec > .test_run_exit\n")
	fh:write("fi\n")
	fh:write("kill $killer 2>/dev/null\n")
	fh:write("wait $killer 2>/dev/null\n")
	fh:close()

	local p = io.popen("sh " .. sh_quote(runner) .. " 2>&1")
	if not p then
		os.remove(runner)
		return nil, "cannot start runner"
	end
	for line in p:lines() do
		io.write(line, "\n")
	end
	p:close()

	local timed_out = is_file(to_file)
	local ec = nil
	if not timed_out and is_file(exit_file) then
		local ef = io.open(exit_file, "r")
		if ef then
			ec = tonumber(ef:read("*a"):match("%d+"))
			ef:close()
		end
	end

	os.remove(exit_file)
	os.remove(to_file)
	os.remove(runner)

	if timed_out then
		return nil, "timeout after " .. timeout_ms .. "ms"
	end
	if ec == nil then
		return nil, "no exit code recorded"
	end
	if ec ~= 0 then
		return nil, "exit code " .. ec
	end
	return true
end

--------------------------------------------------------------------------------
-- Test a single item
--------------------------------------------------------------------------------

local function test_one_item(unit_name, item_name, idir, results)
	local tag = unit_name .. "." .. item_name

	local function fail(reason)
		print("[FAIL] " .. tag .. " (" .. reason .. ")")
		results[#results + 1] = { unit = unit_name, item = item_name, status = "fail", reason = reason }
	end
	local function pass(reason)
		if reason then
			print("[PASS] " .. tag .. " (" .. reason .. ")")
		else
			print("[PASS] " .. tag)
		end
		results[#results + 1] = { unit = unit_name, item = item_name, status = "pass" }
	end

	if not is_file(idir .. "/CMakeLists.txt") then
		fail("missing CMakeLists.txt")
		return
	end

	local meta, err = parse_metadata(idir .. "/metadata.txt")
	if not meta then
		fail(err)
		return
	end

	local ok, stage, out = build_item(idir)
	if meta.expect_compile_error then
		if ok then
			fail("expected compile error, but compilation succeeded")
		else
			pass("compile failed as expected")
		end
		return
	end

	if not ok then
		print("[FAIL] " .. tag .. " (" .. stage .. " failed)")
		if out then io.write(out) end
		results[#results + 1] = { unit = unit_name, item = item_name, status = "fail", reason = stage .. " failed" }
		return
	end

	print("[RUN] " .. tag)
	local rok, rreason = run_item(idir, item_name, meta.timeout)
	if rok then
		pass()
	else
		fail(rreason)
	end
end

--------------------------------------------------------------------------------
-- Main
--------------------------------------------------------------------------------

local units, errors = parse_cli(arg)
if #errors > 0 then
	for _, e in ipairs(errors) do
		io.stderr:write("error: " .. e .. "\n")
	end
	os.exit(2)
end

-- No --u given: scan test/units/ for all units.
if #units == 0 then
	for _, name in ipairs(list_subdirs(UNITS_DIR)) do
		units[#units + 1] = { name = name, items = {}, item_set = {} }
	end
end

if not build_lib() then
	os.exit(4)
end

local results = {}

for _, unit in ipairs(units) do
	local items
	if #unit.items > 0 then
		items = unit.items
	else
		items = list_subdirs(UNITS_DIR .. "/" .. unit.name)
	end
	for _, item_name in ipairs(items) do
		local idir = UNITS_DIR .. "/" .. unit.name .. "/" .. item_name
		test_one_item(unit.name, item_name, idir, results)
	end
end

-- Summary
local passed, failed = 0, 0
local failed_by_unit = {}
for _, r in ipairs(results) do
	if r.status == "pass" then
		passed = passed + 1
	elseif r.status == "fail" then
		failed = failed + 1
		failed_by_unit[r.unit] = failed_by_unit[r.unit] or {}
		table.insert(failed_by_unit[r.unit], r)
	end
end

print("")
print(string.format("Total: %d units, %d items (passed: %d, failed: %d)",
	#units, #results, passed, failed))

if failed > 0 then
	print("")
	print("Failed items:")
	for _, unit in ipairs(units) do
		local fs = failed_by_unit[unit.name]
		if fs and #fs > 0 then
			print("  " .. unit.name .. ":")
			for _, r in ipairs(fs) do
				print("    " .. r.item .. " (" .. r.reason .. ")")
			end
		end
	end
end

os.exit(failed > 0 and 1 or 0)
