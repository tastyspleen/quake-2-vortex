q2print("Lua: Assigning nolag cvar.\n")

is_pvm = ((cvar_get("pvm", "0") ~= "0") or (cvar_get("invasion", "0") ~= "0"))
is_invasion = (cvar_get("invasion", "0") ~= "0")

UsePathfinding = 0

if is_pvm then
    cvar_set("nolag", "1")
	q2dofile("variables_pvm")
	if is_invasion then
		q2print("INFO: Using grid pathfinding.\n")
		UsePathfinding = 1
	end
else
	cvar_set("nolag", "0")
	q2dofile("variables_pvp")
end

reloadvars()

--[[
function string:split(sep)
        local sep, fields = sep or ":", {}
        local pattern = string.format("([^%s]+)", sep)
        self:gsub(pattern, function(c) fields[#fields+1] = c end)
        return fields
end

function parse_maplist(name)
	local ret_table = {}

	for line in io.lines(name) do
			local qsplit = line:split(",");
			print(qsplit[0])
			table.insert(ret_table, qsplit[0])
	end
	return ret_table;
end

parse_maplist("vortex/settings/maplist_pvp.txt")
]]

