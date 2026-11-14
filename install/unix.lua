-- AI-generated
-- CEPO installer / uninstaller, Unix flavour.
--
--   lua install/unix.lua -i    install: copy the files, append a marked
--                              CEPO_CMAKE block to the startup script of the
--                              current shell, write a manifest
--   lua install/unix.lua -u    uninstall: revert the startup script, delete
--                              the installed files and the manifest
--
-- Installation must be started from the repository root, so that ./include,
-- ./src, ./lib.cmake and ./CMakeLists.txt are visible. Uninstallation works
-- from anywhere.
--
-- Exit codes:
--   -i: 0 success, 1 invalid path / cancelled / bad arguments, 2 command or
--       file failure, 3 reinstall cancelled, 4 CEPO_CMAKE set without a
--       manifest, 127 $HOME unset or unsupported shell
--   -u: 0 success or nothing to do, 1 bad arguments, 2 $HOME unset, the
--       startup script could not be reverted, or a file could not be removed

local here = (arg and arg[0] or ""):match("^(.*)[/\\][^/\\]*$") or "."
local deps = dofile(here .. "/dependencies.lua")
local unix = deps.unix

----------------------------------------------------------------------
-- install
----------------------------------------------------------------------

local function install()
	deps.logo()

	local home = deps.required_env("HOME", 127)
	deps.unix.require_repository_root(arg and arg[0] or "install/unix.lua")

	local shell, rc_path = unix.shell_info(home)
	if not shell then
		io.stderr:write("Unsupported shell: " .. tostring(rc_path) .. "\n")
		io.stderr:write("Only bash, zsh, fish are supported.\n")
		os.exit(127)
	end

	-- The shell check runs before anything is touched, so an unsupported
	-- shell never leaves a half-installed tree behind.
	local manifest_dir, manifest_path = unix.manifest_path(home)
	local default_install_dir = home .. "/.local/share/cepo"

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
		local existing_var = os.getenv("CEPO_CMAKE")
		if existing_var and existing_var ~= "" then
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
		if input == "" or input:sub(1, 1) ~= "/" then
			io.stderr:write("Invalid path: " .. input .. "\n")
			os.exit(1)
		end
		install_dir = input
	end

	if not unix.mkdir_p(install_dir) then
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

	local conflicts = deps.existing_items(install_dir, "/", unix.exists, owned)
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
		local problems = unix.remove_installation(manifest_path, previous, print)
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
		unix.run_or_die("rm -rf " .. unix.shq(path), "remove " .. path)
		print("  deleted " .. path)
	end

	-- Copy the files.
	local installed = {}

	local function install_dir_item(name)
		local dst = install_dir .. "/" .. name
		unix.run_or_die(
			"mkdir -p " .. unix.shq(dst) ..
			" && cp -r " .. unix.shq("./" .. name) .. "/. " .. unix.shq(dst),
			"install directory " .. name)
		installed[#installed + 1] = dst
		print("  -> " .. dst)
	end

	local function install_file_item(name)
		local dst = install_dir .. "/" .. name
		unix.run_or_die("cp " .. unix.shq("./" .. name) .. " " .. unix.shq(dst),
			"install file " .. name)
		installed[#installed + 1] = dst
		print("  -> " .. dst)
	end

	install_dir_item("include")
	install_dir_item("src")
	install_file_item("lib.cmake")
	install_file_item("CMakeLists.txt")

	-- 6. Write the manifest BEFORE editing the startup script. If the script
	--    edit fails, the installation is still recorded and `-u` can
	--    clean it up; the opposite order would leave files on disk that no
	--    manifest knows about, which makes the next run fail with code 4.
	local cepo_cmake = install_dir .. "/lib.cmake"
	local block = unix.env_block(shell, cepo_cmake)

	unix.run_or_die("mkdir -p " .. unix.shq(manifest_dir), "create " .. manifest_dir)

	local lines = {}
	for _, f in ipairs(installed) do
		lines[#lines + 1] = "file=" .. f
	end
	lines[#lines + 1] = "changed_file=" .. rc_path
	lines[#lines + 1] = block

	if not deps.write_file_atomic(manifest_path, table.concat(lines, "\n") .. "\n") then
		io.stderr:write("Failed to write manifest: " .. manifest_path .. "\n")
		os.exit(2)
	end
	unix.run("chmod 700 " .. unix.shq(manifest_dir))
	unix.run("chmod 600 " .. unix.shq(manifest_path))

	-- 7. Append the marker block to the startup script.
	local ok, err = unix.write_env_block(rc_path, block)
	if not ok then
		io.stderr:write("Failed to update " .. rc_path .. ": " .. err .. "\n")
		io.stderr:write("The installed files and the manifest are already in place.\n")
		io.stderr:write("Roll back with:\n")
		io.stderr:write("  lua install/unix.lua -u\n")
		os.exit(2)
	end
	print("  -> " .. rc_path)

	print("")
	print("Installation complete.")
	print("To load CEPO_CMAKE in the current shell, open a new terminal")
	print("or run:")
	print("  exec $SHELL")
end

----------------------------------------------------------------------
-- uninstall
----------------------------------------------------------------------

local function uninstall()
	local home = deps.required_env("HOME", 2)
	local manifest_dir, manifest_path = unix.manifest_path(home)

	local content = deps.read_file(manifest_path)
	if not content then
		print("No CEPO installation found at " .. manifest_path)
		os.exit(0)
	end

	print("Uninstalling CEPO from " .. manifest_dir)

	local problems = unix.remove_installation(
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
	print("The current shell still holds CEPO_CMAKE. To clear it in this session:")
	print("  " .. unix.unset_command())
	print("Or close this terminal and open a new one.")
end

----------------------------------------------------------------------
-- dispatch
----------------------------------------------------------------------

local function usage(stream, code)
	stream:write("Usage:\n")
	stream:write("  lua install/unix.lua -i    install CEPO\n")
	stream:write("  lua install/unix.lua -u    uninstall CEPO\n")
	stream:write("  lua install/unix.lua -h    show this message\n")
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
