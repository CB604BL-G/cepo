-- AI-generated
-- CEPO installation scripts -- shared components.
--
-- This file is not meant to be run directly. The two entry points load it with
-- dofile() and use the table it returns:
--
--   install/unix.lua       copy the library into a directory
--   install/windows.lua    the same on Windows
--
-- Both scripts only copy files into the target directory.
--
-- The scripts are expected to be started from the repository root, e.g.
--   lua install/unix.lua
-- so that "./include", "./src", "./lib.cmake" and "./CMakeLists.txt" exist.

local M = {}

-- Items copied by every installation, relative to the repository root. LICENSE
-- is included because it covers the headers and the archive.
M.ITEMS = { "include", "src", "lib.cmake", "CMakeLists.txt", "LICENSE" }

----------------------------------------------------------------------
-- General helpers
----------------------------------------------------------------------

function M.logo()
	local blue = "\27[0;34;1m"
	local reset = "\27[0m"
	io.stdout:write(blue)
	print("  ▇▇▇▇▇▇▇▇▇▇╗▇▇▇▇▇▇▇▇▇▇▇▇╗▇▇▇▇▇▇▇▇▇▇╗    ▇▇▇▇▇▇▇▇╗")
	print("▇▇▇▇▇▇▇▇▇▇▇▇║▇▇▇▇▇▇▇▇▇▇▇▇║▇▇▇▇▇▇▇▇▇▇▇▇╗▇▇▇▇▇▇▇▇▇▇▇▇╗")
	print("▇▇▇▇╔═══════╝▇▇▇▇╔═══════╝▇▇▇▇╔═══▇▇▇▇║▇▇▇▇╔═══▇▇▇▇║")
	print("▇▇▇▇║        ▇▇▇▇║        ▇▇▇▇║   ▇▇▇▇║▇▇▇▇║   ▇▇▇▇║")
	print("▇▇▇▇║        ▇▇▇▇▇▇▇▇▇▇▇▇╗▇▇▇▇▇▇▇▇▇▇▇▇║▇▇▇▇║   ▇▇▇▇║")
	print("▇▇▇▇║        ▇▇▇▇▇▇▇▇▇▇▇▇║▇▇▇▇▇▇▇▇▇▇╔═╝▇▇▇▇║   ▇▇▇▇║")
	print("▇▇▇▇║        ▇▇▇▇╔═══════╝▇▇▇▇╔═════╝  ▇▇▇▇║   ▇▇▇▇║")
	print("▇▇▇▇║        ▇▇▇▇║        ▇▇▇▇║        ▇▇▇▇║   ▇▇▇▇║")
	print("▇▇▇▇▇▇▇▇▇▇▇▇╗▇▇▇▇▇▇▇▇▇▇▇▇╗▇▇▇▇║        ▇▇▇▇▇▇▇▇▇▇▇▇║")
	print(" ╚▇▇▇▇▇▇▇▇▇▇║▇▇▇▇▇▇▇▇▇▇▇▇║▇▇▇▇║         ╚▇▇▇▇▇▇▇▇╔═╝")
	print("  ╚═════════╝╚═══════════╝╚═══╝          ╚═══════╝")
	io.stdout:write(reset)
	io.stdout:flush()
end

function M.trim(s)
	return (s or ""):match("^%s*(.-)%s*$") or ""
end

function M.ask(prompt, default_yes)
	io.write(prompt)
	io.flush()
	local answer = M.trim(io.read("*l")):lower()
	if answer == "" then return default_yes end
	return answer == "y" or answer == "yes"
end

function M.required_env(name, exit_code)
	local value = os.getenv(name)
	if not (value and value ~= "") then
		io.stderr:write("$" .. name .. " is not set\n")
		os.exit(exit_code)
	end
	return value
end

-- Items of M.ITEMS that already exist inside `dir`. `sep` is the platform
-- separator, `exists` the platform existence check. Returns the absolute paths
-- of the conflicts, so the caller can show them before deleting anything.
function M.existing_items(dir, sep, exists)
	local found = {}
	for _, name in ipairs(M.ITEMS) do
		local target = dir .. sep .. name
		if exists(target) then
			found[#found + 1] = target
		end
	end
	return found
end

-- Aborts with a readable message when the script is not started from a
-- checkout that actually contains the items to install.
local function report_missing(script_path, missing)
	if #missing == 0 then return end

	io.stderr:write("Missing " .. table.concat(missing, ", ")
		.. " in the current directory.\n")
	io.stderr:write("Run the script from the repository root, e.g.\n")
	io.stderr:write("  lua " .. (script_path or "install/unix.lua") .. "\n")
	os.exit(2)
end

----------------------------------------------------------------------
-- Unix helpers
----------------------------------------------------------------------

M.unix = {}

function M.unix.shq(s)
	return "'" .. s:gsub("'", "'\\''") .. "'"
end

function M.unix.run(cmd)
	local r = os.execute(cmd)
	return r == true or r == 0
end

function M.unix.run_or_die(cmd, what)
	local r, why, code = os.execute(cmd)
	if r == true or r == 0 then return end
	local reason = (why == "signal")
		and ("killed by signal " .. tostring(code))
		or  ("exited with code " .. tostring(code))
	io.stderr:write(("Failed to %s (%s)\n"):format(what, reason))
	os.exit(2)
end

function M.unix.exists(path)
	return M.unix.run("test -e " .. M.unix.shq(path))
end

function M.unix.mkdir_p(path)
	return M.unix.run("mkdir -p " .. M.unix.shq(path))
end

function M.unix.require_repository_root(script_path)
	local missing = {}
	for _, name in ipairs(M.ITEMS) do
		if not M.unix.exists("./" .. name) then
			missing[#missing + 1] = name
		end
	end
	report_missing(script_path, missing)
end

----------------------------------------------------------------------
-- Windows helpers
----------------------------------------------------------------------

M.windows = {}

function M.windows.quote(s)
	return "'" .. s:gsub("'", "''") .. "'"
end

function M.windows.run(cmd)
	local r = os.execute(cmd)
	return r == true or r == 0
end

function M.windows.local_app_data()
	return M.required_env("LOCALAPPDATA", 2)
end

function M.windows.exists(path)
	local cmd = "powershell -NoProfile -Command \"if (Test-Path -LiteralPath "
		.. M.windows.quote(path) .. ") { exit 0 } else { exit 1 }\""
	return M.windows.run(cmd)
end

function M.windows.require_repository_root(script_path)
	local missing = {}
	for _, name in ipairs(M.ITEMS) do
		if not M.windows.exists(".\\" .. name) then
			missing[#missing + 1] = name
		end
	end
	report_missing(script_path, missing)
end

function M.windows.mkdir_p(path)
	local cmd = "powershell -NoProfile -Command \"New-Item -ItemType Directory -Force -Path "
		.. M.windows.quote(path) .. " | Out-Null\""
	return M.windows.run(cmd)
end

function M.windows.copy_dir(src, dst)
	if not M.windows.mkdir_p(dst) then return false end
	local cmd = "powershell -NoProfile -Command \"Copy-Item -Path "
		.. M.windows.quote(src .. "\\*") .. " -Destination " .. M.windows.quote(dst)
		.. " -Recurse -Force\""
	return M.windows.run(cmd)
end

function M.windows.copy_file(src, dst)
	local cmd = "powershell -NoProfile -Command \"Copy-Item -LiteralPath "
		.. M.windows.quote(src) .. " -Destination " .. M.windows.quote(dst) .. " -Force\""
	return M.windows.run(cmd)
end

function M.windows.rm_rf(path)
	local cmd = "powershell -NoProfile -Command \"Remove-Item -LiteralPath "
		.. M.windows.quote(path) .. " -Recurse -Force -ErrorAction SilentlyContinue\""
	return M.windows.run(cmd)
end

return M
