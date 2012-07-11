#include "g_local.h"
#include "invasion.h"

//FIXME: need queue that holds all players that are waiting to respawn but all spawns are busy
edict_t		*INV_SpawnQue[MAX_CLIENTS];
int			invasion_max_playerspawns;
int nolagoldval; 

void INV_Init (void)
{
	if (!pvm->value || !invasion->value)
		return;

	INV_InitSpawnQue();
	INVASION_OTHERSPAWNS_REMOVED = false;
	invasion_difficulty_level = 1;
	invasion_max_playerspawns = 0;
	nolagoldval = nolag->value;
	gi.cvar_set("nolag", "1");
}

// initialize array values to NULL
void INV_InitSpawnQue (void)
{
	int i;

	for (i=0; i < MAX_CLIENTS; i++)
		INV_SpawnQue[i] = NULL;
}

qboolean INV_InSpawnQue (edict_t *ent)
{
	int i;

	for (i=0; i < MAX_CLIENTS; i++)
		if (INV_SpawnQue[i] && INV_SpawnQue[i] == ent)
			return true;
	return false;
}

// add player to the spawn queue
qboolean INV_AddSpawnQue (edict_t *ent)
{
	int i;

	if (!ent || !ent->inuse || !ent->client)
		return false;

	// don't add them if they are already in the queue
	if (INV_InSpawnQue(ent))
		return false;

	for (i=0; i < MAX_CLIENTS; i++)
	{
		if (!INV_SpawnQue[i])
		{
			INV_SpawnQue[i] = ent;
			//gi.dprintf("added %s to list\n", ent->client->pers.netname);
			return true;
		}
	}
	return false;
}

// remove player from the queue
qboolean INV_RemoveSpawnQue (edict_t *ent)
{
	int i;

	for (i=0; i < MAX_CLIENTS; i++)
	{
		if (INV_SpawnQue[i] && (INV_SpawnQue[i] == ent))
		{
			//gi.dprintf("removed %s from list\n", ent->client->pers.netname);
			INV_SpawnQue[i] = NULL;
			return true;
		}
	}
	return false;
}

// return player that is waiting to respawn
edict_t *INV_GetSpawnPlayer (void)
{
	int i;

	for (i=0; i < MAX_CLIENTS; i++)
	{
		if (INV_SpawnQue[i] && INV_SpawnQue[i]->inuse && INV_SpawnQue[i]->client)
		{
			//gi.dprintf("found %s in list\n", INV_SpawnQue[i]->client->pers.netname);
			return INV_SpawnQue[i];
		}
	}
	return NULL;
}

edict_t *INV_GetRandomSpawn (void)
{
	int i=0;
	edict_t *e, *spawns[MAX_CLIENTS];

	for (e=g_edicts ; e < &g_edicts[globals.num_edicts]; e++)
	{
		if (e && e->inuse && (e->mtype == INVASION_PLAYERSPAWN))
			spawns[i++] = e;
	}

	// didn't find any spawns
	if (i == 0)
		return NULL;
	// found one spawn
	else if (i == 1)
		return spawns[0];
	// found more than one spawn, so return a random one
	else
		return spawns[GetRandom(0, i-1)];

}

edict_t *INV_GetMonsterSpawn (edict_t *from)
{
	if (from)
		from++;
	else
		from = g_edicts;

	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (from && from->inuse && (from->mtype == INVASION_MONSTERSPAWN) 
			&& (level.time > from->wait))
			return from;
	}

	return NULL;
}

void INV_AwardPlayers (void)
{
	int		i, points, credits, num_spawns = INV_GetNumPlayerSpawns();
	edict_t *player;

	// we're not in invasion mode
	if (!INVASION_OTHERSPAWNS_REMOVED)
		return;

	// if map didn't end normally, don't award points
	if (level.time < timelimit->value*60)
		return;

	// no award if the humans were unable to defend their spawns
	if (num_spawns < 1)
		return;

	gi.bprintf(PRINT_HIGH, "Humans win! Players were awarded a bonus.\n");

	gi.cvar_set("nolag", nolagoldval? "1" : "0");

	for (i=0; i<game.maxclients; i++) 
	{
		player = g_edicts+1+i;
		if (!player->inuse)
			continue;

		points = player->client->resp.score*((float)num_spawns/invasion_max_playerspawns);
		if (points > INVASION_BONUS_EXP)
			points = INVASION_BONUS_EXP;
		//points = INVASION_BONUS_EXP*((float)num_spawns/invasion_max_playerspawns);
		credits = INVASION_BONUS_CREDITS*((float)num_spawns/invasion_max_playerspawns);

	//	gi.dprintf("points=%d credits=%d spawns=%d max=%d\n", 
	//		points, credits, num_spawns, invasion_max_playerspawns);
	
		if (!G_IsSpectator(player))
		{
			V_AddFinalExp(player, points);
			player->myskills.credits += credits;
		}
	}
}

int fib(int iter)
{
	int a, b, c;
    a = b = c = 1;

	while (iter)
	{
		iter--;

		c = a+b;
		b = a;
		a = c;
	}
	return c;
}

edict_t* INV_SpawnDrone(edict_t* self, edict_t *e, int index)
{
	edict_t *monster;
	vec3_t	start;
	trace_t	tr;
	int mhealth = 1;

	monster = SpawnDrone(self, index, true);
	//monster = SpawnDrone(self, 4, true);
	
	// calculate starting position
	VectorCopy(e->s.origin, start);
	start[2] = e->absmax[2] + 1 + abs(monster->mins[2]);

	tr = gi.trace(start, monster->mins, monster->maxs, start, NULL, MASK_SHOT);

	// don't spawn here if a friendly monster occupies this space
	if (index != 30) // not a boss?
		if ((tr.fraction < 1) && tr.ent && tr.ent->inuse && tr.ent->activator && tr.ent->activator->inuse 
			&& (tr.ent->activator == self) && (tr.ent->deadflag != DEAD_DEAD))
		{
			// remove the monster and try again
			G_FreeEdict(monster);
			//M_Remove(self, false, false);
			return NULL;
		}
		
	e->wait = level.time + 1.0; // time until spawn is available again
	
	monster->monsterinfo.aiflags |= AI_FIND_NAVI; // search for navi
	monster->s.angles[YAW] = e->s.angles[YAW];
	monster->prev_navi = NULL;


	// we modify the monsters' health lightly
	if (invasion_difficulty_level < 7 && invasion_difficulty_level > 5)
		mhealth = (invasion_difficulty_level-5) * 0.1 + 1;
	else if (invasion_difficulty_level >= 7)
		mhealth = 2 + 0.1 * invasion_difficulty_level;

	monster->max_health = monster->health = monster->max_health*mhealth;

		// move the monster onto the spawn pad
	if (index != 30)
	{
		VectorCopy(start, monster->s.origin);
		VectorCopy(start, monster->s.old_origin);
	}
	monster->s.event = EV_OTHER_TELEPORT;

	if (e->count)
		monster->monsterinfo.inv_framenum = level.framenum + e->count;
	else
		monster->monsterinfo.inv_framenum = level.framenum + 60; // give them quad/invuln to prevent spawn-camping
	gi.linkentity(monster);
	return monster;
}

float TimeFormula()
{
	int base = 3.5*60;
	int playeramt = ActivePlayers() * 10;
	int levelamt = invasion_difficulty_level * 10;
	int cap = 60;
	int rval = base - playeramt - levelamt;

	if (rval < cap)
		rval = cap;

	return rval;
}

int printedmessage = 0;
int mspawned = 0;
float limitframe = 0;
edict_t *boss = NULL;

void BossCheck(edict_t *e, edict_t *self)
{
	if (!(invasion_difficulty_level % 5))// every 5 levels, spawn a boss
	{
		int bcount = 0;
		int iter = 0;
		while (((e = INV_GetMonsterSpawn(e)) != NULL) && (bcount < 1))
		{
			if (!boss)
			{
				if(! (boss = INV_SpawnDrone(self, e, 30)) )
				{
					iter++;
					
					if (iter < 256) // 256 tries before quitting with spawning this boss
						continue;
					else
						break;
				}
				bcount ++;
				total_monsters++;
				mspawned++;
				G_PrintGreenText(va("A level %d tank commander has spawned!", boss->monsterinfo.level));
				break;
			}
		}
	}else
		boss = NULL;
}

void INV_SpawnMonsters (edict_t *self)
{
	int		players, max_monsters;
	int		total_monsters = PVM_TotalMonsters(self);
	edict_t *e=NULL;

	players = max_monsters = total_players();

	// there are still monsters alive
	if ((total_monsters > 0) && (self->count == MONSTERSPAWN_STATUS_IDLE))
	{
		// if there's nobody playing, remove all monsters
		if (players < 1)
		{
			PVM_RemoveAllMonsters(self);
			return; // don't spawn any monsters.
		}
		if (level.intermissiontime)
			return;
		if (limitframe > level.time) // we still got time?
		{
			self->nextthink = level.time + FRAMETIME;
			return;
		}else
		{
			gi.bprintf(PRINT_HIGH, "Time's up!\n");
			if (boss && boss->deadflag != DEAD_DEAD) // out of time for the boss.
			{	
				G_PrintGreenText(va("You failed on eliminating the commander soon enough!\n"));
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_BOSSTPORT);
				gi.WritePosition (boss->s.origin);
				gi.multicast (boss->s.origin, MULTICAST_PVS);
				gi.unlinkentity(boss);
				G_FreeEdict(boss);
				boss = NULL;
			}
			// increase the difficulty level for the next wave
			invasion_difficulty_level += 1; 
			printedmessage = 0;
			mspawned = total_monsters;
			gi.sound(&g_edicts[0], CHAN_VOICE, gi.soundindex("misc/tele_up.wav"), 1, ATTN_NONE, 0);
		}
	}

	self->count = MONSTERSPAWN_STATUS_WORKING;

	if (players < 1)
	{
		// if there's nobody playing, then wait until some join
		self->nextthink = level.time + FRAMETIME;
		return;
	}

	switch (invasion_difficulty_level)
	{
	case 1: max_monsters = 15; break;
	case 2: max_monsters = 20; break;
	case 3: max_monsters = 25; break;
	case 4: max_monsters = 30; break;
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
		max_monsters = 40; break;

	default: max_monsters = 45;
	}

	if (!(invasion_difficulty_level % 5))
	{
		max_monsters = 4*(ActivePlayers()-1);
	}

	if (!printedmessage)
	{
		limitframe = level.time + TimeFormula();
		if (invasion_difficulty_level == 1)
				gi.bprintf(PRINT_HIGH, "The invasion begins!\n");
		if (invasion_difficulty_level % 5)
			gi.bprintf(PRINT_HIGH, "Welcome to level %d. %d monsters incoming!\n", invasion_difficulty_level, max_monsters);
		else
			gi.bprintf(PRINT_HIGH, "Welcome to level %d.\n", invasion_difficulty_level, max_monsters);
		G_PrintGreenText(va("Timelimit: %dm %ds.\n", (int)TimeFormula() / 60, (int)TimeFormula() % 60));

		gi.sound(&g_edicts[0], CHAN_VOICE, gi.soundindex("misc/talk1.wav"), 1, ATTN_NONE, 0);

		BossCheck(e, self);

		printedmessage = 1;
	}

	if (max_monsters > 50) // cap the amount of monsters
		max_monsters = 50;

	// the dm_monsters cvar is the minimum of monsters that will spawn
	if (max_monsters < dm_monsters->value)
		max_monsters = dm_monsters->value;
	
	while ( ((e = INV_GetMonsterSpawn(e)) != NULL) && (total_monsters < max_monsters || mspawned == max_monsters) )
	{
		int randomval = GetRandom(1, 9);

		if (invasion_difficulty_level % 5) // nonboss stage?
			while ( (randomval == 5) ) // disallow medics
			{
				randomval = GetRandom(1, 8);
			}

		if (!INV_SpawnDrone(self, e, randomval))
			continue;

		total_monsters++;
		mspawned++;
		//gi.dprintf("World has %d/%d level %d monsters.\n", total_monsters, max_monsters, monster->monsterinfo.level);
	}

	if (total_monsters == max_monsters || mspawned >= max_monsters)
	{
		// increase the difficulty level for the next wave
		invasion_difficulty_level += 1/* + (invasion_difficulty_level / 10)*/;
		printedmessage = 0;
		mspawned = 0;
		self->count = MONSTERSPAWN_STATUS_IDLE;
	}

	self->nextthink = level.time + FRAMETIME;
}

void INV_SpawnPlayers (void)
{
	edict_t *e, *cl_ent;
	vec3_t	start;
	trace_t	tr;

	// we shouldn't be here if this isn't invasion mode
	if (!INVASION_OTHERSPAWNS_REMOVED)
		return;

	// if there are no players waiting to be spawned, then we're done
	if (!(cl_ent = INV_GetSpawnPlayer()))
		return;

	for (e=g_edicts ; e < &g_edicts[globals.num_edicts]; e++)
	{
		// find an available spawn point
		if (e && e->inuse && (e->mtype == INVASION_PLAYERSPAWN) && (level.time > e->wait))
		{
			// get player starting position
			VectorCopy(e->s.origin, start);
			start[2] = e->absmax[2] + 1 + abs(cl_ent->mins[2]);

			tr = gi.trace(start, cl_ent->mins, cl_ent->maxs, start, NULL, MASK_SHOT);

			// don't spawn if another player is standing in the way
			if ((tr.fraction < 1) && tr.ent && tr.ent->inuse && tr.ent->client)
			{
				e->wait = level.time + 1.0;
				continue;
			}

			e->wait = level.time + 2.0; // delay before other players can use this spawn
			cl_ent->spawn = e; // player may use this spawn
			respawn(cl_ent);

			// get another waiting player
			if (!(cl_ent = INV_GetSpawnPlayer()))
				return;
		}
	}
}

edict_t *INV_SelectPlayerSpawnPoint (edict_t *ent)
{
	if (!ent || !ent->inuse)
		return NULL;

	// spectators always get a spawn
	if (G_IsSpectator(ent))
		return INV_GetRandomSpawn();

	if (ent->spawn && ent->spawn->inuse)
		return ent->spawn;

	return NULL;
}

int INV_GetNumPlayerSpawns (void)
{
	int		i=0;
	edict_t *e;

	for (e=g_edicts ; e < &g_edicts[globals.num_edicts]; e++)
	{
		if (e && e->inuse && (e->mtype == INVASION_PLAYERSPAWN))
			i++;
	}
	return i;
}

void info_player_invasion_death (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	//FIXME: when all of these die (number of player spawns = 0) the map should end
	//FIXME: a message saying monsters won should print, and players don't get any bonus points

	G_UseTargets(self, self);
	self->think = BecomeExplosion1;
	self->nextthink = level.time + FRAMETIME;
}

void info_player_invasion_think (edict_t *self)
{
	if (level.time > self->lasthurt + 1.0)
		M_Regenerate(self, PLAYERSPAWN_REGEN_FRAMES, PLAYERSPAWN_REGEN_DELAY,  1.0, true, false, false, &self->monsterinfo.regen_delay1);

	self->nextthink = level.time + FRAMETIME;
}

void SP_info_player_invasion (edict_t *self)
{
	//FIXME: change to invasion->value
	if (!pvm->value || !invasion->value)
	{
		G_FreeEdict (self);
		return;
	}

	// remove deathmatch spawnpoints
	if (!INVASION_OTHERSPAWNS_REMOVED)
	{
		edict_t *e=NULL;
		
		gi.dprintf("PvM Invasion mode activated!\n");

		while((e = G_Find(e, FOFS(classname), "info_player_deathmatch")) != NULL)
		{
			if (e && e->inuse)
				G_FreeEdict(e);
		}
		INVASION_OTHERSPAWNS_REMOVED = true;
	}

	//FIXME: this entity should be killable and game should end if all of them die
	gi.setmodel (self, "models/objects/dmspot/tris.md2");
	self->s.skinnum = 0;
	self->mtype = INVASION_PLAYERSPAWN;
	self->health = PLAYERSPAWN_HEALTH + 250*AveragePlayerLevel();
	self->max_health = self->health;
	self->takedamage = DAMAGE_YES;
	self->die = info_player_invasion_death;
	self->think = info_player_invasion_think;
	self->nextthink = level.time + FRAMETIME;
	//self->touch = info_player_invasion_touch;
	self->solid = SOLID_BBOX;
	VectorSet (self->mins, -32, -32, -24);
	VectorSet (self->maxs, 32, 32, -16);
	gi.linkentity (self);

	invasion_max_playerspawns++;
}

void SP_info_monster_invasion (edict_t *self)
{
	//FIXME: change to invasion->value
	if (!pvm->value || !invasion->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->mtype = INVASION_MONSTERSPAWN;
	self->solid = SOLID_NOT;
	//self->s.effects |= EF_BLASTER;
	//gi.setmodel (self, "models/items/c_head/tris.md2");
	self->svflags |= SVF_NOCLIENT;
	gi.linkentity (self);
}

void SP_navi_monster_invasion (edict_t *self)
{
	//FIXME: change to invasion->value
	if (!pvm->value || !invasion->value)
	{
		G_FreeEdict (self);
		return;
	}
	
	//gi.dprintf("navi point created!\n");

	self->solid = SOLID_NOT;
	self->mtype = INVASION_NAVI;
	//gi.setmodel (self, "models/items/c_head/tris.md2");
	self->svflags |= SVF_NOCLIENT;
	gi.linkentity (self);
}

int G_GetEntityIndex (edict_t *ent)
{
	int		i = 0;
	edict_t *e;

	if (!ent || !ent->inuse)
		return 0;

	for (e=g_edicts ; e < &g_edicts[globals.num_edicts]; e++)
	{
		if (e && e->inuse && (e == ent))
			return i;
		i++;
	}

	return 0;
}

void inv_defenderspawn_think (edict_t *self)
{
	//int		num=G_GetEntityIndex(self); // FOR DEBUGGING ONLY
	vec3_t	start;
	trace_t	tr;
	edict_t *monster=NULL;

	//FIXME: this isn't a good enough check if monster is dead or not
	// did our monster die?
	if (!G_EntIsAlive(self->enemy))
	{
		if (self->orders == MONSTERSPAWN_STATUS_IDLE)
		{
			// wait some time before spawning another monster
			self->wait = level.time + 30;
			self->orders = MONSTERSPAWN_STATUS_WORKING;
			//gi.dprintf("%d: monster died, waiting to build monster...\n", num);
		}

		// try to spawn another
		if ((level.time > self->wait) 
			&& (monster = SpawnDrone(self, self->sounds, true)) != NULL)
		{
			//gi.dprintf("%d: attempting to spawn a monster\n", num);
			// get starting position
			VectorCopy(self->s.origin, start);
			start[2] = self->absmax[2] + 1 + abs(monster->mins[2]);

			tr = gi.trace(start, monster->mins, monster->maxs, start, NULL, MASK_SHOT);
			
			// kill dead bodies
			if (tr.ent && tr.ent->takedamage && (tr.ent->deadflag == DEAD_DEAD || tr.ent->health < 1))
				T_Damage(tr.ent, self, self, vec3_origin, tr.ent->s.origin, vec3_origin, 10000, 0, 0, 0);
			// spawn is blocked, try again later
			 else if (tr.fraction < 1)
			{
				//gi.dprintf("%d: spawn is blocked, will try again\n", num);
				G_FreeEdict(monster);
				self->nextthink = level.time + 1.0;
				return;
			}

			// should this monster stand ground?
			if (self->style)
				monster->monsterinfo.aiflags |= AI_STAND_GROUND;

			monster->s.angles[YAW] = self->s.angles[YAW];
			// move the monster onto the spawn pad
			VectorCopy(start, monster->s.origin);
			VectorCopy(start, monster->s.old_origin);
			monster->s.event = EV_OTHER_TELEPORT;

			// give them quad/invuln to prevent spawn-camping
			if (self->count)
				monster->monsterinfo.inv_framenum = level.framenum + self->count;
			else
				monster->monsterinfo.inv_framenum = level.framenum + 60; 

			gi.linkentity(monster);

			self->enemy = monster; // keep track of this monster
			//gi.dprintf("%d: spawned a monster successfully\n", num);
		}
		else
		{
			//if (level.time > self->wait)
			//	gi.dprintf("%d: spawndrone() failed to spawn a monster\n", num);
			//else if (!(level.framenum%10))
			//	gi.dprintf("%d: waiting...\n", num);
		}
	}
	else
	{
		//if (self->orders == MONSTERSPAWN_STATUS_WORKING)
		//	gi.dprintf("%d: spawn is now idle\n", num);
		self->orders = MONSTERSPAWN_STATUS_IDLE;
	}

	self->nextthink = level.time + FRAMETIME;
}

// map-editable fields - count (quad+invin), style (stand ground), angle, sounds (mtype to spawn)
void SP_inv_defenderspawn (edict_t *self)
{
	//FIXME: change to invasion->value
	if (!pvm->value || !invasion->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->mtype = INVASION_DEFENDERSPAWN;
	self->solid = SOLID_NOT;
	self->think = inv_defenderspawn_think;
	self->nextthink = level.time + pregame_time->value + FRAMETIME;
	//self->s.effects |= EF_BLASTER;
	//gi.setmodel (self, "models/items/c_head/tris.md2");
	self->svflags |= SVF_NOCLIENT;
	gi.linkentity (self);
}

float GetPlayerBossDamage (edict_t *player, edict_t *boss);
float GetTotalBossDamage (edict_t *self);

void INV_AwardMonsterKill (edict_t *attacker, edict_t *target)
{
	int		i, exp_points, credits, base_credits, max_credits, max_exp, base_exp;
	float	bonus, dmgmod, levelmod, damage, invmod;
	edict_t *player;

	//gi.dprintf("INV_AwardMonsterKill()\n");

	// don't award points for monsters that were just resurrected
	if (target->monsterinfo.resurrected_time > level.time)
		return;

	// player-monsters don't award points
	if (G_GetClient(target))
		return;

	// if a player (or his summon) scores a kill, then increment spree
	if ((player = G_GetClient(attacker)) != NULL)
		player->myskills.streak++;

	for (i=0; i<game.maxclients; i++) 
	{
		player = g_edicts+1+i;

		// award experience and credits to non-spectator clients
		if (!player->inuse || G_IsSpectator(player))
			continue;

		damage = GetPlayerBossDamage(player, target);
		if (damage < 1)
			continue; // they get nothing if they didn't touch the boss

		// calculate level modifier
		levelmod = ((float)target->monsterinfo.level+1) / ((float)attacker->myskills.level+1);

		// calculate damage modifier
		dmgmod = damage / GetTotalBossDamage(target);

		// calculate spree bonus
		bonus = 1.0;
		//bonus = 1 + 0.04 * player->myskills.streak;
		//if (bonus > 2)
		//	bonus = 2;

		invmod = 0.5; // half the exp in invasion.

		base_exp = target->monsterinfo.control_cost*EXP_WORLD_MONSTER*invmod;
		exp_points = (int)ceil(base_exp*levelmod*dmgmod*bonus);
		base_credits = target->monsterinfo.control_cost*CREDITS_OTHER_BASE;
		credits = (int)ceil(base_credits*levelmod*dmgmod*bonus);

		//gi.dprintf("exp=%d base=%d levelmod=%.1f dmgmod=%.1f bonus=%.1f\n", exp_points, base_exp, levelmod, dmgmod, bonus);

		// cap max experience at 300% normal
		max_exp = 3*base_exp;
		if (exp_points > max_exp)
			exp_points = max_exp;

		max_credits = 3*base_credits;
		if (credits > max_credits)
			credits = max_credits;

		// give player credits and experience
		exp_points = V_AddFinalExp(player, exp_points);
		player->myskills.credits += credits;

		//gi.cprintf(player, PRINT_HIGH, "You gained %d experience and %d credits!\n", exp_points, credits);
		gi.cprintf(player, PRINT_HIGH, "You dealt %.0f damage (%.0f%c) to %s (level %d), gaining %d experience and %d credits\n", 
			damage, (dmgmod * 100), '%', V_GetMonsterName(target), target->monsterinfo.level, exp_points, credits);
	}
}
