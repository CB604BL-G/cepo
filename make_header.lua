local PREFIX = "CEPO_"

local function fail(msg)
	io.stderr:write("error: " .. msg .. "\n")
	os.exit(1)
end

-- POSIX shell single-quote escaping: close quote, escaped quote, reopen
-- (so the whole string survives being wrapped in another pair of single quotes)
local function shell_quote(s)
	return "'" .. s:gsub("'", "'\\''") .. "'"
end

local function mkdirp(path)
	local r = os.execute("mkdir -p " .. shell_quote(path))
	-- os.execute returns true/nil on 5.2+, a number on 5.1; handle both
	if r == nil or r == false or (type(r) == "number" and r ~= 0) then
		fail("failed to create directory: " .. path)
	end
end

local argv = arg
if #argv < 1 then
	fail("usage: lua make_header.lua path/to/header.hpp")
end

local input = argv[1]

-- Normalize separators and strip leading/trailing slashes
local rel = input:gsub("\\", "/")
rel = rel:gsub("^/+", ""):gsub("/+$", "")

if rel == "" then
	fail("invalid path: " .. input)
end
if rel:find("%s") then
	fail("path must not contain whitespace: " .. input)
end
-- Plain find (3rd arg true): literal "..", not a Lua pattern
if rel:find("%.%.", 1, true) then
	fail("path must not contain '..': " .. input)
end
-- Allow callers to pass either "foo.hpp" or "include/cepo/foo.hpp"
if rel:sub(1, #"include/cepo/") == "include/cepo/" then
	rel = rel:sub(#"include/cepo/" + 1)
end

local output = "include/cepo/" .. rel

-- Header guard: uppercase, non-alphanumeric -> underscore
local upper = rel:upper():gsub("[^A-Z0-9]", "_")
local guard = PREFIX .. upper

local content = table.concat({
	"//Copyright (c) 2026 CB604BL",
	"#ifndef " .. guard,
	"#define " .. guard,
	"",
	"#endif //" .. guard,
	"",
}, "\n")

-- Greedy match: dir is everything before the last '/'
local dir = output:match("^(.*)/[^/]*$")
if dir then
	mkdirp(dir)
end

-- Block scope so `existing` doesn't leak past the existence check
do
	local existing = io.open(output, "r")
	if existing then
		existing:close()
		fail("file already exists: " .. output)
	end
end

local f, err = io.open(output, "w")

if not f then
	fail("cannot write " .. output .. ": " .. tostring(err))
else
	f:write(content)
	f:close()
end

print("created " .. output)
