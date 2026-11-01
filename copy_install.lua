-- Draw Logo --
io.stdout:write("\27[0;34;1m")
print("  ▇▇▇▇▇▇▇▇▇▇╗▇▇▇▇▇▇▇▇▇▇▇▇╗▇▇▇▇▇▇▇▇▇▇╗    ▇▇▇▇▇▇▇▇╗");
print("▇▇▇▇▇▇▇▇▇▇▇▇║▇▇▇▇▇▇▇▇▇▇▇▇║▇▇▇▇▇▇▇▇▇▇▇▇╗▇▇▇▇▇▇▇▇▇▇▇▇╗");
print("▇▇▇▇╔═══════╝▇▇▇▇╔═══════╝▇▇▇▇╔═══▇▇▇▇║▇▇▇▇╔═══▇▇▇▇║");
print("▇▇▇▇║        ▇▇▇▇║        ▇▇▇▇║   ▇▇▇▇║▇▇▇▇║   ▇▇▇▇║");
print("▇▇▇▇║        ▇▇▇▇▇▇▇▇▇▇▇▇╗▇▇▇▇▇▇▇▇▇▇▇▇║▇▇▇▇║   ▇▇▇▇║");
print("▇▇▇▇║        ▇▇▇▇▇▇▇▇▇▇▇▇║▇▇▇▇▇▇▇▇▇▇╔═╝▇▇▇▇║   ▇▇▇▇║");
print("▇▇▇▇║        ▇▇▇▇╔═══════╝▇▇▇▇╔═════╝  ▇▇▇▇║   ▇▇▇▇║");
print("▇▇▇▇║        ▇▇▇▇║        ▇▇▇▇║        ▇▇▇▇║   ▇▇▇▇║");
print("▇▇▇▇▇▇▇▇▇▇▇▇╗▇▇▇▇▇▇▇▇▇▇▇▇╗▇▇▇▇║        ▇▇▇▇▇▇▇▇▇▇▇▇║");
print(" ╚▇▇▇▇▇▇▇▇▇▇║▇▇▇▇▇▇▇▇▇▇▇▇║▇▇▇▇║         ╚▇▇▇▇▇▇▇▇╔═╝");
print("  ╚═════════╝╚═══════════╝╚═══╝          ╚═══════╝");
io.stdout:write("\27[0m")
io.stdout:flush()

----------------------------------------------------------------------
-- Shared helpers
----------------------------------------------------------------------
local function ask(prompt, default_yes)
	io.write(prompt)
	io.flush()
	local answer = (io.read("*l") or ""):match("^%s*(.-)%s*$"):lower()
	if answer == "" then return default_yes end
	return answer == "y" or answer == "yes"
end

local function detect_platform()
	if package.config:sub(1, 1) == "\\" then return "windows" end
	return "unix"
end

-- Items installed by this script, relative to the install dir.
local ITEMS = { "include", "src", "lib.cmake", "CMakeLists.txt" }

----------------------------------------------------------------------
-- Unix implementation
----------------------------------------------------------------------
local function shq(s)
	return "'" .. s:gsub("'", "'\\''") .. "'"
end

local function unix_exists(path)
	return os.execute("test -e " .. shq(path)) == true
end

local function unix_run_or_die(cmd, what)
	local ok, why, code = os.execute(cmd)
	if ok then return end
	local reason = (why == "signal")
		and ("killed by signal " .. tostring(code))
		or  ("exited with code " .. tostring(code))
	io.stderr:write(("Failed to %s (%s)\n"):format(what, reason))
	os.exit(2)
end

local function unix_main()
	local home = os.getenv("HOME")
	if not (home and home ~= "") then
		io.stderr:write("$HOME is not set\n")
		os.exit(2)
	end

	local default_install_dir = home .. "/.local/share/cepo"

	-- Ask for the install path: default, or a user-supplied absolute path.
	local install_dir
	print("Default installation path: " .. default_install_dir)
	if ask("Use this path? [Y/n] ", true) then
		install_dir = default_install_dir
	else
		io.write("Enter installation path: ")
		io.flush()
		local input = (io.read("*l") or ""):match("^%s*(.-)%s*$")
		if input == "" or input:sub(1, 1) ~= "/" or input:match("^~") then
			io.stderr:write("Invalid path: " .. input .. "\n")
			os.exit(1)
		end
		install_dir = input
		local ok = os.execute("mkdir -p " .. shq(install_dir))
		if not ok then
			io.stderr:write("Invalid or unwritable path: " .. install_dir .. "\n")
			os.exit(1)
		end
	end

	-- Detect name collisions before touching anything.
	local existing = {}
	for _, name in ipairs(ITEMS) do
		if unix_exists(install_dir .. "/" .. name) then
			table.insert(existing, name)
		end
	end

	if #existing > 0 then
		print("The following items already exist in " .. install_dir .. ":")
		for _, name in ipairs(existing) do
			print("  " .. name)
		end
		print("Installing will DELETE these items first.")
		if not ask("Delete them and continue? [y/N] ", false) then
			print("Installation cancelled.")
			os.exit(1)
		end
		for _, name in ipairs(existing) do
			unix_run_or_die("rm -rf " .. shq(install_dir .. "/" .. name),
				"remove " .. install_dir .. "/" .. name)
			print("  deleted " .. install_dir .. "/" .. name)
		end
	end

	unix_run_or_die("mkdir -p " .. shq(install_dir),
		"create " .. install_dir)

	local function install_dir_item(name)
		local dst = install_dir .. "/" .. name
		unix_run_or_die(
			"mkdir -p " .. shq(dst) ..
			" && cp -r " .. shq("./" .. name) .. "/. " .. shq(dst),
			"install directory " .. name)
		print("  -> " .. dst)
	end

	local function install_file_item(name)
		local dst = install_dir .. "/" .. name
		unix_run_or_die("cp " .. shq("./" .. name) .. " " .. shq(dst),
			"install file " .. name)
		print("  -> " .. dst)
	end

	install_dir_item("include")
	install_dir_item("src")
	install_file_item("lib.cmake")
	install_file_item("CMakeLists.txt")

	print("")
	print("Installation complete.")
	print("This script does not modify any shell config or environment variable.")
	print("Set CEPO_CMAKE manually when you need it, e.g.:")
	print("  export CEPO_CMAKE='" .. install_dir .. "/lib.cmake'")
end

----------------------------------------------------------------------
-- Windows implementation
----------------------------------------------------------------------
local function win_quote(s)
	return "'" .. s:gsub("'", "''") .. "'"
end

local function win_exists(path)
	local cmd = "powershell -NoProfile -Command \"if (Test-Path -LiteralPath "
		.. win_quote(path) .. ") { exit 0 } else { exit 1 }\""
	return os.execute(cmd) == true
end

local function win_mkdir_p(path)
	local cmd = "powershell -NoProfile -Command \"New-Item -ItemType Directory -Force -Path "
		.. win_quote(path) .. " | Out-Null\""
	return os.execute(cmd) == true
end

local function win_copy_dir(src, dst)
	win_mkdir_p(dst)
	local cmd = "powershell -NoProfile -Command \"Copy-Item -Path "
		.. win_quote(src .. "\\*") .. " -Destination " .. win_quote(dst)
		.. " -Recurse -Force\""
	return os.execute(cmd) == true
end

local function win_copy_file(src, dst)
	local cmd = "powershell -NoProfile -Command \"Copy-Item -LiteralPath "
		.. win_quote(src) .. " -Destination " .. win_quote(dst) .. " -Force\""
	return os.execute(cmd) == true
end

local function win_rm_rf(path)
	local cmd = "powershell -NoProfile -Command \"Remove-Item -LiteralPath "
		.. win_quote(path) .. " -Recurse -Force -ErrorAction SilentlyContinue\""
	os.execute(cmd)
end

local function windows_main()
	local localappdata = os.getenv("LOCALAPPDATA")
	if not (localappdata and localappdata ~= "") then
		io.stderr:write("%LOCALAPPDATA% is not set\n")
		os.exit(2)
	end

	local default_install_dir = localappdata .. "\\cepo"

	-- Ask for the install path: default, or a user-supplied absolute path.
	local install_dir
	print("Default installation path: " .. default_install_dir)
	if ask("Use this path? [Y/n] ", true) then
		install_dir = default_install_dir
	else
		io.write("Enter installation path: ")
		io.flush()
		local input = (io.read("*l") or ""):match("^%s*(.-)%s*$")
		if input == "" or not input:match("^%a:\\") then
			io.stderr:write("Invalid path: " .. input .. "\n")
			os.exit(1)
		end
		install_dir = input
		if not win_mkdir_p(install_dir) then
			io.stderr:write("Invalid or unwritable path: " .. install_dir .. "\n")
			os.exit(1)
		end
	end

	-- Detect name collisions before touching anything.
	local existing = {}
	for _, name in ipairs(ITEMS) do
		if win_exists(install_dir .. "\\" .. name) then
			table.insert(existing, name)
		end
	end

	if #existing > 0 then
		print("The following items already exist in " .. install_dir .. ":")
		for _, name in ipairs(existing) do
			print("  " .. name)
		end
		print("Installing will DELETE these items first.")
		if not ask("Delete them and continue? [y/N] ", false) then
			print("Installation cancelled.")
			os.exit(1)
		end
		for _, name in ipairs(existing) do
			win_rm_rf(install_dir .. "\\" .. name)
			print("  deleted " .. install_dir .. "\\" .. name)
		end
	end

	if not win_mkdir_p(install_dir) then
		io.stderr:write("Failed to create " .. install_dir .. "\n")
		os.exit(2)
	end

	local function install_dir_item(name)
		local dst = install_dir .. "\\" .. name
		if not win_copy_dir(".\\" .. name, dst) then
			io.stderr:write("Failed to install directory " .. name .. "\n")
			os.exit(2)
		end
		print("  -> " .. dst)
	end

	local function install_file_item(name)
		local dst = install_dir .. "\\" .. name
		if not win_copy_file(".\\" .. name, dst) then
			io.stderr:write("Failed to install file " .. name .. "\n")
			os.exit(2)
		end
		print("  -> " .. dst)
	end

	install_dir_item("include")
	install_dir_item("src")
	install_file_item("lib.cmake")
	install_file_item("CMakeLists.txt")

	print("")
	print("Installation complete.")
	print("This script does not modify any registry entry or environment variable.")
	print("Set CEPO_CMAKE manually when you need it, e.g. in PowerShell:")
	print("  [Environment]::SetEnvironmentVariable(\"CEPO_CMAKE\", \""
		.. install_dir .. "\\lib.cmake\", \"User\")")
end

----------------------------------------------------------------------
local function main()
	local platform = detect_platform()
	if platform == "windows" then
		windows_main()
	else
		unix_main()
	end
end

main()
