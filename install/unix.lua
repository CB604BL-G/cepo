-- AI-generated
-- CEPO installer, Unix flavour.
--
--   lua install/unix.lua
--
-- Copies include/, src/, lib.cmake, CMakeLists.txt and LICENSE into the target
-- directory, then prints the statements that direct CEPO_CMAKE at the copy.
--
-- Must be started from the repository root.
--
-- Exit codes: 0 success, 1 invalid path or cancelled, 2 $HOME unset, the
-- repository root is incomplete, or a copy failed.

local here = (arg and arg[0] or ""):match("^(.*)[/\\][^/\\]*$") or "."
local deps = dofile(here .. "/dependencies.lua")
local unix = deps.unix

deps.logo()

local home = deps.required_env("HOME", 2)
deps.unix.require_repository_root(arg and arg[0] or "install/unix.lua")

local default_install_dir = home .. "/.local/share/cepo"

-- 1. Ask for the install path: default, or a user-supplied absolute path.
local install_dir
print("Default installation path: " .. default_install_dir)
if deps.ask("Use this path? [Y/n] ", true) then
	install_dir = default_install_dir
else
	io.write("Enter installation path: ")
	io.flush()
	local input = deps.trim(io.read("*l"))
	if input == "" or input:sub(1, 1) ~= "/" then
		io.stderr:write("Invalid path: " .. input .. "\n")
		os.exit(1)
	end
	install_dir = input
	if not unix.mkdir_p(install_dir) then
		io.stderr:write("Invalid or unwritable path: " .. install_dir .. "\n")
		os.exit(1)
	end
end

-- 2. Detect name collisions before touching anything.
local conflicts = deps.existing_items(install_dir, "/", unix.exists)

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
		unix.run_or_die("rm -rf " .. unix.shq(path), "remove " .. path)
		print("  deleted " .. path)
	end
end

unix.run_or_die("mkdir -p " .. unix.shq(install_dir), "create " .. install_dir)

-- 3. Copy the files.
local function install_dir_item(name)
	local dst = install_dir .. "/" .. name
	unix.run_or_die(
		"mkdir -p " .. unix.shq(dst) ..
		" && cp -r " .. unix.shq("./" .. name) .. "/. " .. unix.shq(dst),
		"install directory " .. name)
	print("  -> " .. dst)
end

local function install_file_item(name)
	local dst = install_dir .. "/" .. name
	unix.run_or_die("cp " .. unix.shq("./" .. name) .. " " .. unix.shq(dst),
		"install file " .. name)
	print("  -> " .. dst)
end

install_dir_item("include")
install_dir_item("src")
install_file_item("lib.cmake")
install_file_item("CMakeLists.txt")
install_file_item("LICENSE")

print("")
print("Installation complete.")
print("Set CEPO_CMAKE to the installed lib.cmake:")
print("")
print("  bash / zsh:  export CEPO_CMAKE=" .. unix.shq(install_dir .. "/lib.cmake"))
print("  fish:        set -gx CEPO_CMAKE " .. unix.shq(install_dir .. "/lib.cmake"))
print("")
print("Add the statement to the shell configuration to make it persist.")
print("Uninstall by deleting " .. install_dir .. ".")
