-- AI-generated
-- CEPO installer / uninstaller, Windows flavour.
--
--   lua install\windows.lua -i    install: copy the files, set the user-scope
--                                 CEPO_CMAKE variable, write a manifest
--   lua install\windows.lua -u    uninstall: remove the variable, delete the
--                                 installed files and the manifest
--
-- Installation must be started from the repository root, so that .\include,
-- .\src, .\lib.cmake and .\CMakeLists.txt are visible. Uninstallation works
-- from anywhere.
--
-- Exit codes:
--   -i: 0 success, 1 invalid path / cancelled / bad arguments, 2
--       %LOCALAPPDATA% unset or a copy / file / environment failure, 3
--       reinstall cancelled, 4 CEPO_CMAKE set without a manifest
--   -u: 0 success or nothing to do, 1 bad arguments, 2 %LOCALAPPDATA% unset,
--       the variable could not be removed, or a file could not be deleted

local here = (arg and arg[0] or ""):match("^(.*)[/\\][^/\\]*$") or "."
local deps = dofile(here .. "/dependencies.lua")
local win = deps.windows

----------------------------------------------------------------------
-- install
----------------------------------------------------------------------

local function install()
	deps.logo()

	local localappdata = win.local_app_data()
	win.require_repository_root(arg and arg[0] or "install\\windows.lua")

	local manifest_dir, manifest_path = win.manifest_path(localappdata)
	local default_install_dir = localappdata .. "\\cepo"

	-- 1. Decide between reinstall, fresh install, and conflicting environment.
	local manifest_content = deps.read_file(manifest_path)
	local previous = manifest_content and deps.parse_manifest(manifest_content) or nil

	if previous then
		print("Existing installation found: " .. manifest_dir)
		print("Reinstalling will remove the old installation first.")
		if not deps.ask("Reinstall? [y/N] ", false) then
			print("Installation cancelled.")
			os.exit(3)
		end
	else
		local existing_var = win.get_env("CEPO_CMAKE")
		if existing_var then
			print("CEPO_CMAKE is already set to: " .. existing_var)
			print("No CEPO installation manifest was found.")
			print("Please remove the CEPO_CMAKE environment variable manually and try again.")
			os.exit(4)
		end
	end

	-- 2. Ask for the installation path. This happens BEFORE the old
	--    installation is removed: a typo must not cost a working install.
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
	end

	if not win.mkdir_p(install_dir) then
		io.stderr:write("Invalid or unwritable path: " .. install_dir .. "\n")
		os.exit(1)
	end

	-- 3. Check the target directory. This is not limited to a fresh install:
	--    a reinstall into a different directory would otherwise merge into
	--    whatever is already there. Items owned by the installation being
	--    replaced are not conflicts, the uninstaller below removes them.
	local owned = {}
	if previous then
		for _, f in ipairs(previous.files) do owned[f] = true end
	end

	local conflicts = deps.existing_items(install_dir, "\\", win.exists, owned)
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
	end

	-- 4. Drop the previous installation, now that the new location is usable.
	if previous then
		print("Removing the previous installation:")
		local problems = win.remove_installation(manifest_path, previous, print)
		if #problems > 0 then
			for _, p in ipairs(problems) do
				io.stderr:write("  warning: " .. p .. "\n")
			end
			print("  continuing anyway")
		end
	end

	-- 5. Delete the conflicts, so the copy below cannot leave stale files
	--    behind: headers deleted or renamed in the repository would otherwise
	--    survive in the installation forever.
	for _, path in ipairs(conflicts) do
		if not win.rm_rf(path) then
			io.stderr:write("Failed to remove " .. path .. "\n")
			os.exit(2)
		end
		print("  deleted " .. path)
	end

	-- 4. Copy the files.
	local installed = {}

	local function install_dir_item(name)
		local dst = install_dir .. "\\" .. name
		if not win.copy_dir(".\\" .. name, dst) then
			io.stderr:write("Failed to install directory " .. name .. "\n")
			os.exit(2)
		end
		installed[#installed + 1] = dst
		print("  -> " .. dst)
	end

	local function install_file_item(name)
		local dst = install_dir .. "\\" .. name
		if not win.copy_file(".\\" .. name, dst) then
			io.stderr:write("Failed to install file " .. name .. "\n")
			os.exit(2)
		end
		installed[#installed + 1] = dst
		print("  -> " .. dst)
	end

	install_dir_item("include")
	install_dir_item("src")
	install_file_item("lib.cmake")
	install_file_item("CMakeLists.txt")

	-- 6. Write the manifest BEFORE setting the environment variable, so that a
	--    failure there still leaves something `-u` can clean up.
	local cepo_cmake = install_dir .. "\\lib.cmake"

	if not win.mkdir_p(manifest_dir) then
		io.stderr:write("Failed to create manifest directory: " .. manifest_dir .. "\n")
		os.exit(2)
	end

	local lines = {}
	for _, f in ipairs(installed) do
		lines[#lines + 1] = "file=" .. f
	end
	lines[#lines + 1] = "CEPO_CMAKE=" .. cepo_cmake

	if not deps.write_file_atomic(manifest_path, table.concat(lines, "\n") .. "\n") then
		io.stderr:write("Failed to write manifest: " .. manifest_path .. "\n")
		os.exit(2)
	end

	-- 7. Set the user-scope environment variable.
	if not win.set_env("CEPO_CMAKE", cepo_cmake) then
		io.stderr:write("Failed to set CEPO_CMAKE environment variable\n")
		io.stderr:write("The installed files and the manifest are already in place.\n")
		io.stderr:write("Roll back with:\n")
		io.stderr:write("  lua install\\windows.lua -u\n")
		os.exit(2)
	end
	print("  -> HKCU\\Environment CEPO_CMAKE=" .. cepo_cmake)

	print("")
	print("Installation complete.")
	print("Open a new terminal (or restart your IDE) to load CEPO_CMAKE.")
end

----------------------------------------------------------------------
-- uninstall
----------------------------------------------------------------------

local function uninstall()
	local localappdata = win.local_app_data()
	local manifest_dir, manifest_path = win.manifest_path(localappdata)

	local content = deps.read_file(manifest_path)
	if not content then
		print("No CEPO installation found at " .. manifest_path)
		os.exit(0)
	end

	print("Uninstalling CEPO from " .. manifest_dir)

	local problems = win.remove_installation(
		manifest_path, deps.parse_manifest(content), print)

	if #problems > 0 then
		for _, p in ipairs(problems) do
			io.stderr:write("error: " .. p .. "\n")
		end
		io.stderr:write("Uninstallation incomplete.\n")
		io.stderr:write("Fix the problems above and run this script again with -u.\n")
		os.exit(2)
	end

	print("")
	print("Uninstallation complete.")
	print("Open a new terminal (or restart your IDE) to refresh the environment.")
end

----------------------------------------------------------------------
-- dispatch
----------------------------------------------------------------------

local function usage(stream, code)
	stream:write("Usage:\n")
	stream:write("  lua install\\windows.lua -i    install CEPO\n")
	stream:write("  lua install\\windows.lua -u    uninstall CEPO\n")
	stream:write("  lua install\\windows.lua -h    show this message\n")
	stream:write("Run -i from the repository root.\n")
	os.exit(code)
end

local action = arg and arg[1]
if action == "-i" then
	install()
elseif action == "-u" then
	uninstall()
elseif action == "-h" or action == "--help" then
	usage(io.stdout, 0)
else
	if action then
		io.stderr:write("Unknown argument: " .. tostring(action) .. "\n\n")
	end
	usage(io.stderr, 1)
end
