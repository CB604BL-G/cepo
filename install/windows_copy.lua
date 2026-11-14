-- AI-generated
-- CEPO plain-copy installer, Windows flavour.
--
--   lua install\windows_copy.lua
--
-- Copies include\, src\, lib.cmake and CMakeLists.txt into a target directory.
-- No environment variable is set and no manifest is kept: there is no
-- uninstall script for this method.
--
-- Must be started from the repository root.
--
-- Exit codes: 0 success, 1 invalid path or cancelled, 2 %LOCALAPPDATA% unset
-- or a copy failed.

local here = (arg and arg[0] or ""):match("^(.*)[/\\][^/\\]*$") or "."
local deps = dofile(here .. "/dependencies.lua")
local win = deps.windows

deps.logo()

local localappdata = win.local_app_data()
win.require_repository_root(arg and arg[0] or "install\\windows_copy.lua")

local default_install_dir = localappdata .. "\\cepo"

-- 1. Ask for the install path: default, or a user-supplied absolute path.
local install_dir
print("Default installation path: " .. default_install_dir)
if deps.ask("Use this path? [Y/n] ", true) then
	install_dir = default_install_dir
else
	io.write("Enter installation path: ")
	io.flush()
	local input = deps.trim(io.read("*l"))
	if input == "" or not input:match("^%a:\\") then
		io.stderr:write("Invalid path: " .. input .. "\n")
		os.exit(1)
	end
	install_dir = input
	if not win.mkdir_p(install_dir) then
		io.stderr:write("Invalid or unwritable path: " .. install_dir .. "\n")
		os.exit(1)
	end
end

-- 2. Detect name collisions before touching anything.
local conflicts = deps.existing_items(install_dir, "\\", win.exists)

if #conflicts > 0 then
	print("The following items already exist in " .. install_dir .. ":")
	for _, path in ipairs(conflicts) do
		print("  " .. path)
	end
	print("Installing will DELETE these items first.")
	if not deps.ask("Delete them and continue? [y/N] ", false) then
		print("Installation cancelled.")
		os.exit(1)
	end
	for _, path in ipairs(conflicts) do
		win.rm_rf(path)
		print("  deleted " .. path)
	end
end

if not win.mkdir_p(install_dir) then
	io.stderr:write("Failed to create " .. install_dir .. "\n")
	os.exit(2)
end

-- 3. Copy the files.
local function install_dir_item(name)
	local dst = install_dir .. "\\" .. name
	if not win.copy_dir(".\\" .. name, dst) then
		io.stderr:write("Failed to install directory " .. name .. "\n")
		os.exit(2)
	end
	print("  -> " .. dst)
end

local function install_file_item(name)
	local dst = install_dir .. "\\" .. name
	if not win.copy_file(".\\" .. name, dst) then
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
