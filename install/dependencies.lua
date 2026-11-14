-- AI-generated
-- CEPO installation scripts -- shared components.
--
-- This file is not meant to be run directly. The four entry points load it
-- with dofile() and use the table it returns:
--
--   install/unix.lua          -i install, -u uninstall
--   install/unix_copy.lua     plain copy
--   install/windows.lua       -i install, -u uninstall
--   install/windows_copy.lua  plain copy
--
-- Sections:
--   * general    platform detection, prompts, file I/O, manifest parsing
--   * unix       quoting, shell detection, env block editing, removal
--   * windows    PowerShell based equivalents of the unix section
--
-- The scripts are expected to be started from the repository root, e.g.
--   lua install/unix.lua -i
-- so that "./include", "./src", "./lib.cmake" and "./CMakeLists.txt" exist.

local M = {}

M.MARKER_START = "#[env-for-cepo-start]"
M.MARKER_END   = "#[env-for-cepo-end]"

-- Items copied by every install flavour, relative to the repository root.
M.ITEMS = { "include", "src", "lib.cmake", "CMakeLists.txt" }

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

function M.detect_platform()
	if package.config:sub(1, 1) == "\\" then return "windows" end
	return "unix"
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

function M.read_file(path)
	local f = io.open(path, "r")
	if not f then return nil end
	local c = f:read("*a")
	f:close()
	return c
end

function M.write_file_atomic(path, content)
	local tmp = path .. ".tmp." .. tostring(os.time())
	local f = io.open(tmp, "w")
	if not f then return false end
	f:write(content)
	f:close()
	local ok = os.rename(tmp, path)
	if not ok then os.remove(tmp) end
	return ok
end

function M.ensure_trailing_newline(s)
	if s == "" then return s end
	if s:sub(-1) ~= "\n" then return s .. "\n" end
	return s
end

-- Removes every #[env-for-cepo-start] ... #[env-for-cepo-end] block.
-- Returns the new content, or nil plus a reason when a start marker has no
-- matching end marker: in that case the file must not be rewritten, because
-- everything after the dangling marker would be silently dropped.
function M.strip_marker_block(content)
	local lines = {}
	for line in ((content or "") .. "\n"):gmatch("([^\n]*)\n") do
		lines[#lines + 1] = line
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
			out[#out + 1] = line
		end
	end

	if skip then
		return nil, "the '" .. M.MARKER_START .. "' marker has no matching '"
			.. M.MARKER_END .. "'; refusing to rewrite the file"
	end
	return table.concat(out, "\n")
end

-- Manifest format, one entry per line:
--   file=<installed path>           (one line per installed item)
--   changed_file=<startup script>   (unix only)
--   CEPO_CMAKE=<path>               (windows only)
function M.parse_manifest(content)
	local manifest = { files = {}, changed_file = nil, environment = nil }
	for line in ((content or "") .. "\n"):gmatch("([^\n]*)\n") do
		if line:match("^file=") then
			manifest.files[#manifest.files + 1] = line:sub(#"file=" + 1)
		elseif line:match("^changed_file=") then
			manifest.changed_file = line:sub(#"changed_file=" + 1)
		elseif line:match("^CEPO_CMAKE=") then
			manifest.environment = line:sub(#"CEPO_CMAKE=" + 1)
		end
	end
	return manifest
end

-- Items of M.ITEMS that already exist inside `dir`, excluding the ones listed
-- in `owned` (a set of absolute paths, used for items the installation being
-- replaced already owns). `sep` is the platform separator, `exists` the
-- platform existence check. Returns the absolute paths of the conflicts.
function M.existing_items(dir, sep, exists, owned)
	local found = {}
	for _, name in ipairs(M.ITEMS) do
		local target = dir .. sep .. name
		if not (owned and owned[target]) and exists(target) then
			found[#found + 1] = target
		end
	end
	return found
end

function M.required_env(name, exit_code)
	local value = os.getenv(name)
	if not (value and value ~= "") then
		io.stderr:write("$" .. name .. " is not set\n")
		os.exit(exit_code)
	end
	return value
end

-- Aborts with a readable message when the script is not started from a
-- checkout that actually contains the items to install.
local function report_missing(script_path, missing)
	if #missing == 0 then return end

	io.stderr:write("Missing " .. table.concat(missing, ", ")
		.. " in the current directory.\n")
	io.stderr:write("Run the script from the repository root, e.g.\n")
	io.stderr:write("  lua " .. (script_path or "install/unix.lua -i") .. "\n")
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

function M.unix.require_repository_root(script_path)
	local missing = {}
	for _, name in ipairs(M.ITEMS) do
		if not M.unix.exists("./" .. name) then
			missing[#missing + 1] = name
		end
	end
	report_missing(script_path, missing)
end

function M.unix.mkdir_p(path)
	return M.unix.run("mkdir -p " .. M.unix.shq(path))
end

function M.unix.data_home(home)
	local xdg = os.getenv("XDG_DATA_HOME")
	if xdg and xdg ~= "" then return xdg end
	return home .. "/.local/share"
end

function M.unix.manifest_path(home)
	local dir = M.unix.data_home(home) .. "/cepo"
	return dir, dir .. "/manifest"
end

-- Returns shell name and startup script path, or nil plus the raw $SHELL
-- value when the shell is not supported.
function M.unix.shell_info(home)
	local shell_path = os.getenv("SHELL") or ""
	local shell = shell_path:match("([^/]+)$") or ""
	if shell ~= "bash" and shell ~= "zsh" and shell ~= "fish" then
		return nil, shell_path
	end
	if shell == "fish" then
		return shell, home .. "/.config/fish/config.fish"
	end
	return shell, home .. "/" .. (shell == "bash" and ".bashrc" or ".zshrc")
end

function M.unix.env_block(shell, cepo_cmake)
	local statement
	if shell == "fish" then
		statement = "set -gx CEPO_CMAKE " .. M.unix.shq(cepo_cmake)
	else
		statement = "export CEPO_CMAKE=" .. M.unix.shq(cepo_cmake)
	end
	return M.MARKER_START .. "\n" .. statement .. "\n" .. M.MARKER_END
end

-- Command that clears CEPO_CMAKE in the current shell session. Note that
-- `exec $SHELL` is NOT enough: the new shell inherits the stale variable from
-- this process, and the config block that used to override it is gone.
function M.unix.unset_command()
	local shell = (os.getenv("SHELL") or ""):match("([^/]+)$") or ""
	if shell == "fish" then return "set -e CEPO_CMAKE" end
	return "unset CEPO_CMAKE"
end

-- Replaces any existing marker block in rc_path with `block`.
-- Returns true, or nil plus a reason.
function M.unix.write_env_block(rc_path, block)
	-- The startup script may live in a directory that does not exist yet
	-- (a fresh ~/.config/fish without fish ever started, for instance).
	local dir = rc_path:match("^(.*)/[^/]+$")
	if dir and dir ~= "" and not M.unix.mkdir_p(dir) then
		return nil, "could not create " .. dir
	end

	local content = M.read_file(rc_path) or ""
	local stripped, err = M.strip_marker_block(content)
	if not stripped then return nil, err end
	stripped = M.ensure_trailing_newline(stripped)
	if not M.write_file_atomic(rc_path, stripped .. block .. "\n") then
		return nil, "could not write " .. rc_path
	end
	return true
end

function M.unix.install_dir_from_files(files)
	for _, f in ipairs(files) do
		if f:match("/lib%.cmake$") then
			return (f:gsub("/lib%.cmake$", ""))
		end
	end
	return nil
end

-- Reverts the startup script, deletes the installed files, then deletes the
-- manifest and any directory left empty. Returns a list of problems instead of
-- aborting, so that one failure does not leave the rest behind.
-- `report` is an optional function(line) used for progress output.
function M.unix.remove_installation(manifest_path, manifest, report)
	local say = report or function() end
	local problems = {}

	if manifest.changed_file then
		local rc = M.read_file(manifest.changed_file)
		if rc then
			local stripped, err = M.strip_marker_block(rc)
			if not stripped then
				problems[#problems + 1] = manifest.changed_file .. ": " .. err
			elseif not M.write_file_atomic(manifest.changed_file, stripped) then
				problems[#problems + 1] = "could not update " .. manifest.changed_file
			else
				say("removed env block from " .. manifest.changed_file)
			end
		else
			say("skipped missing " .. manifest.changed_file)
		end
	end

	-- Stop here when the startup script could not be reverted: deleting the
	-- files while the CEPO_CMAKE block survives would leave the user with a
	-- variable pointing at nothing and no manifest to retry from.
	if #problems > 0 then
		problems[#problems + 1] =
			"the installed files were left in place, fix the startup script and run the uninstaller again"
		return problems
	end

	for _, f in ipairs(manifest.files) do
		if M.unix.run("rm -rf " .. M.unix.shq(f)) then
			say("removed " .. f)
		else
			problems[#problems + 1] = "could not remove " .. f
		end
	end

	if M.read_file(manifest_path) then
		if os.remove(manifest_path) then
			say("removed " .. manifest_path)
		else
			problems[#problems + 1] = "could not remove " .. manifest_path
		end
	end

	local install_dir = M.unix.install_dir_from_files(manifest.files)
	if install_dir then
		M.unix.run("rmdir " .. M.unix.shq(install_dir) .. " 2>/dev/null")
	end

	local manifest_dir = manifest_path:match("^(.*)/[^/]+$")
	if manifest_dir and manifest_dir ~= "" then
		M.unix.run("rmdir " .. M.unix.shq(manifest_dir) .. " 2>/dev/null")
	end

	return problems
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

function M.windows.manifest_path(localappdata)
	local dir = localappdata .. "\\cepo"
	return dir, dir .. "\\manifest"
end

function M.windows.get_env(name)
	local p = io.popen("powershell -NoProfile -Command \"[Environment]::GetEnvironmentVariable("
		.. M.windows.quote(name) .. ",'User')\" 2>nul")
	if not p then return nil end
	local v = p:read("*a") or ""
	p:close()
	v = M.trim(v)
	if v == "" then return nil end
	return v
end

function M.windows.set_env(name, value)
	local v = (value == nil) and "$null" or M.windows.quote(value)
	local cmd = "powershell -NoProfile -Command \"[Environment]::SetEnvironmentVariable("
		.. M.windows.quote(name) .. "," .. v .. ",'User')\""
	return M.windows.run(cmd)
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

function M.windows.rmdir_if_empty(path)
	local cmd = "powershell -NoProfile -Command \""
		.. "if (Test-Path -LiteralPath " .. M.windows.quote(path) .. ") { "
		.. "if ((Get-ChildItem -LiteralPath " .. M.windows.quote(path)
		.. " -Force | Measure-Object).Count -eq 0) { "
		.. "Remove-Item -LiteralPath " .. M.windows.quote(path) .. " -Force } }\""
	M.windows.run(cmd)
end

function M.windows.install_dir_from_manifest(manifest)
	if manifest.environment then
		local dir = manifest.environment:gsub("\\lib%.cmake$", "")
		if dir ~= manifest.environment then return dir end
	end
	for _, f in ipairs(manifest.files) do
		if f:match("\\lib%.cmake$") then
			return (f:gsub("\\lib%.cmake$", ""))
		end
	end
	return nil
end

-- Windows counterpart of M.unix.remove_installation.
function M.windows.remove_installation(manifest_path, manifest, report)
	local say = report or function() end
	local problems = {}

	local install_dir = M.windows.install_dir_from_manifest(manifest)

	if M.windows.set_env("CEPO_CMAKE", nil) then
		say("removed HKCU\\Environment CEPO_CMAKE")
	else
		problems[#problems + 1] = "could not remove the user environment variable CEPO_CMAKE"
	end

	for _, f in ipairs(manifest.files) do
		if M.windows.rm_rf(f) then
			say("removed " .. f)
		else
			problems[#problems + 1] = "could not remove " .. f
		end
	end

	if M.read_file(manifest_path) then
		if os.remove(manifest_path) then
			say("removed " .. manifest_path)
		else
			problems[#problems + 1] = "could not remove " .. manifest_path
		end
	end

	if install_dir then
		M.windows.rmdir_if_empty(install_dir)
	end

	local manifest_dir = manifest_path:match("^(.*)\\[^\\]+$")
	if manifest_dir and manifest_dir ~= "" then
		M.windows.rmdir_if_empty(manifest_dir)
	end

	return problems
end

return M
