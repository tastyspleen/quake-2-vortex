#include "g_local.h"

void freeze_think (edict_t *self)
{
	// must have a valid target
	if (!self->owner || !self->owner->inuse)
	{
		G_FreeEdict(self);
		return;
	}

	// time's up?
	if (level.time > self->delay)
	{	
		// play a sound
		gi.sound (self->owner, CHAN_WEAPON, gi.soundindex("makron/bfg_fire.wav"), 1, ATTN_NORM, 0);
		
		// remove the curse
		que_removeent(self->owner->curses, self, true);
		
		// teleport effect
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_TELEPORT_EFFECT);
		gi.WritePosition (self->owner->s.origin);
		gi.multicast (self->owner->s.origin, MULTICAST_PVS);

		// make them invincible for a short time to clear away campers
		self->owner->client->invincible_framenum = level.framenum + RESPAWN_INVIN_TIME;

		return;
	}

	// follow the target
	VectorCopy(self->owner->s.origin, self->s.origin);
	self->nextthink = level.time + FRAMETIME;
}

void freeze_player (edict_t *ent) 
{
	edict_t *freeze;

	freeze = G_Spawn();
	freeze->movetype = MOVETYPE_NOCLIP;
	freeze->svflags |= SVF_NOCLIENT;
	freeze->solid = SOLID_NOT;
	freeze->owner = ent;
	freeze->nextthink = level.time + FRAMETIME;
	freeze->think = freeze_think;
	freeze->delay = level.time + GetRandom(1, 5);
	freeze->classname = "curse";
	freeze->mtype = CURSE_FROZEN;
	VectorCopy(ent->s.origin, freeze->s.origin);
	if (!que_addent(ent->curses, freeze, freeze->delay))
	{
		G_FreeEdict(freeze);
		return;
	}
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("makron/bfg_fire.wav"), 1, ATTN_NORM, 0);
}

void Cmd_FreezePlayer(edict_t *ent)
{
	edict_t		*other = NULL;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_FreezePlayer()\n", ent->client->pers.netname);
	
	if (ctf->value)
	{
		gi.cprintf(ent, PRINT_HIGH, "Can't use this ability in CTF.\n");
		return;
	}
	//3.0 polts can't cast freeze_spell in human form
	if(ent->myskills.class_num == CLASS_POLTERGEIST && !ent->mtype)
		return;

	if(ent->myskills.abilities[FREEZE_SPELL].disable)
		return;

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[FREEZE_SPELL].current_level, COST_FOR_FREEZER))
		return;

	ent->client->pers.inventory[power_cube_index] -= COST_FOR_FREEZER;
	ent->client->ability_delay = level.time + DELAY_FREEZE;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("spells/coldcast.wav"), 1, ATTN_NORM, 0);
	
	// look for players to freeze
	while ((other = findradius(other, ent->s.origin, RADIUS_FREEZE)) != NULL)
	{
		if (!G_ValidTarget(ent, other, true))
			continue;
		if (!other->client)
			continue;
		if (SPREE_WAR && (SPREE_DUDE == other))
			continue; // can't freeze someone warring
		
		freeze_player(other);
	}
}
