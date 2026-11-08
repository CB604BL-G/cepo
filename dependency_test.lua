-- Dependency test runner for the cepo project.
--
-- Usage:
--   lua dependency_test.lua
--   lua dependency_test.lua [-b|-r|--] [args...]
--
--   -b, --build   configure and build only
--   -r, --run     run only (requires a prior build)
--   (no flag)     configure, build, then run
--   --            stop parsing; the remaining arguments are passed to the
--                 executable
--
-- This test consumes cepo the way a downstream project would: through
-- `include("$ENV{CEPO_CMAKE}")` and the `cepo::cepo` alias target. It therefore
-- requires CEPO_CMAKE to point at lib.cmake, which install/unix.lua -i and
-- install/windows.lua -i set, and which unix_copy.lua / windows_copy.lua leave
-- to the user.
--
-- Exit codes:
--   0  success
--   1  build or run failed
--   2  invalid argument, or CEPO_CMAKE is unset or does not point at a file

local script_dir = arg[0]:match("^(.*)[/\\]") or "."

local function shq(s)
	return "'" .. s:gsub("'", "'\\''") .. "'"
end

local function run(cmd, what)
	local ok, why, code = os.execute(cmd)
	if ok then return true end
	local reason = (why == "signal") and ("killed by signal " .. tostring(code))
		or ("exited with code " .. tostring(code))
	io.stderr:write(("Failed to %s (%s)\n"):format(what, reason))
	return false
end

local function is_file_basic(path)
	local fh = io.open(path, "rb")
	if not fh then return false end
	fh:close()
	return true
end

local src_dir = script_dir .. "/test/dependency_test"
local build_dir = src_dir .. "/.build"
local exe = build_dir .. "/dependency_test"

local function check_env()
	local cepo_cmake = os.getenv("CEPO_CMAKE")
	if not cepo_cmake or cepo_cmake == "" then
		io.stderr:write("CEPO_CMAKE is not set\n")
		io.stderr:write("This test consumes cepo through CEPO_CMAKE; set it first, e.g.:\n")
		io.stderr:write("  export CEPO_CMAKE=\"$HOME/.local/share/cepo/lib.cmake\"\n")
		return false
	end
	if not is_file_basic(cepo_cmake) then
		io.stderr:write("CEPO_CMAKE does not point at a file: " .. cepo_cmake .. "\n")
		return false
	end
	return true
end

local function build()
	local cmd = table.concat({
		"cmake -S " .. shq(src_dir) .. " -B " .. shq(build_dir),
		"cmake --build " .. shq(build_dir),
	}, " && ")
	return run(cmd, "build")
end

local function execute(passthrough)
	if not is_file_basic(exe) then
		io.stderr:write("executable not found: " .. exe .. " (did you build?)\n")
		return false
	end
	local cmd = shq(exe)
	for _, a in ipairs(passthrough) do
		cmd = cmd .. " " .. shq(a)
	end
	return run(cmd, "run")
end

-- ── 参数解析 ──
local do_build, do_run = false, false
local passthrough = {}
local i = 1
while i <= #arg do
	local a = arg[i]
	if a == "-b" or a == "--build" then
		do_build = true
	elseif a == "-r" or a == "--run" then
		do_run = true
	elseif a == "-h" or a == "--help" then
		print("Usage: lua dependency_test.lua [-b|-r|--] [args...]")
		print("  -b, --build   build only")
		print("  -r, --run     run only (requires prior build)")
		print("  (no flag)     build then run")
		print("  --            stop parsing; remaining args go to the executable")
		os.exit(0)
	elseif a == "--" then
		for j = i + 1, #arg do passthrough[#passthrough + 1] = arg[j] end
		break
	else
		io.stderr:write("unknown argument: " .. a .. "\n")
		os.exit(2)
	end
	i = i + 1
end

if not do_build and not do_run then
	do_build, do_run = true, true
end

if not check_env() then os.exit(2) end

if do_build and not build() then os.exit(1) end
if do_run and not execute(passthrough) then os.exit(1) end
