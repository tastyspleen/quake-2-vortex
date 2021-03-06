#pragma once

#define PLAYTIME_MIN_MINUTES		999.0	// minutes played before penalty begins
#define PLAYTIME_MAX_MINUTES		999.0	// minutes played before max penalty is reached
#define PLAYTIME_MAX_PENALTY		2.0		// reduce experience in half

#define EXP_SHARED_FACTOR				0.5
#define PLAYER_MONSTER_MIN_PLAYERS		4

void VortexAddExp(edict_t* attacker, edict_t* targ);
void NewLevel_Addons(edict_t* ent);
int VortexAddCredits(edict_t* ent, float level_diff, int bonus, qboolean client);
void VortexAddMonsterExp(edict_t* attacker, edict_t* monster);


