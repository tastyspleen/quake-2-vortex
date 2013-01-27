#include "g_local.h"

void TBI_AwardTeam(int Teamnum, int exp, qboolean Broadcast);
void TBI_CheckSpawns();
void TBI_Reinitialize();

#define TBI_SPAWN_HEALTH 15000
#define TBI_MAX_SPAWNS	 16

typedef struct
{
	int RedSpawns;
	int BlueSpawns;
	int TotalRedSpawns;
	int TotalBlueSpawns;
	int RedPlayers, BluePlayers;
	edict_t *EntRedSpawns[TBI_MAX_SPAWNS];
	edict_t *EntBlueSpawns[TBI_MAX_SPAWNS];
} TBI_Info;

TBI_Info tbi_game;

void TBI_AssignTeam(edict_t* ent)
{
	int TeamNum = RED_TEAM;

	if (!tbi->value)
		return;

	if (tbi_game.RedPlayers > tbi_game.BluePlayers)
		TeamNum = BLUE_TEAM;

	switch (TeamNum)
	{
	case RED_TEAM:
		tbi_game.RedPlayers++;
	default:
		tbi_game.BluePlayers++;
	}

	ent->teamnum = TeamNum;

	V_AssignClassSkin(ent, Info_ValueForKey(ent->client->pers.userinfo, "skin"));
}

void TBI_CheckRules(edict_t* self)
{
	TBI_CheckSpawns();

	if (!tbi_game.BlueSpawns || !tbi_game.RedSpawns)
	{
		gi.bprintf(PRINT_HIGH, "Team %s is out of spawns!\n", !tbi_game.RedSpawns ? "Red" : "Blue");
		TBI_Reinitialize();
		gi.sound(&g_edicts[0], CHAN_VOICE, gi.soundindex("misc/tele_up.wav"), 1, ATTN_NONE, 0);
		TBI_AwardTeam(self->teamnum == RED_TEAM? BLUE_TEAM : RED_TEAM, 350 * pow(AveragePlayerLevel(), 2) / 4, true);
	}
}

void TBI_SpawnPlayers()
{
	int CurrentRedSpawn, CurrentBlueSpawn;
	int i_maxclients = maxclients->value;
	edict_t *cl_ent;

	CurrentRedSpawn = CurrentBlueSpawn = 0;

	OrganizeTeams(true);
	
	// Assign everyone a spawn
	for (cl_ent = g_edicts + 1; cl_ent != g_edicts + i_maxclients + 1; cl_ent++)
	{
		if (cl_ent->client && cl_ent->inuse && !G_IsSpectator(cl_ent)) // we never know
		{
			if (cl_ent->teamnum == RED_TEAM) // Red team
			{
				if (tbi_game.EntRedSpawns[CurrentRedSpawn] != NULL) // we got a valid spawn point
				{
					cl_ent->spawn = tbi_game.EntRedSpawns[CurrentRedSpawn];
					CurrentRedSpawn++;

					if (CurrentRedSpawn == tbi_game.TotalRedSpawns) // we're out of spawns then huh
						CurrentRedSpawn = 0;
				}
			}else // Blue team
			{
				if (tbi_game.EntBlueSpawns[CurrentBlueSpawn] != NULL)
				{
					cl_ent->spawn = tbi_game.EntBlueSpawns[CurrentBlueSpawn];
					CurrentBlueSpawn++;

					if (CurrentBlueSpawn == tbi_game.TotalBlueSpawns)
						CurrentBlueSpawn = 0;
				}
			}
		}
	}

	for (cl_ent = g_edicts + 1; cl_ent != g_edicts + i_maxclients + 1; cl_ent++)
	{
		vec3_t start;
		// Alrighty, everyone's got spawns.
		if (cl_ent->client && cl_ent->inuse && !G_IsSpectator(cl_ent))
		{
			VectorCopy(cl_ent->spawn->s.origin, start);
			start[2] += 9;
			VectorCopy(start, cl_ent->s.origin);
			VectorCopy(cl_ent->spawn->s.angles, cl_ent->s.angles);
			cl_ent->client->invincible_framenum = level.framenum + 30;
			gi.linkentity(cl_ent);
		}
	}
}

edict_t *TBI_FindRandomSpawnForTeam(int team)
{
	if (team == RED_TEAM)
	{
		return tbi_game.EntRedSpawns[GetRandom(0, tbi_game.TotalRedSpawns-1)];
	}
	else
		return tbi_game.EntBlueSpawns[GetRandom(0, tbi_game.TotalBlueSpawns-1)];
}

edict_t* TBI_FindSpawn(edict_t *ent)
{
	edict_t *potential_spot;
	int iter = 0;

	if (!ent)
		return NULL;

	if (!ent->client || !ent->inuse)
		return NULL;

	if (G_IsSpectator(ent))
	{
		return SelectDeathmatchSpawnPoint(ent);
	}

	if (!ent->teamnum)
	{
		TBI_AssignTeam(ent);
	}

	if (ent->spawn)
	{
		edict_t* spawn = ent->spawn;
		ent->spawn = NULL;
		return spawn;
	}
	
	potential_spot = TBI_FindRandomSpawnForTeam(ent->teamnum);

	while (potential_spot->deadflag == DEAD_DEAD && iter != 256)
	{
		iter++; // don't let it drift off for too long
		potential_spot = TBI_FindRandomSpawnForTeam(ent->teamnum);
	}
	
	if (potential_spot->deadflag == DEAD_DEAD) // so didn't find an alive spot?
		return SelectDeathmatchSpawnPoint(ent) ; // find a random one

	return potential_spot;
}

void TBI_Reinitialize()
{
	int i;

	tbi_game.RedSpawns = tbi_game.BlueSpawns = 0;

	for (i = 0; i < tbi_game.TotalRedSpawns; i++)
	{
		edict_t *e = tbi_game.EntRedSpawns[i];
		e->health = e->max_health;
		e->deadflag = DEAD_NO;
		e->solid = SOLID_BBOX;
		e->takedamage = DAMAGE_YES;
		e->svflags &= ~SVF_NOCLIENT;
		tbi_game.RedSpawns++;
	}

	for (i = 0; i < tbi_game.TotalBlueSpawns; i++)
	{
		edict_t *e = tbi_game.EntBlueSpawns[i];
		e->health = e->max_health;
		e->deadflag = DEAD_NO;
		e->solid = SOLID_BBOX;
		e->svflags &= ~SVF_NOCLIENT;
		e->takedamage = DAMAGE_YES;
		tbi_game.BlueSpawns++;
	}

	TBI_SpawnPlayers();
}

void TBI_SpawnThink(edict_t *self)
{
}

void TBI_CheckSpawns()
{
	int i;
	tbi_game.RedSpawns = tbi_game.BlueSpawns = 0;

	for (i = 0; i < tbi_game.TotalRedSpawns; i++)
	{
		edict_t *e = tbi_game.EntRedSpawns[i];
		if (e->deadflag != DEAD_DEAD)
			tbi_game.RedSpawns++;
	}

	for (i = 0; i < tbi_game.TotalBlueSpawns; i++)
	{
		edict_t *e = tbi_game.EntBlueSpawns[i];
		if (e->deadflag != DEAD_DEAD)
			tbi_game.BlueSpawns++;
	}
}

void TBI_AwardTeam(int Teamnum, int exp, qboolean Broadcast)
{
	edict_t *cl_ent;
	int i_maxclients = maxclients->value;
	for (cl_ent = g_edicts + 1; cl_ent != g_edicts + i_maxclients + 1; cl_ent++)
	{
		if (!G_IsSpectator(cl_ent) && cl_ent->client)
		{
			if (cl_ent->teamnum == Teamnum)
				V_AddFinalExp(cl_ent, exp);
			cl_ent->myskills.credits += exp;
		}
	}

	if (Broadcast)
		gi.bprintf(PRINT_HIGH, "Awarded team %s a total of %d experience points and credits!\n", Teamnum == RED_TEAM ? "Red" : "Blue", exp);
}

void TBI_SpawnDie(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (self->deadflag != DEAD_DEAD)
	{
		edict_t *cl_ent;
		int i_maxclients = maxclients->value;

		self->svflags |= SVF_NOCLIENT;
		self->solid = SOLID_NOT;
		self->takedamage = DAMAGE_NO;

		gi.bprintf(PRINT_HIGH, "A %s spawn has been destroyed.\n", self->teamnum == RED_TEAM ? "red" : "blue");

		// Give some experience.
		TBI_AwardTeam(self->teamnum == RED_TEAM? BLUE_TEAM : RED_TEAM, pow(self->monsterinfo.level, 2) / 6 * 60, false);	

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (self->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PVS);

		gi.sound(g_edicts, CHAN_VOICE, gi.soundindex(va("bosstank/BTKUNQV%d.wav", GetRandom(1, 2))), 1, ATTN_NONE, 0);
	}

	self->deadflag = DEAD_DEAD;
	TBI_CheckRules(self);
}

void TBI_InitPlayerSpawns(int teamnum)
{
	char *_classname;
	edict_t *e = g_edicts;
	int effects;

	if (teamnum == RED_TEAM)
	{
		_classname = "info_player_team1";
	}
	else if (teamnum == BLUE_TEAM)
	{
		_classname = "info_player_team2";
	}else if (teamnum == 3)
	{
		_classname = "info_player_deathmatch";
	}

	while((e = G_Find(e, FOFS(classname), _classname)) != NULL)
	{
		qboolean nullteam = false;
		e->health = TBI_SPAWN_HEALTH;
		e->max_health = e->health;
		e->takedamage = DAMAGE_YES;
		e->solid = SOLID_BBOX;
		e->deadflag = DEAD_NO;
		e->svflags &= ~SVF_NOCLIENT;
		e->monsterinfo.level = e->myskills.level = AveragePlayerLevel();

		e->s.effects |= EF_COLOR_SHELL;
		e->mtype = TBI_PLAYERSPAWN;
		e->think = TBI_SpawnThink;
		e->die = TBI_SpawnDie;

		if (teamnum == 3)
		{
			nullteam = true;
			if (tbi_game.TotalRedSpawns > tbi_game.TotalBlueSpawns)
				teamnum = BLUE_TEAM;
			else
				teamnum = RED_TEAM;
		}

		if (teamnum == RED_TEAM)
		{
			tbi_game.EntRedSpawns[tbi_game.RedSpawns] = e;
			tbi_game.RedSpawns++;
			tbi_game.TotalRedSpawns++;
			effects = RF_SHELL_RED;
		}
		else
		{
			tbi_game.EntBlueSpawns[tbi_game.BlueSpawns] = e;
			tbi_game.BlueSpawns++;
			tbi_game.TotalBlueSpawns++;
			effects = RF_SHELL_BLUE;
		}
		
		e->teamnum = teamnum;
		e->s.renderfx = effects;

		if (nullteam)
			teamnum = 3;
	}
}

void InitTBI()
{
	memset(&tbi_game, 0, sizeof(TBI_Info));
	TBI_InitPlayerSpawns(RED_TEAM);
	TBI_InitPlayerSpawns(BLUE_TEAM);
	TBI_InitPlayerSpawns(3); // deathmatch stuff.

	TBI_SpawnPlayers();
}