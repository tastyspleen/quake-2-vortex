#include "g_local.h"
#include "boss.h"

#define TANK_FRAMES_START_STAND		0
#define TANK_FRAMES_END_STAND		29
#define TANK_FRAMES_START_WALK		34
#define TANK_FRAMES_END_WALK		49
#define TANK_FRAMES_START_ROCKET	136
#define TANK_FRAMES_END_ROCKET		144
#define TANK_FRAMES_START_PUNCH		98//95
#define TANK_FRAMES_END_PUNCH		102

void boss_locktarget (edict_t *self, int flash_number, vec3_t forward, vec3_t start)
{
	int		i;
	vec3_t	v;

	if (self->owner->client->weapon_mode) // secondary attack
	{
		VectorCopy(self->enemy->s.origin, v);
		v[2]-=100; // aim at floor
		VectorSubtract(v, self->s.origin, forward);
	}
	else
	{
		MonsterAim(self, -1, TANK_ROCKET_SPEED, true, flash_number, forward, start);
	}

	VectorNormalize(forward);
	vectoangles(forward, forward);
	ValidateAngles(forward);

	// set view angles to target
	for (i = 0 ; i < 3 ; i++)
		self->owner->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(forward[i]-self->owner->client->resp.cmd_angles[i]);
	VectorCopy(forward, self->owner->client->ps.viewangles);
	VectorCopy(forward, self->owner->client->v_angle);
}

void boss_punch (edict_t *self)
{
	int damage;
	trace_t tr;
	edict_t *other=NULL;
	vec3_t	v;

	if (!self->groundentity)
		return;

	damage = TANK_PUNCH_INITIAL_DAMAGE+TANK_PUNCH_ADDON_DAMAGE*self->monsterinfo.level;
//	T_RadiusDamage(self, self, damage, self, TANK_PUNCH_RADIUS, 0);
	gi.sound (self, CHAN_AUTO, gi.soundindex ("tank/tnkatck5.wav"), 1, ATTN_NORM, 0);
	

	while ((other = findradius(other, self->s.origin, 256)) != NULL)
	{
		if (other == self)
			continue;
		if (!other->inuse)
			continue;
		if (!other->takedamage)
			continue;
		if (other->solid == SOLID_NOT)
			continue;
		if (!other->groundentity)
			continue;
		if (OnSameTeam(self, other))
			continue;
		if (!visible(self, other))
			continue;
		VectorSubtract(other->s.origin, self->s.origin, v);
		VectorNormalize(v);
		tr = gi.trace(self->s.origin, NULL, NULL, other->s.origin, self, (MASK_PLAYERSOLID | MASK_MONSTERSOLID));
		T_Damage (other, self, self, v, other->s.origin, tr.plane.normal, damage, damage, 0, MOD_UNKNOWN);
		other->velocity[2] += damage / 2;
	}
}

/*
void boss_tank_attack (edict_t *ent)
{
	int		damage, flash_number=0;
	vec3_t	forward, right, start;

	if (ent->s.frame == 138)
		flash_number = MZ2_TANK_ROCKET_1;
	else if (ent->s.frame == 141)
		flash_number = MZ2_TANK_ROCKET_2;
	else if (ent->s.frame == 144)
		flash_number = MZ2_TANK_ROCKET_3;
	else if (ent->style != FRAMES_ATTACK)
		return; // we shouldn't be here if we're not trying to attack!

	// secondary attack
	if (ent->owner->client->weapon_mode)
	{
		//begin attack
		if (ent->s.frame == TANK_FRAMES_START_PUNCH)
		{
			// if our closest target is out of range, teleport near him
			if (boss_findtarget(ent) && (entdist(ent, ent->enemy) > TANK_PUNCH_RADIUS))
			{
				if (TeleportNearTarget(ent, ent->enemy, 16))
					boss_locktarget(ent, 0, forward, start);
			}
			gi.sound (ent, CHAN_WEAPON, gi.soundindex ("tank/tnkatck4.wav"), 1, ATTN_NORM, 0);
		}
		if (ent->s.frame == TANK_FRAMES_END_PUNCH)
			boss_punch(ent);
		return;
	}

	if (!G_ValidTarget(ent, ent->enemy, true) || !infront(ent, ent->enemy))
	{
		// get muzzle origin and firing direction
		if (boss_findtarget(ent))
		{
			boss_locktarget(ent, flash_number, forward, start);
		}
		else
		{
			// we don't have a target, so fire where client is aiming
			AngleVectors(ent->s.angles, forward, right, NULL);
			G_ProjectSource(ent->s.origin, monster_flash_offset[flash_number], forward, right, start);
			AngleVectors(ent->activator->client->v_angle, forward, NULL, NULL);
		}
	}
	else
	{
		boss_locktarget(ent, flash_number, forward, start);
	}

	if (!flash_number) // not a firing frame
		return;

	damage = TANK_ROCKET_INITIAL_DAMAGE+TANK_ROCKET_ADDON_DAMAGE*ent->monsterinfo.level;

	AngleVectors(ent->activator->client->v_angle, forward, NULL, NULL); // client has angle to enemy
	fire_rocket(ent, start, forward, damage, TANK_ROCKET_SPEED, damage, damage);

	gi.WriteByte(svc_muzzleflash2);
	gi.WriteShort(ent-g_edicts);
	gi.WriteByte(flash_number);
	gi.multicast(start, MULTICAST_PVS);
}
*/

void boss_tank_attack (edict_t *ent)
{
	int		damage, flash_number;
	vec3_t	forward, start;

	if ((flash_number=p_tank_getFirePos(ent, start, forward))==-1)
		return;

	ent->owner->client->idle_frames = 0; // you're not idle if you're attacking!

	// punch attack
	if (ent->owner->client->weapon_mode)
	{
		//begin attack
		if (ent->s.frame == TANK_FRAMES_START_PUNCH)
		{
			if (boss_findtarget(ent) && (entdist(ent, ent->enemy) > TANK_PUNCH_RADIUS))
			{
				if (TeleportNearTarget(ent, ent->enemy, 16))
					boss_locktarget(ent, 0, forward, start);
			}
			gi.sound (ent, CHAN_WEAPON, gi.soundindex ("tank/tnkatck4.wav"), 1, ATTN_NORM, 0);
		}
		else if (ent->s.frame == TANK_FRAMES_END_PUNCH)
			boss_punch(ent);
		return;
	}

	if (!flash_number) // not a firing frame
		return;

	damage = TANK_ROCKET_INITIAL_DAMAGE+TANK_ROCKET_ADDON_DAMAGE*ent->monsterinfo.level;

	fire_rocket(ent, start, forward, damage, TANK_ROCKET_SPEED, damage, damage);

	gi.WriteByte(svc_muzzleflash2);
	gi.WriteShort(ent-g_edicts);
	gi.WriteByte(flash_number);
	gi.multicast(start, MULTICAST_PVS);
}

void boss_idle (edict_t *self)
{
	G_RunFrames(self, TANK_FRAMES_START_STAND, TANK_FRAMES_END_STAND, false);
}

void boss_tank_think (edict_t *self)
{
	int			frame;

	if (!boss_checkstatus(self))
		return;

	if (self->linkcount != self->monsterinfo.linkcount)
	{
		self->monsterinfo.linkcount = self->linkcount;
		M_CheckGround (self);
	}

	self->monsterinfo.trail_time = level.time + 1; // stay in eye-cam

	if (self->style == FRAMES_RUN_FORWARD)
		G_RunFrames(self, TANK_FRAMES_START_WALK, TANK_FRAMES_END_WALK, false);
	else if (self->style == FRAMES_RUN_BACKWARD)
		G_RunFrames(self, TANK_FRAMES_START_WALK, TANK_FRAMES_END_WALK, true);
	else if (self->style == FRAMES_ATTACK)
	{
		if (self->owner->client->weapon_mode)
			G_RunFrames(self, TANK_FRAMES_START_PUNCH, TANK_FRAMES_END_PUNCH, false);
		else
			G_RunFrames(self, TANK_FRAMES_START_ROCKET, TANK_FRAMES_END_ROCKET, false);
	}
	else
		boss_idle(self);

	boss_regenerate(self);
		
	boss_tank_attack(self);

	frame = self->s.frame;
	// play tank step sound
	if (self->groundentity && ((frame == 41) || (frame == 49)))
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("tank/step.wav"), 1, ATTN_NORM, 0);

	self->nextthink = level.time + FRAMETIME;
}

void boss_tank_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	//gi.dprintf("boss_tank_die()\n");

	respawn(self->activator);

	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_NO; // we're quite dead, dont call the death function again
	self->think = BecomeExplosion1;
	self->nextthink = level.time + FRAMETIME;
	SpawnRune(self, attacker, false);
	SpawnRune(self, attacker, false);
}

void boss_spawn_tank (edict_t *ent)
{
	char	userinfo[MAX_INFO_STRING], *message;
	//edict_t	*tank;

	//gi.dprintf("boss_spawn_tank()\n");

	if (G_EntExists(ent->owner) && (ent->owner->mtype == BOSS_TANK))
	{
		message = HiPrint(va("%s got bored and left the game.", ent->client->pers.netname));
		gi.bprintf(PRINT_HIGH, "%s\n", message);

		BecomeTE(ent->owner);
		ent->svflags &= ~SVF_NOCLIENT;
		ent->viewheight = 22;
		ent->movetype = MOVETYPE_WALK;
		ent->solid = SOLID_BBOX;
		ent->takedamage = DAMAGE_AIM;

		// recover player info
		memcpy(userinfo, ent->client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant(ent->client);
		ClientUserinfoChanged(ent, userinfo);
		modify_max(ent);
		Pick_respawnweapon(ent);
		ent->owner = NULL;
		return;
	}

	CreateBoss(ent);
/*
	message = HiPrint(va("A level %d boss known as %s has spawned!", average_player_level, ent->client->pers.netname));
	gi.bprintf(PRINT_HIGH, "%s\n", message);

	// create the tank entity that the player will pilot
	tank = G_Spawn();
	tank->classname = "boss";
	tank->solid = SOLID_BBOX;
	tank->takedamage = DAMAGE_YES;
	tank->movetype = MOVETYPE_STEP;
	tank->clipmask = MASK_MONSTERSOLID;
	tank->svflags |= SVF_MONSTER;
	tank->activator = ent;
	tank->die = boss_tank_die;
	tank->think = boss_tank_think;
	tank->mass = 500;
	tank->monsterinfo.level = average_player_level;
	tank->health = TANK_INITIAL_HEALTH+TANK_ADDON_HEALTH*tank->monsterinfo.level;
	tank->max_health = tank->health;
	tank->mtype = BOSS_TANK;
	tank->pain = boss_pain;
	tank->flags |= FL_CHASEABLE; // 3.65 indicates entity can be chase cammed
	// set up pointers
	tank->owner = ent;
	ent->owner = tank;
	
	tank->s.modelindex = gi.modelindex ("models/monsters/tank/tris.md2");
	VectorSet (tank->mins, -24, -24, -16);
	VectorSet (tank->maxs, 24, 24, 64);
	tank->s.skinnum = 2; // commander skin
	VectorCopy(ent->s.angles, tank->s.angles);
	tank->s.angles[PITCH] = 0; // monsters don't use pitch
	tank->nextthink = level.time + FRAMETIME;
	VectorCopy(ent->s.origin, tank->s.origin);
	VectorCopy(ent->s.old_origin, tank->s.old_origin);

	// link up entities
	gi.linkentity(tank);
	gi.linkentity(ent);

	// make the player into a ghost
	ent->svflags |= SVF_NOCLIENT;
	ent->viewheight = 0;
	ent->movetype = MOVETYPE_NOCLIP;
	ent->solid = SOLID_NOT;
	ent->takedamage = DAMAGE_NO;
	ent->client->ps.gunindex = 0;
	memset (ent->client->pers.inventory, 0, sizeof(ent->client->pers.inventory));
	ent->client->pers.weapon = NULL;
	*/
}
