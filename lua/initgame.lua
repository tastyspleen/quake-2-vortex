q2print("Lua: Assigning nolag cvar.\n")

is_pvm = ((cvar_get("pvm", "0") ~= "0") or (cvar_get("invasion", "0") ~= "0"))
is_invasion = (cvar_get("invasion", "0") ~= "0")
alter_multipliers = cvar_get("alter_mults", "1") ~= "0"
UsePathfinding = 0
UseLuaMaplists = 1

if alter_multipliers then
	cvar_set("vrx_over10mult", "0.8")
	cvar_set("vrx_sub10mult", "1.0")
	cvar_set("vrx_pvppointmult", "1")
	cvar_set("vrx_pvpcreditmult", "2")
	cvar_set("vrx_pvmpointmult", "1")
end

if is_pvm then
    cvar_set("nolag", "1")
	q2dofile("variables_pvm")

	if alter_multipliers then
		cvar_set("vrx_over10mult", "0.6")
		cvar_set("vrx_sub10mult", "1.0")
	end

	if is_invasion then
		q2print("INFO: Using grid pathfinding.\n")
		UsePathfinding = 1
	end

else
	cvar_set("nolag", "0")
	q2dofile("variables_pvp")
	if alter_multipliers then
		cvar_set("vrx_pvmpointmult", "1.2") -- ffa case
	end
end

EXP_WORLD_MONSTER = 25
reloadvars()

