#include "g_local.h"
#include "m_player.h"
#include "boss.h"

int		cumsindex;

//Function prototypes required for this .c file:
void ParasiteAttack (edict_t *ent);
void EatCorpses (edict_t *ent);
void PlagueCloudSpawn (edict_t *ent);
void RunCacodemonFrames (edict_t *ent, usercmd_t *ucmd);
//void RunTankFrames (edict_t *ent, usercmd_t *ucmd);
void brain_fire_beam (edict_t *self);

//
// Gross, ugly, disgustuing hack section
//

// this function is an ugly as hell hack to fix some map flaws
//
// the coop spawn spots on some maps are SNAFU.  There are coop spots
// with the wrong targetname as well as spots with no name at all
//
// we use carnal knowledge of the maps to fix the coop spot targetnames to match
// that of the nearest named single player spot

static void SP_FixCoopSpots (edict_t *self)
{
	edict_t	*spot;
	vec3_t	d;

	spot = NULL;

	while(1)
	{
		spot = G_Find(spot, FOFS(classname), "info_player_start");
		if (!spot)
			return;
		if (!spot->targetname)
			continue;
		VectorSubtract(self->s.origin, spot->s.origin, d);
		if (VectorLength(d) < 384)
		{
			if ((!self->targetname) || Q_stricmp(self->targetname, spot->targetname) != 0)
			{
				//gi.dprintf("FixCoopSpots changed %s at %s targetname from %s to %s\n", self->classname, vtos(self->s.origin), self->targetname, spot->targetname);
				self->targetname = spot->targetname;
			}
			return;
		}
	}
}

// now if that one wasn't ugly enough for you then try this one on for size
// some maps don't have any coop spots at all, so we need to create them
// where they should have been

static void SP_CreateCoopSpots (edict_t *self)
{
	edict_t	*spot;

	if(Q_stricmp(level.mapname, "security") == 0)
	{
		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 - 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 128;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		return;
	}
}


/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(edict_t *self)
{
	if (coop->value && Q_stricmp(level.mapname, "security") == 0)
	{
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_CreateCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/
void SP_info_player_deathmatch(edict_t *self)
{
	if (!deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	SP_misc_teleporter_dest (self);
}

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games
*/

void SP_info_player_coop(edict_t *self)
{
	if (!coop->value)
	{
		G_FreeEdict (self);
		return;
	}

	if((Q_stricmp(level.mapname, "jail2") == 0)   ||
	   (Q_stricmp(level.mapname, "jail4") == 0)   ||
	   (Q_stricmp(level.mapname, "mine1") == 0)   ||
	   (Q_stricmp(level.mapname, "mine2") == 0)   ||
	   (Q_stricmp(level.mapname, "mine3") == 0)   ||
	   (Q_stricmp(level.mapname, "mine4") == 0)   ||
	   (Q_stricmp(level.mapname, "lab") == 0)     ||
	   (Q_stricmp(level.mapname, "boss1") == 0)   ||
	   (Q_stricmp(level.mapname, "fact3") == 0)   ||
	   (Q_stricmp(level.mapname, "biggun") == 0)  ||
	   (Q_stricmp(level.mapname, "space") == 0)   ||
	   (Q_stricmp(level.mapname, "command") == 0) ||
	   (Q_stricmp(level.mapname, "power2") == 0) ||
	   (Q_stricmp(level.mapname, "strike") == 0))
	{
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_FixCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
}


/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
void SP_info_player_intermission(void)
{
}


//=======================================================================


void player_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	// player pain is handled at the end of the frame in P_DamageFeedback
}


qboolean IsFemale (edict_t *ent)
{
	char		*info;

	if (!ent->client)
		return false;

	info = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	if (info[0] == 'f' || info[0] == 'F')
		return true;
	return false;
}

qboolean MonsterObits (edict_t *player, edict_t *monster)
{
	edict_t *monster_owner;

	//Spirits will fail G_EntExists(), so check for them first
	if(monster->mtype != M_YANGSPIRIT 
		&& monster->mtype != M_BALANCESPIRIT
		&& monster->mtype != M_YINSPIRIT)
	{
		//Check the normal monsters (now that we know they are not spirits)
		if (!G_EntExists(player) ||  !G_EntExists(monster))
			return false;
	}

	// monsters are not clients
	if (monster->client)
		return false;
	
	// monster must have a type
	if (!monster->mtype)
		return false;
	
	// monster must have a level
	if (monster->monsterinfo.level < 1)
		return false;

	// is this monster owned by another player?
	if (!IsABoss(monster) && ((monster_owner = G_GetClient(monster)) != NULL))
	{
		gi.bprintf(PRINT_MEDIUM, "%s was killed by %s's %s\n", 
			player->client->pers.netname, monster_owner->client->pers.netname, 
			GetMonsterKindString(monster->mtype));
	}
	else
	{
		// this is a world-spawned monster
		gi.bprintf(PRINT_MEDIUM, "%s was killed by a level %d %s\n", player->client->pers.netname, 
			monster->monsterinfo.level, V_GetMonsterName(monster));
	}

	return true;
}

//K03 Begin
qboolean Monster_Obits(edict_t* victim, edict_t* attacker)
{
	char* message1 = "";
	char* message2 = "";

	// Make sure both attacker and victim still in game!
	if (attacker == NULL ||
		attacker->activator == NULL ||
		attacker->activator->client == NULL ||
		victim == NULL)
		return false;

	if (!victim->client)
		return true;

	message1 = "was killed by";

	// What type of monster was this?
	switch (attacker->mtype)
	{
	case M_BERSERK:
		message2 = "'s Berserker";
		break;
	case M_BOSS2:
		message2 = "'s Boss";
		break;
	case M_SOLDIERSS:
		message2 = "'s Machinegun Soldier";
		break;
	case M_JORG:
		message2 = "'s Jorg";
		break;
	case M_BRAIN:
		message2 = "'s Brain";
		break;
	case M_CHICK:
		message2 = "'s Chick";
		break;
	case M_FLIPPER:
		message2 = "'s Shark";
		break;
	case M_FLOATER:
		break;
	case M_FLYER:
		message2 = "'s Flyer";
		break;
	case M_INSANE:
		message2 = "'s Insane"; // how the hell does this work? -(nobody)
		break;
	case M_GLADIATOR:
		message2 = "'s Gladiator";
		break;
	case M_HOVER:
		message2 = "'s Icarus";
		break;
	case M_INFANTRY:
		message2 = "'s Infantry";
		break;
	case M_SOLDIERLT:
		message2 = "'s Blaster Guard";
		break;
	case M_SOLDIER:
		message2 = "'s Shark";
		break;
	case M_MEDIC:
		message2 = "'s Medic";
		break;
	case M_MUTANT:
		message2 = "'s Mutant";
		break;
	case M_PARASITE:
		message2 = "'s Parasite";
		break;
	case M_TANK:
		message2 = "'s Tank";
		break;
	case M_MAKRON:
		message2 = "'s Makron";
		break;
	case M_GUNNER:
		message2 = "'s Gunner";
		break;
	case M_SUPERTANK:
		message2 = "'s Supertank";
		break;
	case M_YANGSPIRIT:
	case M_YINSPIRIT:
	case M_BALANCESPIRIT:
		message1 = "was defeated by";
		message2 = va("'s %s", attacker->classname);
		break;	//3.03 Spirit skill
	default:
		return false;
	} // end switch

	// Print the obituary message..
	if (victim->client && attacker && attacker->activator && attacker->activator->client) {
		gi.bprintf(PRINT_MEDIUM, "%s %s %s%s\n", victim->client->pers.netname, message1, attacker->activator->client->pers.netname, message2);
		//		attacker->activator->client->resp.score++;
		//		attacker->activator->myskills.max_experience++;
		//		attacker->activator->myskills.streak++;
	}

	//3.0 Monster killed the enemy
	attacker->enemy = NULL;
	return true;
}
//K03 End


void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int			mod;
	char		*message;
	char		*message2;
	qboolean	ff;

	assert(attacker != NULL);
	//K03 Begin
	if (attacker && (attacker->creator) &&(!attacker->client) && (meansOfDeath == MOD_SENTRY || meansOfDeath == MOD_SENTRY_ROCKET))
		attacker = attacker->creator;
	// fire totem
	if ((attacker && attacker->mtype == TOTEM_FIRE) && attacker->owner && attacker->owner->inuse)
		attacker = attacker->owner;
	// Is this a monster doing the killing??
	//if (Monster_Obits(self, attacker))
	if (PM_MonsterHasPilot(attacker))
		attacker = attacker->activator;

	if (MonsterObits(self, attacker))
		return;

	//K03 End

	if (attacker && coop->value && attacker->client)
		meansOfDeath |= MOD_FRIENDLY_FIRE;

	if (deathmatch->value || coop->value)
	{
		ff = meansOfDeath & MOD_FRIENDLY_FIRE;
		mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
		message = NULL;
		message2 = "";

		switch (mod)
		{
		case MOD_SUICIDE:
			message = "suicides :)";
			break;
		case MOD_FALLING:
			message = "made a leap of faith";
			break;
		case MOD_CRUSH:
			message = "suffers from claustrophobia";
			break;
		case MOD_WATER:
			message = "forgot to breath";
			break;
		case MOD_SLIME:
			message = "melted";
			break;
		case MOD_LAVA:
			message = "joins the lava gods";
			break;
		case MOD_EXPLOSIVE:
		case MOD_BARREL:
			message = "blew up";
			break;
		case MOD_EXIT:
			message = "found a way out";
			break;
		case MOD_TARGET_LASER:
			message = "saw the light";
			break;
		case MOD_TARGET_BLASTER:
			message = "got blasted";
			break;
		case MOD_BOMB:
		case MOD_SPLASH:
		case MOD_TRIGGER_HURT:
			message = "was in the wrong place";
			break;
		// RAFAEL
		case MOD_GEKK:
		case MOD_BRAINTENTACLE:
			message = "got his brains sucked out";
			break;
		}
		if (attacker == self)
		{
			switch (mod)
			{
			case MOD_HELD_GRENADE:
				message = "tried to put the pin back in";
				break;
			case MOD_HG_SPLASH:
			case MOD_G_SPLASH:
				if (IsFemale(self))
					message = "hugs his grenade";
				else
					message = "hugs her grenade";
				break;
			case MOD_R_SPLASH:
				if (IsFemale(self))
					message = "hates herself";
				else
					message = "hates himself";
				break;
			case MOD_ROCKET:
				message = "ate a rocket";
				break;
			case MOD_BFG_BLAST:
				message = "should have used a smaller gun";
				break;
			case MOD_CORPSEEXPLODE:
				message = "hugs his corpse";
				break;
			case MOD_BOMBS:
				message = "plays catch with his bombs";
				break;
			case MOD_DECOY:
				message = "hates his decoy";
				break;
			// RAFAEL 03-MAY-98
			case MOD_TRAP:
			 	message = "sucked into his own trap";
				break;
			case MOD_SUPPLYSTATION:
				message = "took a seat on his exploding supply station";
				break;
			case MOD_CACODEMON_FIREBALL:
				message = "juggles his exploding skulls";
				break;
			case MOD_EXPLODINGARMOR:
				message = "shows off his exploding armor";
				break;
			case MOD_PROXY:
				message = "stood too close to his proxy grenade";
				break;
			case MOD_METEOR:
				message = "pitched a tent underneath his falling meteor";
				break;
			case MOD_NAPALM:
				message = "makes a thorough examination of his napalm";
				break;
			case MOD_EMP:
				message = "detonates his ammo";
				break;
			case MOD_SPIKEGRENADE:
				message = "ate his spike grenade";
				break;
			case MOD_FIREBALL:
				message = "rides his fireball";
				break;
			case MOD_ICEBOLT:
				message = "became a popsicle";
				break;
			case MOD_PLASMABOLT:
				message = "turns to dust";
				break;
			case MOD_LIGHTNING_STORM:
				message = "receives the death penalty";
				break;
			case MOD_MIRV:
				message = "ate his mirv grenade";
				break;
			default:
				message = "becomes bored with life";
				gi.bprintf(PRINT_MEDIUM, "mod: %i \n", mod);
				break;
			}
		}
		if (message)
		{
			gi.bprintf (PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);
			if (deathmatch->value)
				self->client->resp.score--;
		//	self->enemy = NULL;
			return;
		}

		self->enemy = attacker;
		if (attacker && attacker->client)
		{
			switch (mod)
			{
			case MOD_BLASTER:
				message = "was humiliated by";
				break;
			case MOD_SHOTGUN:
				message = "'s face was impacted by";
				message2 = "'s shotgun";
				break;
			case MOD_SSHOTGUN:
				message = "was blown to pieces by";
				message2 = "'s super shotgun";
				break;
			case MOD_MACHINEGUN:
				message = "was shredded by";
				message2 = "'s machinegun";
				break;
			case MOD_CHAINGUN:
				message = "'s torso was removed by";
				message2 = "'s chaingun";
				break;
			case MOD_GRENADE:
				message = "was gibbed by";
				message2 = "'s grenade";
				break;
			case MOD_G_SPLASH:
				message = "was splattered by";
				message2 = "'s shrapnel";
				break;
			case MOD_ROCKET:
				message = "rides";
				message2 = "'s rocket";
				break;
			case MOD_R_SPLASH:
				message = "was reamed by";
				message2 = "'s rocket";
				break;
			case MOD_HYPERBLASTER:
				message = "was liquidated by";
				message2 = "'s hyperblaster";
				break;
			case MOD_RAILGUN:
				message = "was railed by";
				break;
			case MOD_SNIPER:
				message = "receives a sucking chest wound from";
				break;
			case MOD_BFG_LASER:
				message = "turned inside out from";
				message2 = "'s BFG";
				break;
			case MOD_BFG_BLAST:
				message = "was disintegrated by";
				message2 = "'s BFG blast";
				break;
			case MOD_BFG_EFFECT:
				message = "couldn't hide from";
				message2 = "'s BFG";
				break;
			case MOD_HANDGRENADE:
				message = "tries to hatch";
				message2 = "'s handgrenade";
				break;
			case MOD_HG_SPLASH:
				message = "was split in half by";
				message2 = "'s handgrenade";
				break;
			case MOD_HELD_GRENADE:
				message = "feels";
				message2 = "'s pain";
				break;
			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space";
				break;
//ZOID
			case MOD_GRAPPLE:
				message = "was caught by";
				message2 = "'s grapple";
				break;
//ZOID
// RAFAEL 14-APR-98
			case MOD_RIPPER:
				message = "ripped to shreds by";
				message2 = "'s ripper gun";
				break;
			case MOD_PHALANX:
				message = "was evaporated by";
				break;
			case MOD_TRAP:
				message = "caught in trap by";
				break;
// END 14-APR-98	
			case MOD_BURN:
				message = "was burned alive by";
				break;
			case MOD_SWORD:
				message = "was eviscerated by";
				message2 ="'s sword";
				break;
			case MOD_LASER_DEFENSE:
				message = "saw";
				message2 = "'s light";
				break;
			case MOD_SENTRY:
				message = "'s spine was extracted by";
				message2 = "'s Sentry Gun";
				break;
			case MOD_SENTRY_ROCKET:
				message = "hates";
				message2 = "'s Sentry Gun";
				break;
			case MOD_CORPSEEXPLODE:
				message = "stood to close to";
				message2 = "'s corpse";
				break;
			case MOD_BOMBS:
				message = "happily collects all of";
				message2 = "'s exploding bombs";
				break;
			case MOD_DECOY:
				message = "gets too friendly with";
				message2 = "'s decoy";
				break;
			case MOD_CANNON:
				message = "'s ribs were shattered by";
				message2 = "'s 20mm cannon";
				break;
			case MOD_PARASITE:
				message = "tastes finger lickin' good to";
				break;
			case MOD_SUPPLYSTATION:
				message = "is blown to bits by";
				message2 = "'s supply station";
				break;
			case MOD_CACODEMON_FIREBALL:
				message = "succumbs to";
				message2 = "'s fiery skull";
				break;
			case MOD_PLAGUE:
				message = "dies from";
				message2 = "'s social disease";
				break;
			case MOD_SKULL:
				message = "is executed by";
				message2 = "'s hell spawn";
				break;
			case MOD_CORPSEEATER:
				message = "becomes";
				message2 = "'s human dessert";
				break;
			case MOD_TENTACLE:
				message = "becomes";
				message2 = "'s tasty treat";
				break;
			case MOD_BEAM:
				message = "gets cooked by";
				message2 = "'s laser beam";
				break;
			case MOD_MAGICBOLT:
				message = "catches";
				message2 = "'s magic bolt";
				break;
			case MOD_CALTROPS:
				message = "is impaled by";
				message2 = "'s caltrops";
				break;
			case MOD_CRIPPLE:
				message = "'s heart skips a beat thanks to";
				message2 = "'s cripple spell";
				break;
			case MOD_NOVA:
				message = "is blown away by";
				message2 = "'s nova spell";
				break;
			case MOD_EXPLODINGARMOR:
				message = "adorns";
				message2 = "'s exploding armor";
				break;
			case MOD_MUTANT:
				message = "was torn to pieces by";
				break;
			case MOD_SPIKE:
				message = "was poked to death by";
				message2 = "'s spike";
				break;
			case MOD_SPIKEGRENADE:
				message = "got cozy with";
				message2 = "'s spike grenade";
				break;
			case MOD_MINDABSORB:
				message = "'s brain was fried by";
				message2 = "'s mind absorb spell";
				break;
			case MOD_LIFE_DRAIN:
				message = "'s soul was taken by";
				break;
			case MOD_PROXY:
				message = " got a closer look of";
				message2 = "'s exploding proxy grenade";
				break;
			case MOD_METEOR:
				message = " was crushed under";
				message2 = "'s meteorite";
				break;
			case MOD_NAPALM:
				message = "enjoys";
				message2 = "'s toasty napalm";
				break;
			case MOD_TANK_PUNCH:
				message = "was pulverized by";
				break;
			case MOD_TANK_BULLET:
				message = "was perforated by";
				break;
			case MOD_FREEZE:
				message = "was frozen solid by";
				break;
			case MOD_LIGHTNING:
				message = "was electrocuted by";
				break;
			case MOD_AUTOCANNON:
				message = "receives a lead enema from";
				message2 = "'s autocannon";
				break;
			case MOD_HAMMER:
				message = "was smashed by";
				message2 = "'s blessed hammer";
				break;
			case MOD_BLACK_HOLE:
				message = "was crushed by the gravitational forces of";
				message2 = "'s black hole";
				break;
			case MOD_BERSERK_PUNCH:
				message = "was knocked out by";
				break;
			case MOD_BERSERK_CRUSH:
				message = "was pulverized by";
				break;
			case MOD_BERSERK_SLASH:
				message = "was ripped apart by";
				break;
			case MOD_BLEED:
				message = "donates blood to";
				break;
			case MOD_EMP:
				message = "'s ammo detonates thanks to";
				message2 = "'s EMP grenade";
				break;
			case MOD_FIREBALL:
				message = "caught";
				message2 = "'s fireball";
				break;
			case MOD_ICEBOLT:
				message = "became";
				message2 = "'s ice sculpture";
				break;
			case MOD_PLASMABOLT:
				message = "was disintegrated by";
				message2 = "'s plasma bolt";
				break;
			case MOD_LIGHTNING_STORM:
				message = "was electrocuted by";
				message2 = "'s lightning storm";
				break;
			case MOD_MIRV:
				message = "was blown away by";
				message2 = "'s mirv grenade";
				break;
			case MOD_OBSTACLE:
				message = "was impaled by";
				message2 = "'s obstacle";
				break;
			case MOD_GAS:
				message = "visits";
				message2 = "'s gas chamber";
				break;
			case MOD_ACID:
				message = "took a bath in";
				message2 = "'s acid";
				break;
			case MOD_UNHOLYGROUND:
				message = "was sacrificed by";
				message2 = "'s unholy ground";
				break;
				//K03 End
			}
			if (message)
			{
				gi.bprintf (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
				if (deathmatch->value)
				{
					if (ff)
						attacker->client->resp.score--;
					else
						attacker->client->resp.score++;
				}
				return;
			}
		}
	}

	gi.bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);
	if (deathmatch->value)
		self->client->resp.score--;
}


void TossClientWeapon (edict_t *self)
{
	gitem_t		*item;
	edict_t		*drop;
	qboolean	quad;
	// RAFAEL
	qboolean	quadfire;
	float		spread;

	if (!deathmatch->value)
		return;

	item = self->client->pers.weapon;
	if (! self->client->pers.inventory[self->client->ammo_index] )
		item = NULL;
	if (item && (strcmp (item->pickup_name, "Blaster") == 0))
		item = NULL;

	if (!((int)(dmflags->value) & DF_QUAD_DROP))
		quad = false;
	else
		quad = (self->client->quad_framenum > (level.framenum + 10));

	// RAFAEL
	if (!((int)(dmflags->value) & DF_QUADFIRE_DROP))
		quadfire = false;
	else
		quadfire = (self->client->quadfire_framenum > (level.framenum + 10));
	

	if (item && quad)
		spread = 22.5;
	else if (item && quadfire)
		spread = 12.5;
	else
		spread = 0.0;

	if (item)
	{
		self->client->v_angle[YAW] -= spread;
		drop = Drop_Item (self, item);
		self->client->v_angle[YAW] += spread;
		drop->spawnflags = DROPPED_PLAYER_ITEM;
		self->client->pers.inventory[ITEM_INDEX(item)] = 0;
		self->client->newweapon = FindItem ("Blaster");
		self->client->weaponstate = WEAPON_ACTIVATING;
		self->client->ps.gunframe = 0;
		ChangeWeapon (self);
	}

	if (quad)
	{
		self->client->v_angle[YAW] += spread;
		drop = Drop_Item (self, FindItemByClassname ("item_quad"));
		self->client->v_angle[YAW] -= spread;
		drop->spawnflags |= DROPPED_PLAYER_ITEM;

		drop->touch = Touch_Item;
		drop->nextthink = level.time + (self->client->quad_framenum - level.framenum) * FRAMETIME;
		drop->think = G_FreeEdict;
	}
	// RAFAEL
	if (quadfire)
	{
		self->client->v_angle[YAW] += spread;
		drop = Drop_Item (self, FindItemByClassname ("item_quadfire"));
		self->client->v_angle[YAW] -= spread;
		drop->spawnflags |= DROPPED_PLAYER_ITEM;

		drop->touch = Touch_Item;
		drop->nextthink = level.time + (self->client->quadfire_framenum - level.framenum) * FRAMETIME;
		drop->think = G_FreeEdict;
	}
}


/*
==================
LookAtKiller
==================
*/
void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	vec3_t		dir;

	if (attacker && attacker != world && attacker != self)
	{
		VectorSubtract (attacker->s.origin, self->s.origin, dir);
	}
	else if (inflictor && inflictor != world && inflictor != self)
	{
		VectorSubtract (inflictor->s.origin, self->s.origin, dir);
	}
	else
	{
		self->client->killer_yaw = self->s.angles[YAW];
		return;
	}

	self->client->killer_yaw = 180/M_PI*atan2(dir[1], dir[0]);
}

/*
==================
player_die
==================
*/

void VortexDeathCleanup(edict_t *attacker, edict_t *targ);

void VortexGibSound (edict_t *self, int index)
{
	switch (index)
	{
	case 1: gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0); break;
	case 2: gi.sound (self, CHAN_BODY, gi.soundindex ("death/gib2.wav"), 1, ATTN_NORM, 0); break;
	case 3: gi.sound (self, CHAN_BODY, gi.soundindex ("death/gib3.wav"), 1, ATTN_NORM, 0); break;
	case 4: gi.sound (self, CHAN_BODY, gi.soundindex ("death/malive5.wav"), 1, ATTN_NORM, 0); break;
	case 5: gi.sound (self, CHAN_BODY, gi.soundindex ("death/mdeath4.wav"), 1, ATTN_NORM, 0); break;
	case 6: gi.sound (self, CHAN_BODY, gi.soundindex ("death/mdeath5.wav"), 1, ATTN_NORM, 0); break;
	default: gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0); break;
	}
}

void VortexPlayerDeath (edict_t *self, edict_t *attacker, edict_t *inflictor)
{
	if (debuginfo->value > 1)
		gi.dprintf("VortexPlayerDeath()\n");
	
	// only call this function once
	if (self->deadflag)
		return;

	V_ResetPlayerState(self);
	self->gib_health = -100;

	// don't drop powercubes or tballs
	self->myskills.inventory[ITEM_INDEX(Fdi_POWERCUBE)] = self->client->pers.inventory[ITEM_INDEX(Fdi_POWERCUBE)];
	self->myskills.inventory[ITEM_INDEX(Fdi_TBALL)] = self->client->pers.inventory[ITEM_INDEX(Fdi_TBALL)];

	VortexAddExp(attacker, self); // modify experience
	VortexDeathCleanup(attacker,self);

	TossClientBackpack(self, attacker); // toss a backpack
	SpawnRune(self, attacker, false); // possibly generate a rune
	GetScorePosition();// gets each persons rank and total players in game

	dmgListCleanup(self, true);
}

void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;
//GHz START
	if (debuginfo->value > 1)
		gi.dprintf("player_die()\n");
	
	VortexPlayerDeath(self, attacker, inflictor);
//GHz END
	VectorClear (self->avelocity);

	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;
	self->s.modelindex2 = 0; // remove linked weapon model
	self->s.modelindex3 = 0;
	self->s.angles[0] = 0;
	self->s.angles[2] = 0;
	self->s.sound = 0;
	self->client->weapon_sound = 0;
//GHz START
	if (self->mtype != M_MYPARASITE) // parasite's bbox is already short enough
		self->maxs[2] = -8;
//GHz END
	self->svflags |= SVF_DEADMONSTER;

	if (!self->deadflag) // called only once at player death
	{
//GHz START
		if (ctf->value)
			CTF_PlayerRespawnTime(self);
		else
//GHz END
			self->client->respawn_time = level.time + 1.0;

		LookAtKiller (self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary (self, inflictor, attacker);
		//TossClientWeapon (self);
		if (deathmatch->value)
			Cmd_Help_f (self);		// show scores
		// clear inventory
		memset(self->client->pers.inventory, 0, sizeof(self->client->pers.inventory));
	}

	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;
	self->flags &= ~FL_POWER_ARMOR;
	// RAFAEL
	self->client->quadfire_framenum = 0;
//GHz START
	if (self->health < self->gib_health)
	{	// gib
		VortexGibSound(self, GetRandom(1, 6));
//GHz END
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowClientHead (self, damage);
//ZOID
		self->client->anim_priority = ANIM_DEATH;
		self->client->anim_end = 0;
//ZOID.
		self->takedamage = DAMAGE_NO;
	}
	else
	{	// normal death
		if (!self->deadflag)
		{
			static int i;

			i = (i+1)%3;
			// start a death animation
			self->client->anim_priority = ANIM_DEATH;
			if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				self->s.frame = FRAME_crdeath1-1;
				self->client->anim_end = FRAME_crdeath5;
			}
			else switch (i)
			{
			case 0:
				self->s.frame = FRAME_death101-1;
				self->client->anim_end = FRAME_death106;
				break;
			case 1:
				self->s.frame = FRAME_death201-1;
				self->client->anim_end = FRAME_death206;
				break;
			case 2:
				self->s.frame = FRAME_death301-1;
				self->client->anim_end = FRAME_death308;
				break;
			}
			gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
		}
	}

	self->deadflag = DEAD_DEAD;
	gi.linkentity (self);
}

//=======================================================================

/*
==============
InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
void InitClientPersistant (gclient_t *client)
{
	int saved;
	//K03 Begin
	gitem_t		*item;
	
	int spectator=client->pers.spectator;

	if (debuginfo->value > 1)
		gi.dprintf("InitClientPersistant()\n");

	// 4.5 - latest client combat preferences, don't clear this!
	saved = client->pers.combat_changed;

	//K03 End

	memset (&client->pers, 0, sizeof(client->pers));

	//K03 Begin
	item = FindItem("Blaster");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[ITEM_INDEX(item)] = 1;
	client->pers.inventory[ITEM_INDEX(FindItem("Sword"))] = 1;
//	client->pers.inventory[ITEM_INDEX(FindItem("Lasers"))] = LASER_SPAWN; 
	client->pers.inventory[ITEM_INDEX(FindItem("Power Screen"))] = 1;
	client->pers.inventory[flag_index] = 0;
	client->pers.inventory[red_flag_index] = 0;
	client->pers.inventory[blue_flag_index] = 0;

	client->pers.spectator = spectator;
	client->pers.weapon = item;
	client->pers.combat_changed = saved;//4.5
	//K03 End

	client->pers.health			= 100;
	client->pers.max_health		= 100;

	client->pers.max_bullets	= 200;
	client->pers.max_shells		= 100;
	client->pers.max_rockets	= 50;
	client->pers.max_grenades	= 50;
	client->pers.max_cells		= 200;
	client->pers.max_slugs		= 50;

	// RAFAEL
	client->pers.max_magslug	= 50;
	client->pers.max_trap		= 5;

	//K03 Begin
	client->pers.max_powercubes = 200;
	client->pers.max_tballs = 20;
	//K03 End

	client->pers.connected = true;
	ClearScanner(client);
}


void InitClientResp (gclient_t *client)
{
	memset (&client->resp, 0, sizeof(client->resp));
	client->resp.enterframe = level.framenum;
	client->resp.coop_respawn = client->pers;
}

/*
==================
SaveClientData

Some information that should be persistant, like health, 
is still stored in the edict structure, so it needs to
be mirrored out to the client structure before all the
edicts are wiped.
==================
*/
void SaveClientData (void)
{
	int		i;
	edict_t	*ent;

	for (i=0 ; i<game.maxclients ; i++)
	{
		ent = &g_edicts[1+i];
		if (!ent->inuse)
			continue;
		game.clients[i].pers.health = ent->health;
		game.clients[i].pers.max_health = ent->max_health;
		game.clients[i].pers.powerArmorActive = (ent->flags & FL_POWER_ARMOR);
		if (coop->value)
			game.clients[i].pers.score = ent->client->resp.score;
	}
}

void FetchClientEntData (edict_t *ent)
{
	ent->health = ent->client->pers.health;
	ent->max_health = ent->client->pers.max_health;
	if (ent->client->pers.powerArmorActive)
		ent->flags |= FL_POWER_ARMOR;
	if (coop->value)
		ent->client->resp.score = ent->client->pers.score;
}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float	PlayersRangeFromSpot (edict_t *spot)
{
	edict_t	*player;
	float	bestplayerdistance;
	vec3_t	v;
	int		n;
	float	playerdistance;


	bestplayerdistance = 9999999;

	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];

		if (!player->inuse)
			continue;

		if (player->health <= 0)
			continue;

		VectorSubtract (spot->s.origin, player->s.origin, v);
		playerdistance = VectorLength (v);

		if (playerdistance < bestplayerdistance)
			bestplayerdistance = playerdistance;
	}

	return bestplayerdistance;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
edict_t *SelectRandomDeathmatchSpawnPoint (edict_t *ent)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	//gi.dprintf("SelectRandomDeathmatchSpawnPoint()\n");

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		if (ctf->value && ent && spot->teamnum && spot->teamnum != ent->teamnum)
			continue;

		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return NULL;

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/
edict_t *SelectFarthestDeathmatchSpawnPoint (edict_t *ent)
{
	edict_t	*bestspot;
	float	bestdistance, bestplayerdistance;
	edict_t	*spot;

	//gi.dprintf("SelectFarthestDeathmatchSpawnPoint()\n");

	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		// in CTF, don't use spawns without matching teamnum, unless they are unclaimed
		//FIXME: ent can be NULL, but for some reason we still crash here :(
		if (ctf->value && ent && ent->inuse && ent->teamnum && spot->teamnum && spot->teamnum != ent->teamnum)
			continue;

		bestplayerdistance = PlayersRangeFromSpot (spot);

		if (bestplayerdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}
	}

	if (bestspot)
	{
		return bestspot;
	}

	if (ctf->value)
		return NULL; // find somewhere else to spawn!

	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");

	return spot;
}

edict_t *SelectDeathmatchSpawnPoint (edict_t *ent)
{
	if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
		return SelectFarthestDeathmatchSpawnPoint (ent);
	else
		return SelectRandomDeathmatchSpawnPoint (ent);
}


edict_t *SelectCoopSpawnPoint (edict_t *ent)
{
	int		index;
	edict_t	*spot = NULL;
	char	*target;

	index = ent->client - game.clients;

	// player 0 starts in normal player spawn point
	if (!index)
		return NULL;

	spot = NULL;

	// assume there are four coop spots at each spawnpoint
	while (1)
	{
		spot = G_Find (spot, FOFS(classname), "info_player_coop");
		if (!spot)
			return NULL;	// we didn't have enough...

		target = spot->targetname;
		if (!target)
			target = "";
		if ( Q_stricmp(game.spawnpoint, target) == 0 )
		{	// this is a coop spawn point for one of the clients here
			index--;
			if (!index)
				return spot;		// this is it
		}
	}


	return spot;
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
qboolean SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles)
{
	edict_t	*spot = NULL;
	qboolean failed=false;

//GHz START
	if (ctf->value)
	{
		if ((spot = CTF_SelectSpawnPoint(ent)) == NULL)
		{
			edict_t *base = CTF_GetFlagBaseEnt(ent->teamnum);

			//gi.dprintf("CTF_SelectSpawnPoint() couldn't find a player spawn\n");

			if (base && !TeleportNearArea(ent, base->s.origin, 512, false))
				failed = true;
			else if (!base && !FindValidSpawnPoint(ent, false))
				failed = true;
			else
			{
				//gi.dprintf("successfully teleported\n");
				VectorCopy(ent->s.angles, angles);
				VectorCopy(ent->s.origin, origin);
				VectorCopy(vec3_origin, ent->s.origin); // to avoid killbox() fragging us
				gi.linkentity(ent);
				origin[2] += 9;
				return true;
			}

			if (failed)
				gi.dprintf("CTF failed to find a valid spawn point for player\n");
		}
	}
			
	// if we're in invasion mode, then only use invasion spawns
	else if (INVASION_OTHERSPAWNS_REMOVED)
	{
		// if a spot is not found, they'll have to wait until a spawn is free
		if ((spot = INV_SelectPlayerSpawnPoint(ent)) != NULL)
		{
			INV_RemoveSpawnQue(ent); // remove from waiting list
			ent->spawn = NULL; // player is no longer assigned this spawn
			VectorCopy (spot->s.origin, origin);
			origin[2] += 9;
			VectorCopy (spot->s.angles, angles);
			return true;
		}
		return false;
	}
//GHz END
	else if (deathmatch->value)
		spot = SelectDeathmatchSpawnPoint (ent);
	else if (coop->value)
		spot = SelectCoopSpawnPoint (ent);

	// find a single player start spot
	if (!spot)
	{
		while ((spot = G_Find (spot, FOFS(classname), "info_player_start")) != NULL)
		{
			if (!game.spawnpoint[0] && !spot->targetname)
				break;

			if (!game.spawnpoint[0] || !spot->targetname)
				continue;

			if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
				break;
		}

		if (!spot)
		{
			if (!game.spawnpoint[0])
			{	// there wasn't a spawnpoint without a target, so use any
				spot = G_Find (spot, FOFS(classname), "info_player_start");
			}
			if (!spot)
			{
				gi.error("Couldn't find spawn point %s\n", game.spawnpoint);
				return false; //QW// Never executed, passify compiler.
			}
		}
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);
	return true;
}

//======================================================================


void InitBodyQue (void)
{
	int		i;
	edict_t	*ent;

	level.body_que = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++)
	{
		ent = G_Spawn();
		ent->classname = "bodyque";
	}
}


void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int	n;

	if (self->health < self->gib_health) //GHz changed from -40
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		self->s.origin[2] -= 48;
		ThrowClientHead (self, damage);
		self->s.frame = 0;
		self->takedamage = DAMAGE_NO; //3.0 gibbed corpses should not be solid! ;)
		self->solid = SOLID_NOT;
	}
}

//K03 Begin
void floatbody(edict_t *ent)
{
   if (!(ent->flipping & FLIP_WATER))
      {
      ent->s.origin[2] -= 18;
      if (gi.pointcontents (ent->s.origin) & MASK_WATER)
         {
         ent->s.origin[2] += 3;
         ent->nextthink = level.time + .1;
         if (!(gi.pointcontents (ent->s.origin) & MASK_WATER))
            {
            ent->flipping |= FLIP_WATER;
            ent->style = 1;
            ent->speed = ent->s.origin[2] + 16;
            ent->accel = ent->s.origin[2] + 18;
            ent->decel = level.time + 4 + (rand() % 10);
            }
         }
      ent->s.origin[2] += 18;
      }
   else
      {
      //if (ent->s.origin[2] > ent->accel || ent->s.origin[2] < ent->speed) 
      //   ent->style = -ent->style;
      //ent->s.origin[2] += ent->style;   
      ent->nextthink = level.time + .2;
      if (ent->decel < level.time)
         {
         gi.sound (ent, CHAN_VOICE, gi.soundindex(va("player/wade%i.wav", (rand()%3)+1)), .6, ATTN_STATIC, 0);
         ent->decel = level.time + 4 + (rand() % 10);
         }
      }
}
//K03 End
void body_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	forward, right, start, offset;
	
	V_Touch(self, other, plane, surf);

	// push the body
	if (G_EntIsAlive(other) && other->client)
	{
		AngleVectors (other->client->v_angle, forward, right, NULL);
		VectorScale (forward, -3, other->client->kick_origin);
		VectorSet(offset, 0, 7,  other->viewheight-8);
		P_ProjectSource (other->client, other->s.origin, offset, forward, right, start);

		self->velocity[0] += forward[0] * 30;
		self->velocity[1] += forward[1] * 30;
		self->velocity[2] = 100;
	}
}

void body_think (edict_t *self)
{
	self->s.effects = 0;
	self->s.renderfx = 0;

	if (self->linkcount != self->monsterinfo.linkcount)
	{
		self->monsterinfo.linkcount = self->linkcount;
		M_CheckGround (self);
	}

	if (self->groundentity)
	{
		self->velocity[0] *= 0.6;
		self->velocity[1] *= 0.6;
	}

	self->nextthink = level.time+FRAMETIME;
}

void CopyToBodyQue (edict_t *ent)
{
	edict_t		*body;

	// grab a body que and cycle to the next one
	body = &g_edicts[(int)maxclients->value + level.body_que + 1];
	level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

	// FIXME: send an effect on the removed body

	gi.unlinkentity (ent);
	gi.unlinkentity (body);

	body->s = ent->s;
//GHz START
	body->s.effects = 0;
	body->s.renderfx = 0;
//GHz END
	body->s.number = body - g_edicts;
	body->svflags = ent->svflags;
	VectorCopy (ent->mins, body->mins);
	VectorCopy (ent->maxs, body->maxs);
	VectorCopy (ent->absmin, body->absmin);
	VectorCopy (ent->absmax, body->absmax);
	VectorCopy (ent->size, body->size);
	body->solid = ent->solid;
	body->clipmask = ent->clipmask;
	body->owner = ent->owner;
	body->movetype = ent->movetype;
	body->die = body_die;
	body->takedamage = DAMAGE_YES;
//GHz START
	body->deadflag = DEAD_DEAD; // added so its easier to identify a body
	body->gib_health = ent->gib_health; // necessary for corpse explosion and yin spirit
	body->max_health = ent->max_health;//4.55
//GHz END

	gi.linkentity (body);
}

void respawn (edict_t *self)
{
	if (debuginfo->value > 1)
		gi.dprintf("respawn()\n");
	if (deathmatch->value || coop->value)
	{
		// don't let them respawn unless they've been assigned a spawn
		if (INVASION_OTHERSPAWNS_REMOVED)
		{
			if (!self->spawn)
			{
				INV_AddSpawnQue(self);
				return;
			}
		}

		// spectator's don't leave bodies
		if (self->movetype != MOVETYPE_NOCLIP)
			CopyToBodyQue (self);
		self->mtype = 0;
		self->svflags &= ~SVF_NOCLIENT;

		if (self->myskills.respawns != (unsigned int)self->client->pers.combat_changed)
			gi.cprintf(self, PRINT_HIGH, "Combat preferences updated.\n");
		self->myskills.respawns = self->client->pers.combat_changed;//4.5 use changed combat preferences
		
		PutClientInServer (self);

		// add a teleportation effect
		self->s.event = EV_PLAYER_TELEPORT;

		// hold in place briefly
		self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		self->client->ps.pmove.pm_time = 14;

		//3.0 You get invincibility when you spawn, but you can't shoot
		self->client->respawn_time = self->client->ability_delay = level.time + (RESPAWN_INVIN_TIME / 10);
		self->client->invincible_framenum = level.framenum + RESPAWN_INVIN_TIME;		
		return;
	}

	// restart the entire server
	gi.AddCommandString ("menu_loadgame\n");
}

/* 
 * only called when pers.spectator changes
 * note that resp.spectator should be the opposite of pers.spectator here
 */
void spectator_respawn (edict_t *ent)
{
	int i, numspec;

	if (debuginfo->value > 1)
		gi.dprintf("spectator_respawn()\n");
	// if the user wants to become a spectator, make sure he doesn't
	// exceed max_spectators

	if (ent->client->pers.spectator) {
		char *value = Info_ValueForKey (ent->client->pers.userinfo, "spectator");
		if (*spectator_password->string && 
			strcmp(spectator_password->string, "none") && 
			strcmp(spectator_password->string, value)) {
			gi.cprintf(ent, PRINT_HIGH, "Spectator password incorrect.\n");
			ent->client->pers.spectator = false;
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("spectator 0\n");
			gi.unicast(ent, true);
			return;
		}

		// count spectators
		for (i = 1, numspec = 0; i <= maxclients->value; i++)
			if (g_edicts[i].inuse && g_edicts[i].client->pers.spectator)
				numspec++;

		if (numspec >= maxspectators->value) {
			gi.cprintf(ent, PRINT_HIGH, "Server spectator limit is full.");
			ent->client->pers.spectator = false;
			// reset his spectator var
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("spectator 0\n");
			gi.unicast(ent, true);
			return;
		}
	} else {
		// he was a spectator and wants to join the game
		// he must have the right password
		char *value = Info_ValueForKey (ent->client->pers.userinfo, "password");
		if (*password->string && strcmp(password->string, "none") && 
			strcmp(password->string, value)) {
			gi.cprintf(ent, PRINT_HIGH, "Password incorrect.\n");
			ent->client->pers.spectator = true;
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("spectator 1\n");
			gi.unicast(ent, true);
			return;
		}
	}

	// clear client on respawn
	ent->client->resp.score = ent->client->pers.score = 0;

	ent->svflags &= ~SVF_NOCLIENT;
	PutClientInServer (ent);

	// add a teleportation effect
	if (!ent->client->pers.spectator)  {
		// send effect
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
	}

	ent->client->respawn_time = level.time;

	if (ent->client->pers.spectator) 
		gi.bprintf (PRINT_HIGH, "%s has moved to the sidelines\n", ent->client->pers.netname);
	else
		gi.bprintf (PRINT_HIGH, "%s joined the game\n", ent->client->pers.netname);
}

//==============================================================

/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
// ### Hentai ### BEGIN
void ShowGun(edict_t *ent);
// ###	Hentai ### END

//void MorphToBrain (edict_t *ent);
void PutClientInServer (edict_t *ent)
{
	vec3_t	mins = {-16, -16, -24};
	vec3_t	maxs = {16, 16, 32};
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	client_persistant_t	saved;
	client_respawn_t	resp;

	if (debuginfo->value > 1)
		gi.dprintf("PutClientInServer()\n");

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	if (!SelectSpawnPoint (ent, spawn_origin, spawn_angles))
		return;

	index = ent-g_edicts-1;
	client = ent->client;

	// deathmatch wipes most client data every spawn
	if (deathmatch->value)
	{
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant (client);
		ClientUserinfoChanged (ent, userinfo);
	}
	else if (coop->value)
	{
		int			n;
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		// this is kind of ugly, but it's how we want to handle keys in coop
		for (n = 0; n < MAX_ITEMS; n++)
		{
			if (itemlist[n].flags & IT_KEY)
				resp.coop_respawn.inventory[n] = client->pers.inventory[n];
		}
		client->pers = resp.coop_respawn;
		ClientUserinfoChanged (ent, userinfo);
		if (resp.score > client->pers.score)
			client->pers.score = resp.score;
	}
	else
	{
		memset (&resp, 0, sizeof(resp));
	}
	
	// clear everything but the persistant data
	saved = client->pers;
	memset (client, 0, sizeof(*client));
	client->pers = saved;
	if (client->pers.health <= 0)
		InitClientPersistant(client);
	client->resp = resp;

	// copy some data from the client to the entity
	FetchClientEntData (ent);

	//K03 Begin
	if (ent->myskills.class_num > 0)
	{
		int talentLevel;

		modify_health(ent);
		modify_max(ent);
		Give_respawnweapon(ent, ent->myskills.respawn_weapon);
		Give_respawnitems(ent);

		//Talent: Sidearms
		talentLevel = getTalentLevel(ent, TALENT_SIDEARMS);
		if(talentLevel > 0)
		{
			//Give the player one additional respawn weapon for every point in the talent.
			//This does not give them ammo.
			for(i = 0; i < talentLevel+1; ++i)
				giveAdditionalRespawnWeapon(ent, i+1);
		}

		//4.57 give a partial ability charge
		if (ent->myskills.abilities[SHIELD].current_level > 0)
			ent->myskills.abilities[SHIELD].charge = 50;
		if (ent->myskills.abilities[BERSERK].current_level > 0)
			ent->myskills.abilities[BERSERK].charge = 50;
		if (ent->myskills.abilities[BEAM].current_level > 0)
			ent->myskills.abilities[BEAM].charge = 50;

	}
	//K03 End

	// clear entity values
	ent->groundentity = NULL;
	ent->client = &game.clients[index];
	ent->takedamage = DAMAGE_AIM;
	ent->movetype = MOVETYPE_WALK;
	ent->viewheight = 22;
	ent->inuse = true;
	ent->classname = "player";
	ent->mass = 200;
	ent->solid = SOLID_BBOX;
	ent->deadflag = DEAD_NO;
	ent->owner = NULL; //GHz
	ent->air_finished = level.time + 12;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->model = "players/male/tris.md2";
	ent->pain = player_pain;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->flags &= ~FL_CHATPROTECT;//GHz
	ent->svflags &= ~SVF_DEADMONSTER;
	ent->svflags &= ~SVF_MONSTER;
	ent->lastkill = level.time;//GHz
	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(client->ps));

	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;
//ZOID
	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
//ZOID

	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		client->ps.fov = 90;
	}
	else
	{
		client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
		if (client->ps.fov < 1)
			client->ps.fov = 90;
		else if (client->ps.fov > 160)
			client->ps.fov = 160;
	}

	if (client->pers.weapon)//K03
		client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

	// clear entity state values
	ent->s.effects = 0;
	ent->s.skinnum = ent - g_edicts - 1;
	ent->s.modelindex = 255;		// will use the skin specified model
//	ent->s.modelindex2 = 255;		// custom gun model
	ShowGun(ent);					// ### Hentai ### special gun model
	ent->s.frame = 0;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground
	VectorCopy (ent->s.origin, ent->s.old_origin);

	// set the delta angle
	for (i=0 ; i<3 ; i++)
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, client->ps.viewangles);
	VectorCopy (ent->s.angles, client->v_angle);

	ent->client->lowlight = 0; //GHz: clear lowlight flag

	// spawn a spectator
	if (client->pers.spectator) {
		client->chase_target = NULL;

		client->resp.spectator = true;

		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->ps.gunindex = 0;
		gi.linkentity (ent);
		//K03 return;
	} else
		client->resp.spectator = false;

	if (level.time < pregame_time->value) {
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE);
	}

	//K03 Begin
	GetScorePosition();//Gets each persons rank and total players in game
	if (StartClient(ent))
		return;

//	if (ent->myskills.class_num == CLASS_BRAIN)
//		MorphToBrain(ent);

	ent->client->invincible_framenum = level.framenum + 20;//Add 2.0 seconds to invincibility when you spawn.
	KickPlayerBack(ent);//Kicks all campers away!
	//K03 End

	KillBox(ent);

	gi.linkentity (ent);

	// force the current weapon up
	client->newweapon = client->pers.weapon;
	ChangeWeapon (ent);
}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in 
deathmatch mode, so clear everything out before starting them.
=====================
*/
void ClientBeginDeathmatch (edict_t *ent)
{
	if (debuginfo->value > 1)
		gi.dprintf("ClientBeginDeathmatch()\n");

	G_InitEdict (ent);
	assert(ent != NULL);

	InitClientResp (ent->client);

	//K03 Begin
	ent->client->pers.spectator = true;
	ent->client->resp.spectator = true;
	ent->client->cloaking = false;
	ent->max_pipes = 0;
	ent->num_hammers = 0;//4.4 Talent: Boomerang
	
	G_StuffPlayerCmds(ent, "alias +thrust thrust on\nalias -thrust thrust off\n");
	G_StuffPlayerCmds(ent, "alias +hook hook\nalias -hook unhook\n");
	G_StuffPlayerCmds(ent, "alias +superspeed sspeed\nalias -superspeed nosspeed on\n");
	G_StuffPlayerCmds(ent, "alias +sprint sprinton\nalias -sprint sprintoff\n");
	G_StuffPlayerCmds(ent, "alias +shield shieldon\nalias -shield shieldoff\n");
	G_StuffPlayerCmds(ent, "alias +lockon lockon_on\nalias -lockon lockon_off\n");
	G_StuffPlayerCmds(ent, "alias +beam beam_on\nalias -beam beam_off\n");
	// WARNING: this might cause overflowing!!!
	//stuffcmd(ent, "alias +thrust thrust on\nalias -thrust thrust off\n");
	//stuffcmd(ent, "alias +hook hook\nalias -hook unhook\n");
	//stuffcmd(ent, "alias +superspeed sspeed\nalias -superspeed nosspeed on\n");
	//stuffcmd(ent, "alias +lockon lockon_on\nalias -lockon lockon_off\n");
	//stuffcmd(ent, "alias +beam beam_on\nalias -beam beam_off\n");

	if (SPREE_WAR == true && ent && ent == SPREE_DUDE)
	{
		SPREE_WAR = false;
		SPREE_DUDE = NULL;
	}
	//K03 End

	// locate ent at a spawn point
	PutClientInServer (ent);

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

		gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);

//	gi.centerprintf(ent,ClientMessage);

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
}


/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void ClientBegin (edict_t *ent)
{
	int		i;

	if (debuginfo->value > 1)
		gi.dprintf("ClientBegin()\n");

	ent->client = game.clients + (ent - g_edicts - 1);

	//[QBS]
	// set msg mode fully on so zbot would receive text & crash :)
	gi.WriteByte (svc_stufftext);
	gi.WriteString ("msg 4\n");
	gi.unicast(ent, true);

	//[QBS] RATBOT STOPPER 
	gi.WriteByte (svc_stufftext);
	gi.WriteString (".==|please.disconnect.all.bots|==.\n");
	gi.unicast(ent, true);
	//[QBS]

	gi.WriteByte (svc_stufftext);
	gi.WriteString ("msg 0\n");
	gi.unicast(ent, true);
	//[QBS]end

	if (deathmatch->value)
	{
		ClientBeginDeathmatch (ent);
		return;
	}

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == true)
	{
		// the client has cleared the client side viewangles upon
		// connecting to the server, which is different than the
		// state when the game is saved, so we need to compensate
		// with deltaangles
		for (i=0 ; i<3 ; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->ps.viewangles[i]);
	}
	else
	{
		// a spawn point will completely reinitialize the entity
		// except for the persistant data that was initialized at
		// ClientConnect() time
		G_InitEdict (ent);
		ent->classname = "player";
		InitClientResp (ent->client);
		PutClientInServer (ent);
	}

	if (level.intermissiontime)
	{
		MoveClientToIntermission (ent);
	}
	else
	{
		// send effect if in a multiplayer game
		if (game.maxclients > 1)
		{
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_LOGIN);
			gi.multicast (ent->s.origin, MULTICAST_PVS);

			gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
		}
	}

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
}

/*
===========
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
//K03 Begin
qboolean Is_Observer(edict_t *self);
//K03 End
void ClientUserinfoChanged (edict_t *ent, char *userinfo)
{
	char	*s;
	int		playernum;
	char	ip[16];
	
	if (debuginfo->value > 1)
		gi.dprintf("ClientUserinfoChanged()\n");
	// check for malformed or illegal info strings

	if (!Info_Validate(userinfo))
	{
		strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
	}

	//r1: FIXME: clean up client pers/resp system, don't allow the player to change their own ip via userinfo!!
	
	// don't allow invalid ip
	s = Info_ValueForKey (userinfo, "ip");
	if (strlen(s) < 1)
	{
		s = Info_ValueForKey (ent->client->pers.userinfo, "ip");
		Info_SetValueForKey(userinfo, "ip", s);
	}
	
	// update current ip, with less userinfo clobbering
	s = Info_ValueForKey (userinfo, "ip");

	if (s[0])
	{
		Q_strncpy (ip, s, sizeof(ip));

		s = strchr (ip, ':');
		if (s)
			s[0] = '\0';

		Q_strncpy(ent->client->pers.current_ip, ip, sizeof(ent->client->pers.current_ip));
	}

	// name changes not allowed
	s = Info_ValueForKey (userinfo, "name");
	// log event if new name is different than old name
	if (strcmp(s, ent->client->pers.netname))
		WriteToLogfile(ent, va("Changed name to %s.\n", s));
	if (strlen( ent->client->pers.netname) < 1)
		Q_strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname));
	Info_SetValueForKey(userinfo, "name", ent->client->pers.netname);

	if (!ClientCanConnect(ent, userinfo))
	{
		s = Info_ValueForKey (userinfo, "rejmsg");
		gi.bprintf(PRINT_HIGH, "%s was kicked. Reason: %s\n", ent->client->pers.netname, s);
		stuffcmd(ent, "disconnect\n");
		return;
	}

	// set skin
	s = Info_ValueForKey (userinfo, "skin");
	playernum = ent-g_edicts-1;

	// combine name and skin into a configstring
	if (ptr->value || domination->value || ctf->value)
		AssignTeamSkin(ent, s);
	else if (!V_AssignClassSkin(ent, s))
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );

	// fov
	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		ent->client->ps.fov = 90;
	}
	else
	{
		ent->client->ps.fov = atoi(Info_ValueForKey(userinfo, "fov"));
		if (ent->client->ps.fov < 1)
			ent->client->ps.fov = 90;
		else if (ent->client->ps.fov > 160)
			ent->client->ps.fov = 160;
	}

	// handedness
	s = Info_ValueForKey (userinfo, "hand");
	if (strlen(s))
	{
		ent->client->pers.hand = atoi(s);
	}

	// save off the userinfo in case we want to check something later
	Q_strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo));
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again, but
loadgames will.
============
*/
qboolean ClientConnect (edict_t *ent, char *userinfo)
{
	char	ip[16];
	char	*value;
	
	if (debuginfo->value > 1)
		gi.dprintf("ClientConnect()\n");

	value = Info_ValueForKey (userinfo, "name");
	Q_strncpy (ent->client->pers.netname, value, sizeof(ent->client->pers.netname)-1);

	// update current ip
	value = Info_ValueForKey (userinfo, "ip");
	// update current ip, with less userinfo clobbering
	if (value[0])
	{
		Q_strncpy (ip, value, sizeof(ip)-1);

		value = strchr (ip, ':');
		if (value)
			value[0] = '\0';

		Q_strncpy(ent->client->pers.current_ip, ip, sizeof(ent->client->pers.current_ip)-1);
	}

	//Info_SetValueForKey(userinfo, "ip", value);

	if (!ClientCanConnect(ent, userinfo))
		return false;

	// check to see if they are on the banned IP list
	value = Info_ValueForKey (userinfo, "ip");
	if (SV_FilterPacket(value)) {
		Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
		return false;
	}

	// check for a spectator
	value = Info_ValueForKey (userinfo, "spectator");
	if (deathmatch->value && *value && strcmp(value, "0")) {
		int i, numspec;

		if (*spectator_password->string && 
			Q_strcmp(spectator_password->string, "none") && 
			Q_strcmp(spectator_password->string, value)) {
			Info_SetValueForKey(userinfo, "rejmsg", "Spectator password required or incorrect.");
			return false;
		}

		// count spectators
		for (i = numspec = 0; i < maxclients->value; i++)
			if (g_edicts[i+1].inuse && g_edicts[i+1].client->pers.spectator)
				numspec++;

		if (numspec >= maxspectators->value) {
			Info_SetValueForKey(userinfo, "rejmsg", "Server spectator limit is full.");
			return false;
		}
	} else {
		// check for a password
		value = Info_ValueForKey (userinfo, "password");
		if (*password->string && Q_strcmp(password->string, "none") && 
			Q_strcmp(password->string, value)) {
			Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
			return false;
		}
	}

	// they can connect
	ent->client = game.clients + (ent - g_edicts - 1);
/*	if (ent->client->disconnect_time + 1 > level.time) {
		Info_SetValueForKey(userinfo, "rejmsg", "Too fast connection, please try again.");
		return false;
	}*/

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == false)
	{
		// clear the respawning variables
		InitClientResp (ent->client);
		if (!game.autosaved || !ent->client->pers.weapon)
			InitClientPersistant (ent->client);

		//K03 Begin
		if (SPREE_WAR == true && ent && ent == SPREE_DUDE)
		{
			SPREE_WAR = false;
			SPREE_DUDE = NULL;
		}
		//K03 End
	}

	ClientUserinfoChanged (ent, userinfo);
//GHz START
	value = Info_ValueForKey (userinfo, "ip");
	if (ent && game.maxclients > 1)
	{
		gi.dprintf ("%s@%s connected at %s on %s\n", ent->client->pers.netname, value, CURRENT_TIME, CURRENT_DATE);
		WriteToLogfile(ent, "Connected to server.\n");
	}
//GHz END

//	ent->client->disconnect_time = -1;
	assert(ent != NULL);
	ent->svflags = 0;// make sure we start with known default
	//ent->num_lasers = 0;
	ent->client->pers.connected = true;
//	initmenu(ent);
	return true;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void ClientDisconnect (edict_t *ent)
{
	int		i;
	edict_t *player;
 	int		playernum;

	if (debuginfo->value > 1)
		gi.dprintf("ClientDisconnect()\n");

	if (!ent->client)
		return;

	KillMyVote (ent);

	dmgListCleanup(ent, true);
	V_ResetPlayerState(ent);

	// remove this player from allies' list
	if (allies->value && !ctf->value && !domination->value && !ptr->value)
	{
		NotifyAllies(ent, 1, va("%s disconnected\n", ent->client->pers.netname));
		RemoveAlly(ent, NULL);	
	}

	// remove from list of players waiting to spawn
	if (INVASION_OTHERSPAWNS_REMOVED)
		INV_RemoveSpawnQue(ent);

	// reset gds status for this player
	//ent->client->gds_downloading = false;
	//ent->client->gds_complete = true;

	if (ent->teamnum && !G_IsSpectator(ent))
	{
		gi.bprintf(PRINT_HIGH, "Waiting for player(s) to re-join...\n");
		AddJoinedQueue(ent);
	}
//	else
//		gi.dprintf("%s %s teamnum %d\n", ent->client->pers.netname, G_IsSpectator(ent)?"y":"n", ent->teamnum);

	average_player_level = AveragePlayerLevel();//GHz

	// make sure there are no mini-bosses on spree war status
	if (deathmatch->value)
	{
		for (i = 1; i <= maxclients->value; i++) {
			player = &g_edicts[i];
			if (!player->inuse)
				continue;
			if (player->solid == SOLID_NOT)
				continue;
			if (IsNewbieBasher(player) && SPREE_WAR && (SPREE_DUDE == player))
			{
				SPREE_WAR = false;
				SPREE_DUDE = NULL;
			}
		}
	}

	//K03 Begin -- Save users info
	ent->myskills.respawns = ent->client->pers.combat_changed;//4.5 save preferences when you disconnect

	if (SPREE_WAR == true && (ent == SPREE_DUDE))
	{
		SPREE_WAR = false;
		SPREE_DUDE = NULL;
	}

	//ent->myskills.inuse = 0;
	SaveCharacter(ent);

	//GHz: Reset their skills_t to prevent any possibility of duping
	if(!gds->value)		memset(&ent->myskills,0,sizeof(skills_t));
	//K03 End

	gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/eject.wav"), 1, ATTN_NORM, 0);
	gi.bprintf (PRINT_HIGH, "%s disconnected at %s on %s\n", ent->client->pers.netname, CURRENT_TIME, CURRENT_DATE);

	if (G_EntExists(ent))
		WriteToLogfile(ent, "Logged out.\n");
	else
		WriteToLogfile(ent, "Disconnected from server.\n");

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGOUT);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.unlinkentity (ent);
	ent->s.modelindex = 0;
	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->classname = "disconnected";
	ent->client->pers.connected = false;

	playernum = ent-g_edicts-1;
	gi.configstring (CS_PLAYERSKINS+playernum, "");
}

edict_t	*pm_passent;

// pmove doesn't need to know about passent and contentmask
trace_t	PM_trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
//GHz START
	// allows non-standard bbox sizes to be used by pmove()
	if (pm_passent->mtype)
	{
		VectorCopy(pm_passent->mins, mins);
		VectorCopy(pm_passent->maxs, maxs);
	}
//GHz END
	if (pm_passent->health > 0)
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

unsigned CheckBlock (void *b, int c)
{
	int	v,i;
	v = 0;
	for (i=0 ; i<c ; i++)
		v+= ((byte *)b)[i];
	return v;
}
void PrintPmove (pmove_t *pm)
{
	unsigned	c1, c2;

	c1 = CheckBlock (&pm->s, sizeof(pm->s));
	c2 = CheckBlock (&pm->cmd, sizeof(pm->cmd));
	Com_Printf ("sv %3i:%i %i\n", pm->cmd.impulse, c1, c2);
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/

void SpawnExtra(vec3_t position,char *classname);

void Get_Position ( edict_t *ent, vec3_t position )
{
	float yaw,pitch;

	yaw = ent->s.angles[YAW];
	pitch = ent->s.angles[PITCH];

	yaw = yaw * M_PI * 2 / 360;
	pitch = pitch * M_PI * 2 / 360;

	position[0] = cos(yaw) * cos(pitch);
	position[1] = sin(yaw) * cos(pitch);
	position[2] = -sin(pitch);
}

void RechargeAbilities (edict_t *ent)
{
	if (!G_EntIsAlive(ent))
		return;

	if (!(level.framenum % 10) && (level.time > ent->client->charge_regentime))
	{
		// 3.5 add all abilities here that must recharge

		// beam ability
		if (!ent->myskills.abilities[BEAM].disable 
			&& (ent->myskills.abilities[BEAM].current_level > 0)
			&& !ent->client->firebeam)
		{
			if (ent->myskills.abilities[BEAM].charge < 100)
			{
				ent->myskills.abilities[BEAM].charge += 10;
				ent->client->charge_time = level.time + 1.0; // show charge until we are full

				if (ent->myskills.abilities[BEAM].charge > 100)
					ent->myskills.abilities[BEAM].charge = 100;
			}
		}

		// shield ability
		if (!ent->myskills.abilities[SHIELD].disable 
			&& (ent->myskills.abilities[SHIELD].current_level > 0)
			&& !ent->shield)
		{
			if (ent->myskills.abilities[SHIELD].charge < SHIELD_MAX_CHARGE)
			{
				ent->myskills.abilities[SHIELD].charge += SHIELD_CHARGE_RATE;
				ent->client->charge_time = level.time + 1.0; // show charge until we are full
			}

			if (ent->myskills.abilities[SHIELD].charge > SHIELD_MAX_CHARGE)
				ent->myskills.abilities[SHIELD].charge = SHIELD_MAX_CHARGE;
		}
		
		// berserker sprint
		if ((ent->mtype == MORPH_BERSERK) && !ent->superspeed)
		{
			if (ent->myskills.abilities[BERSERK].charge < SPRINT_MAX_CHARGE)
			{
				ent->myskills.abilities[BERSERK].charge += SPRINT_CHARGE_RATE;
				ent->client->charge_time = level.time + 1.0; // show charge until we are full
			}

			if (ent->myskills.abilities[BERSERK].charge > SPRINT_MAX_CHARGE)
				ent->myskills.abilities[BERSERK].charge = SPRINT_MAX_CHARGE;
		}

		// stop showing charge on hud
		if (ent->client->charge_index && (level.time > ent->client->charge_time))
			ent->client->charge_index = 0;
	}
}

//K03 Begin
void Vampire_Think(edict_t *self);
void DeflectProjectiles (edict_t *self, float chance, qboolean in_front);
void DrawNearbyGrid(edict_t *ent);
void DrawChildLinks (edict_t *ent);


void ClientThinkstuff(edict_t *ent)
{
	int			health_factor;//K03
	int			max_armor;	// 3.5 max armor client can hold
	int			*armor;		// 3.5 pointer to client armor
	float		nextRegen;

	if (ent->client->showGridDebug > 0)
	{
		DrawNearbyGrid(ent);
		if (ent->client->showGridDebug >= 2)
			DrawChildLinks(ent);
	}

	//DrawPath();
	
	// 3.5 don't stuff player commands all at once, or they will overflow
	if (!(level.framenum%10))
		StuffPlayerCmds(ent);

	if (ent->cocoon_factor > 0 && level.time > ent->cocoon_time - 5)
	{
		if (!(level.framenum % 10))
			gi.cprintf(ent, PRINT_HIGH, "Cocoon bonus wears off in %.0f second(s)\n", (double)ent->cocoon_time - (double)level.time);

		if (level.time >= ent->cocoon_time)
		{
			gi.cprintf(ent, PRINT_HIGH, "Cocoon bonus wore off\n");
			ent->cocoon_time = 0;
			ent->cocoon_factor = 0;
		}
	}

	//4.2 are they in a wormhole? they can't stay in there forever!
	if (ent->flags & FL_WORMHOLE)
	{
		if (level.time == ent->client->wormhole_time - 10)
			gi.cprintf(ent, PRINT_HIGH, "You have 10 seconds to exit the wormhole.\n");
		if (level.time == ent->client->wormhole_time - 5)
			gi.cprintf(ent, PRINT_HIGH, "You have 5 seconds to exit the wormhole.\n");
		if (level.time == ent->client->wormhole_time - 4)
			gi.cprintf(ent, PRINT_HIGH, "You have 4 seconds to exit the wormhole.\n");
		if (level.time == ent->client->wormhole_time - 3)
			gi.cprintf(ent, PRINT_HIGH, "You have 3 seconds to exit the wormhole.\n");
		if (level.time == ent->client->wormhole_time - 2)
			gi.cprintf(ent, PRINT_HIGH, "You have 2 seconds to exit the wormhole.\n");
		if (level.time == ent->client->wormhole_time - 1)
			gi.cprintf(ent, PRINT_HIGH, "You have 1 second to exit the wormhole.\n");

		// remove wormhole flag and force respawn
		if (level.time == ent->client->wormhole_time)
		{
			ent->flags &= ~FL_WORMHOLE;
			ent->svflags &= ~SVF_NOCLIENT;
			ent->movetype = MOVETYPE_WALK;
			Teleport_them(ent);
		}
	}
	
	if (SPREE_WAR && (ent->myskills.streak >= SPREE_WARS_START) && (SPREE_DUDE == ent))
	{
		if (level.time < SPREE_TIME + 120)
		{
			if (SPREE_TIME + 90 == level.time)
				gi.cprintf(ent, PRINT_HIGH, "Hurry up! Only 30 seconds remaining...\n");
			else if (SPREE_TIME + 110 == level.time)
				gi.bprintf(PRINT_HIGH, "HURRY! War ends in 10 seconds. %s must die!\n", ent->client->pers.netname);
		}
		else
		{
			gi.bprintf(PRINT_HIGH, "%s's spree war ran out of time :(\n", ent->client->pers.netname);
			SPREE_WAR = false;
			SPREE_DUDE = NULL;
			ent->myskills.streak = 0;
		}
	}

	if (ent->client->allying && (level.time >= ent->client->ally_time+ALLY_WAIT_TIMEOUT))
	{
		gi.cprintf(ent->client->allytarget, PRINT_HIGH, "Alliance request timed out.\n");
		AbortAllyWait(ent->client->allytarget);
		gi.cprintf(ent, PRINT_HIGH, "Alliance request timed out.\n");
		AbortAllyWait(ent);
	}

	// 4.0 we shouldn't be here if we're dead
	if ((ent->health < 1) || (ent->deadflag == DEAD_DEAD) || G_IsSpectator(ent) || (ent->flags & FL_WORMHOLE))
		return;

	// 4.2 recharge blaster ammo
	// (apple)
	// Blaster should be affected by player's max ammo.
	if (!(level.framenum%2) && !(ent->client->buttons & BUTTON_ATTACK) 
		&& (ent->monsterinfo.lefty < (25 + 12.5*ent->myskills.abilities[MAX_AMMO].level)))
		ent->monsterinfo.lefty++;

	if (ent->shield && (level.time > ent->shield_activate_time))
	{
		if (ent->myskills.abilities[SHIELD].charge < SHIELD_COST)
		{
			ent->shield = 0;
		}
		else
		{
			//Talent: Repel
			int		talentLevel;
			if ((talentLevel = getTalentLevel(ent, TALENT_REPEL)) > 0)
				DeflectProjectiles(ent, ((float)0.1*talentLevel), true);

			ent->myskills.abilities[SHIELD].charge -= SHIELD_COST;
			ent->client->charge_index = SHIELD + 1;
			ent->client->charge_time = level.time + 1.0;
			ent->client->ability_delay = level.time + FRAMETIME;		// can't use abilities
			ent->monsterinfo.attack_finished = level.time + FRAMETIME;	// morphed players can't attack
		}
	}

	RechargeAbilities(ent);
	CursedPlayer(ent);
	AllyID(ent);

	//Talent: Basic Ammo Regeneration
	if(ent->client && getTalentSlot(ent, TALENT_BASIC_AMMO_REGEN) != -1)
	{
		talent_t *talent = &ent->myskills.talents.talent[getTalentSlot(ent, TALENT_BASIC_AMMO_REGEN)];
        
		if(talent->upgradeLevel > 0 && (talent->delay < level.time) 
			&& !(ent->client->buttons & BUTTON_ATTACK)) // don't regen ammo while firing
		{
			//Give them some ammo
			switch(ent->client->ammo_index)
			{
			case 24:	V_GiveAmmoClip(ent, 1.0, AMMO_ROCKETS);		break;	//rocket_index
			case 21:	V_GiveAmmoClip(ent, 1.0, AMMO_BULLETS);		break;	//bullet_index
			case 22:	V_GiveAmmoClip(ent, 1.0, AMMO_CELLS);		break;	//cell_index
			case 23:	V_GiveAmmoClip(ent, 1.0, AMMO_GRENADES);	break;	//grenade_index
			case 20:	V_GiveAmmoClip(ent, 1.0, AMMO_SHELLS);		break;	//shell_index
			case 25:	V_GiveAmmoClip(ent, 1.0, AMMO_SLUGS);		break;	//slug_index
			default: //do nothing
				break;
			}
			
			talent->delay = level.time + 30 - talent->upgradeLevel * 5;	//30 seconds - 5 seconds per upgrade
		}
	}

	if (ent->myskills.abilities[AMMO_REGEN].current_level > 0)
		Ammo_Regen(ent);
	
	if (!(level.framenum%50) && ent->myskills.abilities[POWER_REGEN].current_level > 0)
		Special_Regen(ent);

	//3.0 Mind absorb every x seconds
	if (ent->myskills.abilities[MIND_ABSORB].current_level > 0)
	{
		int cooldown = 50;

		//Talent: Mind Control
		if(getTalentSlot(ent, TALENT_IMP_MINDABSORB) != -1)
			cooldown -= 5 * getTalentLevel(ent, TALENT_IMP_MINDABSORB);

		if(!(level.framenum % cooldown))
			MindAbsorb(ent);
	}

	//4.1 Fury ability causes regeneration to occur every second.
	if (!(level.framenum % 10) && (ent->fury_time > level.time && ent->client))
	{
		if(G_EntIsAlive(ent) && !(ctf->value && ctf_enable_balanced_fc->value && HasFlag(ent)))
		{
			int maxHP = MAX_HEALTH(ent);
			int maxAP = MAX_ARMOR(ent);
			armor = &ent->client->pers.inventory[body_armor_index];
			float factor = FURY_INITIAL_REGEN + (FURY_ADDON_REGEN * ent->myskills.abilities[FURY].current_level);
			
			if (factor > FURY_MAX_REGEN)
				factor = FURY_MAX_REGEN;

			//Regenerate some health.
			if (ent->health < maxHP)
				ent->health_cache += maxHP * factor;
			//Regenerate some health.
			if (*armor < maxAP)
				ent->armor_cache += maxAP * factor;
		
		}
	//	else	ent->fury_time = 0.0;
	}

	//QW// Added these to passify VS2019. 
	// Testing will reveal if there is a real error.
	assert(ent != NULL);
	assert(ent->client != NULL);

	// regeneration tech
	if (ent->client->pers.inventory[regeneration_index])
	{
		edict_t *e;
		qboolean regenerate, regenNow=false;

		if (PM_PlayerHasMonster(ent))
			e = ent->owner;
		else
			e = ent;

		if (e->monsterinfo.regen_delay1 == level.framenum)
			regenNow = true;

		if (ent->myskills.level <= 5)
			regenerate = M_Regenerate(e, 100, 10, 1.0, true, true, false, &e->monsterinfo.regen_delay1);
		else if (ent->myskills.level <= 10)
			regenerate = M_Regenerate(e, 200, 10, 1.0, true, true, false, &e->monsterinfo.regen_delay1);
		else
			regenerate = M_Regenerate(e, 300, 10, 1.0, true, true, false, &e->monsterinfo.regen_delay1);

		if (regenNow && regenerate)
			gi.sound (ent, CHAN_ITEM, gi.soundindex("ctf/tech4.wav"), 1, ATTN_STATIC, 0);
	}

	// regenerate health
	if ((ent->health < ent->max_health) && (!ent->myskills.abilities[REGENERATION].disable) 
		&& (ent->deadflag != DEAD_DEAD) && (level.time > ent->client->healthregen_time)
		&& !(ctf->value && ctf_enable_balanced_fc->value && HasFlag(ent)))
	{
		//3.0 cursed players can't heal through regeneration
		if (que_findtype(ent->curses, NULL, CURSE) == NULL)
		{
			health_factor = 2*ent->myskills.abilities[REGENERATION].current_level;
			ent->health += health_factor;

			if (ent->health > ent->max_health)
				ent->health = ent->max_health;
			//stuffcmd(ent, va("\nplay %s\n", "items/n_health.wav"));
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/n_health.wav"), 1, ATTN_STATIC, 0);
			nextRegen = 10.0 / (float) ent->myskills.abilities[REGENERATION].current_level;
			if (nextRegen < 1)
				nextRegen = 1;

			ent->client->healthregen_time = level.time + nextRegen;
		}
	}

	//4.4 - max hp/sec capped to 50/sec (25 per 5 frames)
	V_HealthCache(ent, 50, 5);
	//4.4 - max armor/sec capped to 25/sec
	V_ArmorCache(ent, 25, 5);

	// 3.5 regenerate armor
	armor = &ent->client->pers.inventory[body_armor_index];
	max_armor = MAX_ARMOR(ent);

	if (!ent->myskills.abilities[ARMOR_REGEN].disable 
		&& (ent->myskills.abilities[ARMOR_REGEN].current_level > 0)
		&& G_EntIsAlive(ent) && (*armor < max_armor) 
		&& (level.time > ent->client->armorregen_time)
		&& !(ctf->value && ctf_enable_balanced_fc->value && HasFlag(ent))
		&& !que_findtype(ent->curses, NULL, CURSE)) // can't regen when cursed
	{
		health_factor = floattoint(0.5*ent->myskills.abilities[ARMOR_REGEN].current_level);
		//gi.dprintf("%d\n", health_factor);
		if (health_factor < 1)
			health_factor = 1;

		*armor += health_factor;
		if (*armor > max_armor)
			*armor = max_armor;

		nextRegen = 10.0 / (float) ent->myskills.abilities[ARMOR_REGEN].current_level;
		if (nextRegen < 1)
			nextRegen = 1;

		ent->client->armorregen_time = level.time + nextRegen;
	}

	if (ent->client->thrusting)
		ApplyThrust (ent);
	if (ent->sucking)
		ParasiteAttack(ent);
	if (ent->superspeed && ent->deadflag != DEAD_DEAD)
	{
		//3.0 Blessed players get a speed boost too
		que_t *slot = NULL;
		slot = que_findtype(ent->curses, NULL, BLESS);

		//eat cubes if the player isn't blessed
		if (slot == NULL)
		{
			// if we are not morphed, then just use cubes
			if (ent->mtype != MORPH_BERSERK)
				ent->client->pers.inventory[power_cube_index] -= SUPERSPEED_DRAIN_COST;
			else // berserker can sprint
			{
				// if superspeed is upgraded and we have insufficient charge, then use cubes
				if ((ent->myskills.abilities[SUPER_SPEED].current_level > 0) && (ent->myskills.abilities[BERSERK].charge < SPRINT_COST))
					ent->client->pers.inventory[power_cube_index] -= SUPERSPEED_DRAIN_COST;
				// otherwise use ability charge
				else
				{
					ent->client->charge_index = BERSERK + 1;
					ent->client->charge_time = level.time + 1.0;
					ent->myskills.abilities[BERSERK].charge -= SPRINT_COST;
				}
			}
		}
	}

	if ((ent->client->hook_state == HOOK_ON) && ent->client->hook)
		hook_service(ent->client->hook);

	if (!ent->myskills.abilities[VAMPIRE].disable)
		Vampire_Think(ent);

	if (ent->client->firebeam)
		brain_fire_beam(ent);

	if (ent->myskills.administrator == 11)
		ent->client->ping = GetRandom(300, 400);
	
	// cloak them
	if (!ent->myskills.abilities[CLOAK].disable) 
	{
		if ((ent->myskills.abilities[CLOAK].current_level > 0) && (ent->client->idle_frames >= 10) 
			&& !que_typeexists(ent->auras, 0) && (level.time > ent->client->ability_delay)
			&& !HasFlag(ent) && !V_HasSummons(ent))
		{
			//if (!ent->svflags & SVF_NOCLIENT)
			//	que_list(ent->auras);

			ent->svflags |= SVF_NOCLIENT;
			ent->client->cloaking = true;

			//FIXME: we really shouldnt be calling this every frame...
			VortexRemovePlayerSummonables(ent); // 3.75 no more cheap apps with cloak+laser/monster/etc
		}
		else if (ent->movetype != MOVETYPE_NOCLIP && !(ent->flags & FL_CHATPROTECT) 
			&& !(ent->flags & FL_COCOONED))// don't see player-monsters
		{
			ent->client->cloaking = false;
			ent->svflags &= ~SVF_NOCLIENT;
		}
		else
			ent->client->cloaking = false;
	}

	if ((!ent->myskills.abilities[PLAGUE].disable) && (ent->myskills.abilities[PLAGUE].current_level))
		PlagueCloudSpawn(ent);

	if (ent->flags & FL_CHATPROTECT)
		ent->client->ability_delay = level.time + 1; // can't use abilities in chat-protect
}

qboolean CanSuperSpeed (edict_t *ent)
{
	// can't superspeed while being hurt
	//if (ent->lasthurt + DAMAGE_ESCAPE_DELAY > level.time)
	//	return false;

	// bless
	if (que_findtype(ent->curses, NULL, BLESS))
		return true;

	// superspeed ability
	if (!ent->myskills.abilities[SUPER_SPEED].disable && ent->myskills.abilities[SUPER_SPEED].current_level > 0 
		&& ent->client->pers.inventory[power_cube_index] >= SUPERSPEED_DRAIN_COST)
		return true;

	// sprint
	if (ent->mtype == MORPH_BERSERK && ent->myskills.abilities[BERSERK].charge > SPRINT_COST)
		return true;

	return false;
}

void V_PlayerJump (edict_t *ent)
{
	ent->lastsound = level.framenum;

	// make player jumping noise
	if (!ent->mtype && (ent->client->pers.inventory[ITEM_INDEX(FindItem("Stealth Boots"))] < 1)
		&& (ent->myskills.abilities[CLOAK].disable || (ent->myskills.abilities[CLOAK].current_level < 1)))
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
	}

	if (ent->mtype == MORPH_MUTANT)
	{
		if (mutant_boost(ent))
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("mutant/mutsght1.wav"), 1, ATTN_NORM, 0);
	}
	else if (ent->mtype == MORPH_MEDIC)
	{
		gi.sound (ent, CHAN_VOICE, gi.soundindex ("medic/medsght1.wav"), 1, ATTN_NORM, 0);
	}
	else if (ent->mtype == MORPH_BERSERK)
	{
		gi.sound (ent, CHAN_VOICE, gi.soundindex ("berserk/sight.wav"), 1, ATTN_NORM, 0);
	}
	else if (ent->mtype == MORPH_BRAIN)
	{
		if (mutant_boost(ent))
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("brain/brnsght1.wav"), 1, ATTN_NORM, 0);
	}
	else if ((ent->mtype == M_MYPARASITE) || (ent->mtype == MORPH_BRAIN))
	{
		ent->velocity[0] *= 1.66;
		ent->velocity[1] *= 1.66;
		//ent->velocity[2] *= 1.66;
		// FIXME: we probably should do this for any ability that can get the player up in the air
		ent->monsterinfo.jumpup = 1; // player has jumped, cleared on touch-down
	}
}

qboolean CanDoubleJump (edict_t *ent, usercmd_t *ucmd)
{
	if (ent->client)
		return (!ent->waterlevel && !ent->groundentity && !(ent->v_flags & SFLG_DOUBLEJUMP) && !HasFlag(ent)
			&& (ucmd->upmove > 0) && !ent->client->jump && !ent->myskills.abilities[DOUBLE_JUMP].disable 
			&& (ent->myskills.abilities[DOUBLE_JUMP].current_level > 0));
	else if (PM_MonsterHasPilot(ent))
		return (!ent->waterlevel && !ent->groundentity && !(ent->v_flags & SFLG_DOUBLEJUMP) && !HasFlag(ent->activator)
			&& (ucmd->upmove > 0) && !ent->activator->client->jump && !ent->activator->myskills.abilities[DOUBLE_JUMP].disable 
			&& (ent->activator->myskills.abilities[DOUBLE_JUMP].current_level > 0));
	else 
		return false;
}

//K03 End
void RunParasiteFrames (edict_t *ent, usercmd_t *ucmd);
void RunBrainFrames (edict_t *ent, usercmd_t *ucmd);
void V_AutoAim (edict_t *player);
void UpdateMirroredEntities (edict_t *ent);
void LeapAttack (edict_t *ent);

void ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t	*client;
	edict_t	*other;
	int		i, j;
	byte	impulse;
	pmove_t	pm;
	float modifier;

	//int		fire_last = 18;		//QW// Removed this until I figure it out.
	que_t	*curse=NULL;
	int		viewheight = ent->viewheight;

	impulse = ucmd->impulse;
// GHz START
	// if we're a morphed player, then save the current viewheight
	// the player will be locked into this viewheight while morphed
	if (ent->mtype)
		viewheight = ent->viewheight;
//GHz END
	if(impulse == 1) gi.bprintf(PRINT_HIGH,"%f\n",ent->s.origin[2]);

	//K03 Begin
	if (ent->client->resp.spectator != true)
	{
		// need this for sniper mode calculations
		//if (ent->myskills.weapons[WEAPON_RAILGUN].mods[1].current_level > 0)
		//{
		//	if (ent->myskills.weapons[WEAPON_RAILGUN].mods[1].current_level > 9)
		//		fire_last = 12;
		//	else if (ent->myskills.weapons[WEAPON_RAILGUN].mods[1].current_level > 8)
		//		fire_last = 13;
		//	else if (ent->myskills.weapons[WEAPON_RAILGUN].mods[1].current_level > 6)
		//		fire_last = 14;
		//	else if (ent->myskills.weapons[WEAPON_RAILGUN].mods[1].current_level > 4)
		//		fire_last = 15;
		//	else if (ent->myskills.weapons[WEAPON_RAILGUN].mods[1].current_level > 2)
		//		fire_last = 16;
		//	else fire_last = 17;
		//}

		// assault cannon slows you down
		if(ent->client->pers.weapon && (ent->client->pers.weapon->weaponthink == Weapon_Chaingun) 
			&& (ent->client->weaponstate == WEAPON_FIRING) && (ent->client->weapon_mode))
		{
			ucmd->forwardmove *= 0.33;
			ucmd->sidemove *= 0.33;
			ucmd->upmove *= 0.33;
		}
		// sniper mode slows you down
		if (ent->client->snipertime >= level.time)
		{
			ucmd->forwardmove *= 0.33;
			ucmd->sidemove *= 0.33;
			ucmd->upmove *= 0.33;
		}

		curse = que_findtype(ent->curses, curse, AURA_HOLYFREEZE);
		// are we affected by the holy freeze aura?
		if (curse)
		{
			modifier = 1 / (1 + 0.1 * curse->ent->owner->myskills.abilities[HOLY_FREEZE].current_level);
			if (modifier < 0.25) modifier = 0.25;
			//gi.dprintf("holyfreeze modifier = %.2f\n", modifier);
			ucmd->forwardmove *= modifier;
			ucmd->sidemove *= modifier;
			ucmd->upmove *= modifier;
		}

		//Talent: Frost Nova
		//4.2 Water Totem
		if(ent->chill_time > level.time)
		{
			modifier = 1 / (1 + CHILL_DEFAULT_BASE + CHILL_DEFAULT_ADDON * ent->chill_level);
			if (modifier < 0.25) modifier = 0.25;
			//gi.dprintf("chill modifier = %.2f\n", modifier);
			ucmd->forwardmove *= modifier;
			ucmd->sidemove *= modifier;
			ucmd->upmove *= modifier;
		}
		
		//if (!ent->client->resp.spectator && VectorLength(ent->velocity) > 10)
		//	gi.dprintf("vel = %.0f\n", VectorLength(ent->velocity));

		//4.2 caltrops
		if (ent->slowed_time > level.time)
		{
			ucmd->forwardmove *= ent->slowed_factor;
			ucmd->sidemove *= ent->slowed_factor;
			ucmd->upmove *= ent->slowed_factor;
		}


		// 3.5 weaken slows down target
		if ((curse = que_findtype(ent->curses, NULL, WEAKEN)) != NULL)
		{
			modifier = 1 / (1 + WEAKEN_SLOW_BASE + WEAKEN_SLOW_BONUS 
				* curse->ent->owner->myskills.abilities[WEAKEN].current_level);

			ucmd->forwardmove *= modifier;
			ucmd->sidemove *= modifier;
			ucmd->upmove *= modifier;
		}

		//GHz: Keep us still and don't allow shooting
		if (ent->holdtime && ent->holdtime > level.time)
		{
			ucmd->forwardmove = 0;
			ucmd->sidemove = 0;
			ucmd->upmove = 0;

			if (ent->client->buttons & BUTTON_ATTACK)
				ent->client->buttons &= ~BUTTON_ATTACK;

			if (ucmd->buttons & BUTTON_ATTACK)
				ucmd->buttons &= ~BUTTON_ATTACK;
		}
			
		//End of slow move
		if(que_typeexists(ent->curses, CURSE_FROZEN))
		{
			ent->client->ps.pmove.pm_type = PM_DEAD;
			if (ent->client->buttons & BUTTON_ATTACK)
				ent->client->buttons &= ~BUTTON_ATTACK;
			return;
		}
	}

	//K03 End
	//get JumpMax

//--------------------------------------
	level.current_entity = ent;
	client = ent->client;

	if (level.intermissiontime)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
		if (level.time > level.intermissiontime + 5.0 
			&& (ucmd->buttons & BUTTON_ANY) )
			level.exitintermission = true;
		return;
	}

	

	pm_passent = ent;

	if (ent->client->chase_target) {
		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

	} else {

		if (ent->lockon == 1 && ent->enemy)
		{
			client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
			client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
			client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);
		}

		// set up for pmove
		memset (&pm, 0, sizeof(pm));
		
		if (ent->movetype == MOVETYPE_NOCLIP)
			client->ps.pmove.pm_type = PM_SPECTATOR;
		else if (ent->deadflag)
			client->ps.pmove.pm_type = PM_DEAD;
		else
			client->ps.pmove.pm_type = PM_NORMAL;
		//K03 Begin
		if (client->hook_state == HOOK_ON)
			client->ps.pmove.gravity = 0;
		else
			client->ps.pmove.gravity = sv_gravity->value;
		//K03 End

		//3.0 matrix jump
		if (ent->v_flags & SFLG_MATRIXJUMP)
			client->ps.pmove.gravity = sv_gravity->value / MJUMP_GRAVITY_MULT;
		else if ((ent->mtype == MORPH_FLYER) && (ent->deadflag != DEAD_DEAD)) // flyers are not affected by gravity
			client->ps.pmove.gravity = 0;
		else
			client->ps.pmove.gravity = sv_gravity->value;
		//pm.s = client->ps.pmove;
		//end doomie

		// reset jump flag
		if (!ucmd->upmove)
			ent->client->jump = false;

		// double jump
		if (CanDoubleJump(ent, ucmd))
		{
			if (ent->velocity[2] < 256)
				ent->velocity[2] = 256;
			else
				ent->velocity[2] += 256;
			ent->v_flags |= SFLG_DOUBLEJUMP;
			V_PlayerJump(ent);
		}
		
		pm.s = client->ps.pmove;

		for (i=0 ; i<3 ; i++)
		{
			pm.s.origin[i] = ent->s.origin[i]*8;
			pm.s.velocity[i] = ent->velocity[i]*8;
		}

		if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
			pm.snapinitial = true;

		pm.cmd = *ucmd;

		pm.trace = PM_trace;	// adds default parms
		

		pm.pointcontents = gi.pointcontents;

		// perform a pmove
		gi.Pmove (&pm);
// GHz START
		// if this is a morphed player, restore saved viewheight
		// this locks them into that viewheight
		if (ent->mtype)
			pm.viewheight = viewheight;
//GHz END

		// save results of pmove
		client->ps.pmove = pm.s;
		client->old_pmove = pm.s;

		for (i=0 ; i<3 ; i++)
		{
			ent->s.origin[i] = pm.s.origin[i]*0.125;
			ent->velocity[i] = pm.s.velocity[i]*0.125;
		}

		VectorCopy (pm.mins, ent->mins);
		VectorCopy (pm.maxs, ent->maxs);

		if (!(ent->lockon == 1 && ent->enemy)){
			client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
			client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
			client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);
		}

		//K03 Begin
		//4.07 can't superspeed while being hurt
		if (ent->superspeed)
		{
			
			if (!CanSuperSpeed(ent))
			{
				ent->superspeed = false;				
			}
			else if (level.time > ent->lasthurt + DAMAGE_ESCAPE_DELAY)
			{
				ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
				pm.s = client->ps.pmove;

				for (i=0 ; i<3 ; i++)
				{
					pm.s.origin[i] = ent->s.origin[i]*8;
					//pm.s.velocity[i] = ent->velocity[i]*8;
				}
				pm.s.velocity[0] = ent->velocity[0]*8;
				pm.s.velocity[1] = ent->velocity[1]*8;

				if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
				{
					pm.snapinitial = true;
			//		gi.dprintf ("pmove changed!\n");
				}

				pm.cmd = *ucmd;

				pm.trace = PM_trace;	// adds default parms
				
				pm.pointcontents = gi.pointcontents;

				// perform a pmove
				gi.Pmove (&pm);
				
// GHz START
				// if this is a morphed player, restore saved viewheight
				// this locks them into that viewheight
				if (ent->mtype)
					pm.viewheight = viewheight;
//GHz END
				// save results of pmove
				client->ps.pmove = pm.s;
				client->old_pmove = pm.s;

				for (i=0 ; i<3 ; i++)
				{
					ent->s.origin[i] = pm.s.origin[i]*0.125;
					//ent->velocity[i] = pm.s.velocity[i]*0.125;
				}
				ent->velocity[0] = pm.s.velocity[0]*0.125;
				ent->velocity[1] = pm.s.velocity[1]*0.125;

				VectorCopy (pm.mins, ent->mins);
				VectorCopy (pm.maxs, ent->maxs);

				client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
				client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
				client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

				ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			}
		}
		//K03 End

		if (/*ent->groundentity && !pm.groundentity &&*/ (pm.cmd.upmove >= 10) /*&& (pm.waterlevel == 0)*/)
		{
			if (((ent->mtype == MORPH_BRAIN || ent->mtype == MORPH_MUTANT) && pm.waterlevel > 0) || (ent->groundentity && !pm.groundentity))
			{
				V_PlayerJump(ent);
				ent->client->jump = true;
			}
		}

		ent->viewheight = pm.viewheight;
		ent->waterlevel = pm.waterlevel;
		ent->watertype = pm.watertype;
		ent->groundentity = pm.groundentity;

		if (pm.groundentity)
			ent->groundentity_linkcount = pm.groundentity->linkcount;

		if (ent->deadflag)
		{
			client->ps.viewangles[ROLL] = 40;
			client->ps.viewangles[PITCH] = -15;
			client->ps.viewangles[YAW] = client->killer_yaw;
		}
		else
		{	
			VectorCopy (pm.viewangles, client->v_angle);
			VectorCopy (pm.viewangles, client->ps.viewangles);
		}

		gi.linkentity (ent);

		if (ent->movetype != MOVETYPE_NOCLIP)
			G_TouchTriggers (ent);

		// touch other objects
		for (i=0 ; i<pm.numtouch ; i++)
		{
			other = pm.touchents[i];
			for (j=0 ; j<i ; j++)
				if (pm.touchents[j] == other)
					break;
			if (j != i)
				continue;	// duplicated
			if (!other->touch)
				continue;
			other->touch (other, ent, NULL, NULL);
		}

					//3.0 begin doomie
	
			/*************************************************************************
			The following is a flag to replace pm.waterlevel
				there is no point in calling gi.Pmove() more than how often iD
				calls for it, if we can avoid doing so. -doomie
			**************************************************************************/

			switch (pm.waterlevel)
			{
			case 0:	//not in water
				{
					//Make sure all water flags are off
					if (ent->v_flags & (SFLG_TOUCHING_WATER))
					{
						if (ent->v_flags & SFLG_UNDERWATER)
							ent->v_flags ^= SFLG_UNDERWATER;
						if (ent->v_flags & SFLG_PARTIAL_INWATER)
							ent->v_flags ^= SFLG_PARTIAL_INWATER;
					}
				}
				break;
			case 1:	//standing or swimming
				{
					//Turn off underwater flag
					if (ent->v_flags & SFLG_UNDERWATER)
						ent->v_flags ^= SFLG_UNDERWATER;
					//Turn on partial underwater flag
					if (!(ent->v_flags & SFLG_PARTIAL_INWATER))
						ent->v_flags ^= SFLG_PARTIAL_INWATER;
				}
				break;
			default: //underwater
				{
					//Turn on underwater flag
					if (!(ent->v_flags & SFLG_UNDERWATER))
						ent->v_flags ^= SFLG_UNDERWATER;
					//Turn off partial underwater flag
					if (ent->v_flags & SFLG_PARTIAL_INWATER)
						ent->v_flags ^= SFLG_PARTIAL_INWATER;
				}
				break;
			}

			if ((ent->v_flags & SFLG_MATRIXJUMP) && (ent->velocity[2] < 10))
				ent->v_flags ^= SFLG_MATRIXJUMP;

	//end doomie (3.0)

	}
	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;

	// fire weapon from final position if needed
	if (client->latched_buttons & BUTTON_ATTACK)
	{
		if (client->resp.spectator) {

			client->latched_buttons = 0;

			//K03 Begin
			client->chasecam_mode = (client->chasecam_mode == 1) ? 0 : 1;
			gi.cprintf(ent, PRINT_HIGH, "CHASE CAM MODE: %s\n", (client->chasecam_mode == 1) ? "EYE CAM" : "FOLLOW CAM");
			if (!client->chase_target)
				GetChaseTarget(ent);

		} else if (!client->weapon_thunk) {
			client->weapon_thunk = true;
			Think_Weapon (ent);
		}
	}

	// double jump reset flag
	if (ent->groundentity)
	{
		ent->v_flags &= ~SFLG_DOUBLEJUMP;

		// the player (parasite) has completed his jump, clear the flag
		ent->monsterinfo.jumpup = 0;

		//Talent: Leap Attack - player has touched down
		LeapAttack(ent);
	}

	if (client->resp.spectator) {
		if (ucmd->upmove >= 10) {
			if (!(client->ps.pmove.pm_flags & PMF_JUMP_HELD)) {
				client->ps.pmove.pm_flags |= PMF_JUMP_HELD;
				if (client->chase_target)
					ChaseNext(ent);
				else
					GetChaseTarget(ent);
			}
		} else
			client->ps.pmove.pm_flags &= ~PMF_JUMP_HELD;
	}

	/*
	// update chase cam if being followed
	for (i = 1; i < globals.num_edicts; i++) {
		other = g_edicts + i;
		if (other->inuse && ent->client->chase_target == other)
			UpdateChaseCam(ent);
	}*/
	UpdateChaseCam(ent);

	//if (ent->client->chase_target)

	if (ent->lockon == 1)
		V_AutoAim(ent);

		//LockOnTarget(ent);

	//K03 Begin
	if ((client->hook_state == HOOK_ON) && (VectorLength(ent->velocity) < 10)) {
        client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
    } else {
        client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
    }
	//K03 End

	//3.0 new trading
	//If this player isn't showing a menu any more, cancel the trade
	if (ent->trade_with && !ent->client->menustorage.menu_active)
	{
		//alert both players
		gi.cprintf(ent, PRINT_HIGH, "%s has stopped the trade.\n", ent->myskills.player_name);
		gi.cprintf(ent->trade_with, PRINT_HIGH, "%s has stopped the trade.\n", ent->myskills.player_name);

		//Clear the trade pointers
		for (i = 0; i < 3; ++i)
		{
			ent->trade_item[i] = NULL;
			ent->trade_with->trade_item[i] = NULL;
		}

		//cancel the trade (trade_with)
		closemenu(ent->trade_with);
		ent->trade_with->client->trade_accepted = false;
		ent->trade_with->client->trade_final = false;
		ent->trade_with->client->trading = false;
		ent->trade_with->trade_with = NULL;

		//cancel the trade (ent)
        ent->trade_with = NULL;
		ent->client->trade_accepted = false;
		ent->client->trade_final = false;
		ent->client->trading = false;
	}
	//3.0 end

	boss_update(ent, ucmd);
	RunParasiteFrames(ent, ucmd);
	RunCacodemonFrames(ent, ucmd);
	//RunTankFrames(ent, ucmd);
	RunBrainFrames(ent, ucmd);
	RunFlyerFrames(ent, ucmd);
	RunMutantFrames(ent, ucmd);
	RunMedicFrames(ent, ucmd);
	RunBerserkFrames(ent, ucmd);
	EatCorpses(ent);
	UpdateMirroredEntities(ent);
}


/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
void ClientBeginServerFrame (edict_t *ent)
{
	int			frames;
	gclient_t	*client;

	if (level.intermissiontime)
		return;

	//GHz START
	if (G_EntExists(ent) && !(level.framenum%10))
	{
		//3.0 reduce their mute list
		int i;
        for(i = 0; i < MAX_CLIENTS; ++i)
		{
			if (!ent->myskills.mutelist[i].player || ent->myskills.mutelist[i].time < 1)
				continue;
			ent->myskills.mutelist[i].time -= 1;
			if (ent->myskills.mutelist[i].time < 1)
				ent->myskills.mutelist[i].player = NULL;

		}
		//3.0 end
		ent->myskills.playingtime++;		
	}

	// idle frame counter
	if ((ent->velocity[0] != 0) || (ent->velocity[1] != 0) || (ent->velocity[2] != 0))
	{
		ent->client->still_frames = 0;

		//Talent: Improved cloak
		//Check to see if the player is cloaked AND crouching
		if(ent->client->cloaking && ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			

			//Make sure they have the talent
			int talentLevel = getTalentLevel(ent, TALENT_IMP_CLOAK);
			
			if(talentLevel > 0)
			{
				int cloak_cubecost = 6 - talentLevel;

				//During night time, improved cloak takes 1/3 the cubes (rounded up).
				if(!level.daytime)	cloak_cubecost = ceil((double)cloak_cubecost / 3.0);
                
				//Take the cubes away
				ent->client->pers.inventory[power_cube_index] -= cloak_cubecost;

				//If the player runs out of cubes, they will uncloak!
				if(ent->client->pers.inventory[power_cube_index] <= 0)
				{
					ent->client->idle_frames = 0;
					ent->client->cloaking = false;
					ent->client->pers.inventory[power_cube_index] = 0;
				}
			}
			else
			{
				ent->client->cloaking = false;
				ent->client->idle_frames = 0;
			}
		}
		else
		{
			ent->client->cloaking = false;
			ent->client->idle_frames = 0;
		}
	}
	else if (ent->client->snipertime > 0 && ent->client->idle_frames > 15)
	{
		ent->client->idle_frames = 15;
	}
	else if ((ent->health > 0) && !G_IsSpectator(ent) && (ent->movetype != MOVETYPE_NONE))
	{
		ent->client->idle_frames++;
		ent->client->still_frames++;
	}

	// chat-protect ends when you move or fire
	if ((ent->client->idle_frames < CHAT_PROTECT_FRAMES) && (ent->flags & FL_CHATPROTECT))
	{
		ent->flags &= ~FL_CHATPROTECT;
		if (!PM_PlayerHasMonster(ent))
			ent->svflags &= ~SVF_NOCLIENT;//4.5

		// if it's CTF or Domination mode, or FFA mode and the player is hostile against players, teleport them
		if (ctf->value || domination->value || (ffa->value && V_MatchPlayerPrefs(ent, -1, 1)))
			Teleport_them(ent);
	}
    
	if (level.time > pregame_time->value)
	{
		if (ptr->value || domination->value || ctf->value)
			frames = PTR_MAX_IDLE_FRAMES;
		else
			frames = MAX_IDLE_FRAMES;

		if (!ent->myskills.administrator)
		{
			if (ent->client->still_frames == frames-300)
				gi.centerprintf(ent, "You have 30 seconds to stop\nidling or you will be kicked.\n");
			else if (ent->client->still_frames == frames-100)
				gi.centerprintf(ent, "You have 10 seconds to stop\nidling or you will be kicked.\n");

			if (ent->client->still_frames > frames)
			{
				ent->client->still_frames = 0;
				gi.bprintf(PRINT_HIGH, "%s was kicked for inactivity.\n", ent->client->pers.netname);
				stuffcmd(ent, "disconnect\n");
			}
		}
	}
	// initialize chat-protect
	/*else*/ if (!ptr->value && !domination->value && !ctf->value && !ent->myskills.administrator
	&& !que_typeexists(ent->curses, 0) && (ent->myskills.streak < SPREE_START))
	{
		if(!((!ent->myskills.abilities[CLOAK].disable) && ((ent->myskills.abilities[CLOAK].current_level > 0))))
		{
			if (ent->client->idle_frames == CHAT_PROTECT_FRAMES-100)
				gi.centerprintf(ent, "10 seconds to chat-protect.\n");
			else if (ent->client->idle_frames == CHAT_PROTECT_FRAMES-50)
				gi.centerprintf(ent, "5 seconds to chat-protect.\n");
		
			if (ent->client->idle_frames == CHAT_PROTECT_FRAMES)
			{
				gi.centerprintf(ent, "Now in chat-protect mode.\n");
				ent->flags |= FL_CHATPROTECT;
				ent->svflags |= SVF_NOCLIENT;//4.5
				VortexRemovePlayerSummonables(ent);
				//3.0 Remove all active auras when entering chat protect
				AuraRemove(ent, 0);
			}
		}
	}
	//GHz END

	client = ent->client;

	if (ent->client->resp.spectator != true)
		ClientThinkstuff(ent);

	//Check player to see if they were loading or saving
	if(gds->value)	GDS_CheckPlayer(ent);

	if (deathmatch->value &&
		client->pers.spectator != client->resp.spectator &&
		(level.time - client->respawn_time) >= 5) {
		spectator_respawn(ent);
		return;
	}

	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk
//ZOID
		&& ent->movetype != MOVETYPE_NOCLIP
//ZOID
		)
		Think_Weapon (ent);
	else
		client->weapon_thunk = false;

	if (ent->deadflag)
	{
		// wait for any button just going down
		if ( level.time > client->respawn_time)
		{
			if (client->latched_buttons || ctf->value || // 3.7 force respawn in CTF
				(deathmatch->value && ((int)dmflags->value & DF_FORCE_RESPAWN)))
			{
				//gi.dprintf("respawn() called from player\n");
				respawn(ent);
				client->latched_buttons = 0;
			}
		}
		return;
	}

	// add player trail so monsters can follow
	if (!deathmatch->value)
		if (!visible (ent, PlayerTrail_LastSpot() ) )
			PlayerTrail_Add (ent->s.old_origin);

	client->latched_buttons = 0;

}
