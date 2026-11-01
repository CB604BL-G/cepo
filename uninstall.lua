----------------------------------------------------------------------
-- Shared helpers
----------------------------------------------------------------------
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

-- Return the command that removes CEPO_CMAKE from the current shell
-- session, based on $SHELL. Note that `exec $SHELL` is NOT enough here:
-- the new shell inherits the stale variable from this process, and the
-- config block that used to override it has already been removed.
local function current_shell_unset_command()
	local shell_path = os.getenv("SHELL") or ""
	local shell = shell_path:match("([^/]+)$") or ""
	if shell == "fish" then
		return "set -e CEPO_CMAKE"
	end
	return "unset CEPO_CMAKE"
end

----------------------------------------------------------------------
-- Unix implementation
----------------------------------------------------------------------
local function shq(s)
	return "'" .. s:gsub("'", "'\\''") .. "'"
end

local function unix_main()
	local home = os.getenv("HOME")
	if not (home and home ~= "") then
		io.stderr:write("$HOME is not set\n")
		os.exit(2)
	end

	local xdg = os.getenv("XDG_DATA_HOME")
	local data_home = (xdg and xdg ~= "") and xdg or (home .. "/.local/share")
	local manifest_dir = data_home .. "/cepo"
	local manifest_path = manifest_dir .. "/manifest"

	local content = read_file(manifest_path)
	if not content then
		print("No CEPO installation found at " .. manifest_path)
		os.exit(0)
	end

	print("Uninstalling CEPO from " .. manifest_dir)

	local files, changed_file = {}, nil
	for line in (content .. "\n"):gmatch("([^\n]*)\n") do
		if line:match("^file=") then
			table.insert(files, line:sub(#"file=" + 1))
		elseif line:match("^changed_file=") then
			changed_file = line:sub(#"changed_file=" + 1)
		end
	end

	-- 1. Revert the startup script first.
	if changed_file then
		local rc = read_file(changed_file)
		if rc then
			if write_file_atomic(changed_file, strip_marker_block(rc)) then
				print("  removed env block from " .. changed_file)
			else
				io.stderr:write("Failed to update " .. changed_file .. "\n")
				os.exit(2)
			end
		else
			print("  skipped missing " .. changed_file)
		end
	end

	-- 2. Remove installed files and directories.
	for _, f in ipairs(files) do
		os.execute("rm -rf " .. shq(f))
		print("  removed " .. f)
	end

	-- 3. Remove the manifest last.
	os.remove(manifest_path)
	print("  removed " .. manifest_path)

	-- 4. Remove the install dir if it is now empty.
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

	-- 5. Remove the manifest dir if it is now empty.
	os.execute("rmdir " .. shq(manifest_dir) .. " 2>/dev/null")

	print("")
	print("Uninstallation complete.")
	print("The current shell still holds CEPO_CMAKE. To clear it in this session:")
	print("  " .. current_shell_unset_command())
	print("Or close this terminal and open a new one.")
end

----------------------------------------------------------------------
-- Windows implementation
----------------------------------------------------------------------
local function win_quote(s)
	return "'" .. s:gsub("'", "''") .. "'"
end

local function win_set_env(name, value)
	local v = (value == nil) and "$null" or win_quote(value)
	local cmd = "powershell -NoProfile -Command \"[Environment]::SetEnvironmentVariable("
		.. win_quote(name) .. "," .. v .. ",'User')\""
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

local function windows_main()
	local localappdata = os.getenv("LOCALAPPDATA")
	if not (localappdata and localappdata ~= "") then
		io.stderr:write("%LOCALAPPDATA% is not set\n")
		os.exit(2)
	end

	local manifest_dir = localappdata .. "\\cepo"
	local manifest_path = manifest_dir .. "\\manifest"

	local content = read_file(manifest_path)
	if not content then
		print("No CEPO installation found at " .. manifest_path)
		os.exit(0)
	end

	print("Uninstalling CEPO from " .. manifest_dir)

	local files, cepo_cmake = parse_win_manifest(content)

	-- 1. Remove the user-scope environment variable.
	if win_set_env("CEPO_CMAKE", nil) then
		print("  removed HKCU\\Environment CEPO_CMAKE")
	end

	-- 2. Remove installed files and directories.
	for _, f in ipairs(files) do
		win_rm_rf(f)
		print("  removed " .. f)
	end

	-- 3. Remove the manifest last.
	os.remove(manifest_path)
	print("  removed " .. manifest_path)

	-- 4. Remove the install dir if it is now empty.
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

	-- 5. Remove the manifest dir if it is now empty.
	win_rmdir_if_empty(manifest_dir)

	print("")
	print("Uninstallation complete.")
	print("Open a new terminal (or restart your IDE) to refresh the environment.")
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
