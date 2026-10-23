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

local home_path = os.getenv("HOME")
if not (home_path and home_path ~= "") then
	print("$HOME is not set")
	os.exit(127)
end

local cepo_library_dir = home_path .. "/.local/share/cepo"

local function shq(s)
	return "'" .. s:gsub("'", "'\\''") .. "'"
end

local function run_or_die(cmd, what)
	local ok, why, code = os.execute(cmd)
	if ok then return end
	local reason = (why == "signal")
		and ("killed by signal " .. tostring(code))
		or  ("exited with code " .. tostring(code))
	io.stderr:write(("Failed to %s (%s)\n"):format(what, reason))
	os.exit(code or 1)
end

local function dir_exists(path)
	return os.execute("test -d " .. shq(path)) == true
end

local function ask(prompt, default_yes)
	io.write(prompt)
	io.flush()
	local answer = (io.read("*l") or ""):match("^%s*(.-)%s*$"):lower()
	if answer == "" then return default_yes end
	return answer == "y" or answer == "yes"
end

-- Confirmation step: distinguish between "new installation" and "reinstallation" --
if dir_exists(cepo_library_dir) then
	-- Prompt for reinstallation risk when it exists, default is N --
	print("Existing installation found: " .. cepo_library_dir)
	print("Reinstalling will DELETE the folder and copy fresh files.")
	if not ask("Reinstall? [y/N] ", false) then
		print("Installation cancelled.")
		os.exit(1)
	end
	run_or_die("rm -rf " .. shq(cepo_library_dir),
		"remove old installation at " .. cepo_library_dir)
	print("Removed old installation.")
else
	print("The library will be installed in " .. cepo_library_dir .. " next.")
	if not ask("Do you want to continue?[Y/n] ", true) then
		print("Installation cancelled.")
		os.exit(1)
	end
end

-- install --
run_or_die(
	"mkdir -p " .. shq(cepo_library_dir),
	"create " .. cepo_library_dir)

local function install_dir(name)
	local dst = cepo_library_dir .. "/" .. name
	run_or_die(
		"mkdir -p " .. shq(dst) ..
		" && cp -r " .. shq("./" .. name) .. "/. " .. shq(dst),
		"install directory " .. name)
	print("  -> " .. dst)
end

local function install_file(name)
	local dst = cepo_library_dir .. "/" .. name
	run_or_die(
		"cp " .. shq("./" .. name) .. " " .. shq(dst),
		"install file " .. name)
	print("  -> " .. dst)
end

install_dir("include")
install_dir("src")
install_file("lib.cmake")
install_file("CMakeLists.txt")

print("")
print("Installation complete.")
