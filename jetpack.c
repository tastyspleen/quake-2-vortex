#include "g_local.h"

#define JETPACK_DRAIN	1 //every x frames drain JETPACK_AMMO
#define JETPACK_AMMO	4

void ApplyThrust(edict_t* ent)
{
	int talentLevel, cost = JETPACK_AMMO;
	vec3_t forward, right;
	vec3_t pack_pos, jet_vector;

	assert(ent != NULL);

	if (ent->myskills.abilities[JETPACK].disable)
		return;

	//Talent: Flight
	if ((talentLevel = getTalentLevel(ent, TALENT_FLIGHT)) > 0)
	{
		int num;

		num = 0.4 * talentLevel;
		if (num < 1)
			num = 1;
		cost -= num;
	}

	//4.0 better jetpack check.
	if (!G_CanUseAbilities(ent, ent->myskills.abilities[JETPACK].current_level, cost))
		return;
	//can't use abilities (spawning sentry gun/drone/etc...)
	if (ent->holdtime > level.time)
		return;
	//4.07 can't use jetpack while being hurt
	if (ent->lasthurt + DAMAGE_ESCAPE_DELAY > level.time)
		return;
	//amnesia disables jetpack
	if (que_findtype(ent->curses, NULL, AMNESIA) != NULL)
		return;

	if (HasFlag(ent))
	{
		gi.cprintf(ent, PRINT_HIGH, "Can't use this ability while carrying the flag!\n");
		return;
	}

	if (ent->client->snipertime >= level.time)
	{
		gi.cprintf(ent, PRINT_HIGH, "You can't use jetpack while trying to snipe!\n");
		return;
	}

	if (ent->client->pers.inventory[power_cube_index] >= cost)
	{
		ent->client->thrustdrain++;
		if (ent->client->thrustdrain == JETPACK_DRAIN)
		{
			ent->client->pers.inventory[power_cube_index] -= cost;
			ent->client->thrustdrain = 0;
		}
	}
	else
	{
		ent->client->thrusting = 0;
		return;
	}

	if (ent->velocity[2] < 350)
	{
		if (ent->groundentity)
			ent->velocity[2] = 150;
		ent->velocity[2] += 150;
	}

	AngleVectors(ent->client->v_angle, forward, right, NULL);
	VectorScale(forward, -7, pack_pos);
	VectorAdd(pack_pos, ent->s.origin, pack_pos);
	pack_pos[2] += (ent->viewheight);

	VectorScale(forward, -50, jet_vector);

	if (ent->client->next_thrust_sound < level.time)
	{
		if (ent->client)
		{
			gi.sound(ent, CHAN_BODY, gi.soundindex("weapons/rockfly.wav"), 1, ATTN_NORM, 0);
			ent->client->next_thrust_sound = level.time + 1.0f;
		}
	}

	ent->lastsound = level.framenum;
}
