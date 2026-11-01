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

local function read_file(path)
	local f = io.open(path, "r")
	if not f then return nil end
	local c = f:read("*a")
	f:close()
	return c
end

local function write_file_atomic(path, content)
	local tmp = path .. ".tmp." .. tostring(os.time())
	local f = io.open(tmp, "w")
	if not f then return false end
	f:write(content)
	f:close()
	local ok = os.rename(tmp, path)
	if not ok then os.remove(tmp) end
	return ok
end

-- Remove the env-for-cepo marker block from the given content.
local function strip_marker_block(content)
	local lines = {}
	for line in (content .. "\n"):gmatch("([^\n]*)\n") do
		table.insert(lines, line)
	end
	if #lines > 0 and lines[#lines] == "" then table.remove(lines) end

	local out = {}
	local skip = false
	for _, line in ipairs(lines) do
		if line:match("^#%[env%-for%-cepo%-start%]") then
			skip = true
		elseif line:match("^#%[env%-for%-cepo%-end%]") then
			skip = false
		elseif not skip then
			table.insert(out, line)
		end
	end
	return table.concat(out, "\n")
end

local function ensure_trailing_newline(s)
	if s == "" then return s end
	if s:sub(-1) ~= "\n" then return s .. "\n" end
	return s
end

----------------------------------------------------------------------
-- Unix implementation
----------------------------------------------------------------------
local function shq(s)
	return "'" .. s:gsub("'", "'\\''") .. "'"
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

local function unix_write_env_block(rc_path, block)
	local content = read_file(rc_path) or ""
	content = strip_marker_block(content)
	content = ensure_trailing_newline(content)
	content = content .. block .. "\n"
	if not write_file_atomic(rc_path, content) then
		io.stderr:write("Failed to write " .. rc_path .. "\n")
		os.exit(2)
	end
end

-- Parse a Unix manifest, revert the startup script, and remove installed files.
-- Used both by uninstall and by reinstall (to clean up before writing again).
local function unix_uninstall_from_manifest(manifest_path, content)
	local files, changed_file = {}, nil
	local seen_changed = false
	for line in (content .. "\n"):gmatch("([^\n]*)\n") do
		if line:match("^file=") then
			table.insert(files, line:sub(#"file=" + 1))
		elseif line:match("^changed_file=") then
			changed_file = line:sub(#"changed_file=" + 1)
			seen_changed = true
		end
	end

	-- Revert the startup script.
	if changed_file then
		local rc = read_file(changed_file)
		if rc then
			write_file_atomic(changed_file, strip_marker_block(rc))
		end
	end

	-- Remove installed files and directories.
	for _, f in ipairs(files) do
		os.execute("rm -rf " .. shq(f))
	end

	-- Remove the manifest last.
	os.remove(manifest_path)

	-- Remove the install dir if it is now empty.
	local install_dir = nil
	for _, f in ipairs(files) do
		if f:match("/lib%.cmake$") then
			install_dir = f:gsub("/lib%.cmake$", "")
			break
		end
	end
	if install_dir then
		os.execute("rmdir " .. shq(install_dir) .. " 2>/dev/null")
	end

	-- Remove the manifest dir if it is now empty.
	local manifest_dir = manifest_path:match("^(.*)/[^/]+$")
	if manifest_dir and manifest_dir ~= "" then
		os.execute("rmdir " .. shq(manifest_dir) .. " 2>/dev/null")
	end
end

local function unix_main()
	local home = os.getenv("HOME")
	if not (home and home ~= "") then
		io.stderr:write("$HOME is not set\n")
		os.exit(127)
	end

	-- Only bash, zsh and fish are supported.
	local shell_path = os.getenv("SHELL") or ""
	local shell = shell_path:match("([^/]+)$") or ""
	if shell ~= "bash" and shell ~= "zsh" and shell ~= "fish" then
		io.stderr:write("Unsupported shell: " .. shell_path .. "\n")
		io.stderr:write("Only bash, zsh, fish are supported.\n")
		os.exit(127)
	end

	local xdg = os.getenv("XDG_DATA_HOME")
	local data_home = (xdg and xdg ~= "") and xdg or (home .. "/.local/share")
	local manifest_dir = data_home .. "/cepo"
	local manifest_path = manifest_dir .. "/manifest"

	local default_install_dir = home .. "/.local/share/cepo"

	-- Decide whether this is a fresh install, a reinstall, or a conflicting install.
	local existing_manifest = read_file(manifest_path)
	if existing_manifest then
		print("Existing installation found: " .. manifest_dir)
		print("Reinstalling will remove the old installation first.")
		if not ask("Reinstall? [y/N] ", false) then
			print("Installation cancelled.")
			os.exit(3)
		end
		unix_uninstall_from_manifest(manifest_path, existing_manifest)
		print("Removed old installation.")
	else
		local existing_var = os.getenv("CEPO_CMAKE")
		if existing_var and existing_var ~= "" then
			print("CEPO_CMAKE is already set to: " .. existing_var)
			print("No CEPO installation manifest was found.")
			print("Please remove the CEPO_CMAKE environment variable manually and try again.")
			os.exit(4)
		end
	end

	-- Ask for the install path.
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

	unix_run_or_die("mkdir -p " .. shq(install_dir),
		"create " .. install_dir)

	local installed = {}

	local function install_dir_item(name)
		local dst = install_dir .. "/" .. name
		unix_run_or_die(
			"mkdir -p " .. shq(dst) ..
			" && cp -r " .. shq("./" .. name) .. "/. " .. shq(dst),
			"install directory " .. name)
		table.insert(installed, dst)
		print("  -> " .. dst)
	end

	local function install_file_item(name)
		local dst = install_dir .. "/" .. name
		unix_run_or_die("cp " .. shq("./" .. name) .. " " .. shq(dst),
			"install file " .. name)
		table.insert(installed, dst)
		print("  -> " .. dst)
	end

	install_dir_item("include")
	install_dir_item("src")
	install_file_item("lib.cmake")
	install_file_item("CMakeLists.txt")

	-- Write the marker block into the startup script.
	local cepo_cmake = install_dir .. "/lib.cmake"
	local rc_path
	local block
	if shell == "fish" then
		local fish_dir = home .. "/.config/fish"
		os.execute("mkdir -p " .. shq(fish_dir))
		rc_path = fish_dir .. "/config.fish"
		block = "#[env-for-cepo-start]\n"
			.. "set -gx CEPO_CMAKE " .. shq(cepo_cmake) .. "\n"
			.. "#[env-for-cepo-end]"
	else
		rc_path = (shell == "bash") and (home .. "/.bashrc")
			or  (home .. "/.zshrc")
		block = "#[env-for-cepo-start]\n"
			.. "export CEPO_CMAKE=" .. shq(cepo_cmake) .. "\n"
			.. "#[env-for-cepo-end]"
	end

	unix_write_env_block(rc_path, block)
	print("  -> " .. rc_path)

	-- Write the manifest.
	os.execute("mkdir -p " .. shq(manifest_dir))
	local lines = {}
	for _, f in ipairs(installed) do
		table.insert(lines, "file=" .. f)
	end
	table.insert(lines, "changed_file=" .. rc_path)
	table.insert(lines, block)
	local manifest_out = table.concat(lines, "\n") .. "\n"

	if not write_file_atomic(manifest_path, manifest_out) then
		io.stderr:write("Failed to write manifest: " .. manifest_path .. "\n")
		os.exit(2)
	end
	os.execute("chmod 700 " .. shq(manifest_dir))
	os.execute("chmod 600 " .. shq(manifest_path))

	print("")
	print("Installation complete.")
	print("To load CEPO_CMAKE in the current shell, open a new terminal")
	print("or run:")
	print("  exec $SHELL")
end

----------------------------------------------------------------------
-- Windows implementation
----------------------------------------------------------------------
local function win_quote(s)
	return "'" .. s:gsub("'", "''") .. "'"
end

local function win_get_env(name)
	local p = io.popen("powershell -NoProfile -Command \"[Environment]::GetEnvironmentVariable("
		.. win_quote(name) .. ",'User')\" 2>nul")
	if not p then return nil end
	local v = p:read("*a") or ""
	p:close()
	v = v:gsub("^%s+", ""):gsub("%s+$", "")
	if v == "" then return nil end
	return v
end

local function win_set_env(name, value)
	local v = (value == nil) and "$null" or win_quote(value)
	local cmd = "powershell -NoProfile -Command \"[Environment]::SetEnvironmentVariable("
		.. win_quote(name) .. "," .. v .. ",'User')\""
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

local function win_rmdir_if_empty(path)
	local cmd = "powershell -NoProfile -Command \""
		.. "if (Test-Path -LiteralPath " .. win_quote(path) .. ") { "
		.. "if ((Get-ChildItem -LiteralPath " .. win_quote(path)
		.. " -Force | Measure-Object).Count -eq 0) { "
		.. "Remove-Item -LiteralPath " .. win_quote(path) .. " -Force } }\""
	os.execute(cmd)
end

local function parse_win_manifest(content)
	local files, cepo_cmake = {}, nil
	for line in (content .. "\n"):gmatch("([^\n]*)\n") do
		if line:match("^file=") then
			table.insert(files, line:sub(#"file=" + 1))
		elseif line:match("^CEPO_CMAKE=") then
			cepo_cmake = line:sub(#"CEPO_CMAKE=" + 1)
		end
	end
	return files, cepo_cmake
end

-- Parse a Windows manifest, remove the env var, and delete installed files.
local function windows_uninstall_from_manifest(manifest_path, content)
	local files, cepo_cmake = parse_win_manifest(content)

	win_set_env("CEPO_CMAKE", nil)

	for _, f in ipairs(files) do
		win_rm_rf(f)
	end

	os.remove(manifest_path)

	-- Derive the install dir from the env var or from the file list.
	local install_dir = nil
	if cepo_cmake then
		install_dir = cepo_cmake:gsub("\\lib%.cmake$", "")
	end
	if not install_dir then
		for _, f in ipairs(files) do
			if f:match("\\lib%.cmake$") then
				install_dir = f:gsub("\\lib%.cmake$", "")
				break
			end
		end
	end
	if install_dir then
		win_rmdir_if_empty(install_dir)
	end

	local manifest_dir = manifest_path:match("^(.*)\\[^\\]+$")
	if manifest_dir then
		win_rmdir_if_empty(manifest_dir)
	end
end

local function windows_main()
	local localappdata = os.getenv("LOCALAPPDATA")
	if not (localappdata and localappdata ~= "") then
		io.stderr:write("%LOCALAPPDATA% is not set\n")
		os.exit(2)
	end

	local manifest_dir = localappdata .. "\\cepo"
	local manifest_path = manifest_dir .. "\\manifest"
	local default_install_dir = localappdata .. "\\cepo"

	-- Decide whether this is a fresh install, a reinstall, or a conflicting install.
	local existing_manifest = read_file(manifest_path)
	if existing_manifest then
		print("Existing installation found: " .. manifest_dir)
		print("Reinstalling will remove the old installation first.")
		if not ask("Reinstall? [y/N] ", false) then
			print("Installation cancelled.")
			os.exit(3)
		end
		windows_uninstall_from_manifest(manifest_path, existing_manifest)
		print("Removed old installation.")
	else
		local existing_var = win_get_env("CEPO_CMAKE")
		if existing_var then
			print("CEPO_CMAKE is already set to: " .. existing_var)
			print("No CEPO installation manifest was found.")
			if not ask("Overwrite this environment variable? [y/N] ", false) then
				print("Installation cancelled.")
				os.exit(3)
			end
			win_set_env("CEPO_CMAKE", nil)
		end
	end

	-- Ask for the install path.
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

	if not win_mkdir_p(install_dir) then
		io.stderr:write("Failed to create " .. install_dir .. "\n")
		os.exit(2)
	end

	local installed = {}

	local function install_dir_item(name)
		local dst = install_dir .. "\\" .. name
		if not win_copy_dir(".\\" .. name, dst) then
			io.stderr:write("Failed to install directory " .. name .. "\n")
			os.exit(2)
		end
		table.insert(installed, dst)
		print("  -> " .. dst)
	end

	local function install_file_item(name)
		local dst = install_dir .. "\\" .. name
		if not win_copy_file(".\\" .. name, dst) then
			io.stderr:write("Failed to install file " .. name .. "\n")
			os.exit(2)
		end
		table.insert(installed, dst)
		print("  -> " .. dst)
	end

	install_dir_item("include")
	install_dir_item("src")
	install_file_item("lib.cmake")
	install_file_item("CMakeLists.txt")

	-- Write the user-scope environment variable.
	local cepo_cmake = install_dir .. "\\lib.cmake"
	if not win_set_env("CEPO_CMAKE", cepo_cmake) then
		io.stderr:write("Failed to set CEPO_CMAKE environment variable\n")
		os.exit(2)
	end
	print("  -> HKCU\\Environment CEPO_CMAKE=" .. cepo_cmake)

	-- Write the manifest.
	if not win_mkdir_p(manifest_dir) then
		io.stderr:write("Failed to create manifest directory: " .. manifest_dir .. "\n")
		os.exit(2)
	end

	local lines = {}
	for _, f in ipairs(installed) do
		table.insert(lines, "file=" .. f)
	end
	table.insert(lines, "CEPO_CMAKE=" .. cepo_cmake)
	local manifest_out = table.concat(lines, "\n") .. "\n"

	if not write_file_atomic(manifest_path, manifest_out) then
		io.stderr:write("Failed to write manifest: " .. manifest_path .. "\n")
		os.exit(2)
	end

	print("")
	print("Installation complete.")
	print("Open a new terminal (or restart your IDE) to load CEPO_CMAKE.")
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
