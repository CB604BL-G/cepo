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

local src_dir = script_dir .. "/quick_test"
local build_dir = src_dir .. "/.build"
local exe = build_dir .. "/quick_test"

local function build()
	local cmd = table.concat({
		"cmake -S " .. shq(src_dir) .. " -B " .. shq(build_dir),
		"cmake --build " .. shq(build_dir),
	}, " && ")
	return run(cmd, "build")
end

local function execute(passthrough)
	if not os.rename(exe, exe) then
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
		print("Usage: lua quick_test.lua [-b|-r|--] [args...]")
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

if do_build and not build() then os.exit(1) end
if do_run and not execute(passthrough) then os.exit(1) end
