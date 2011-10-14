#include "g_local.h"
#include "m_player.h"

#define MaxSentries 2

#define Level1Idle 0
#define Level1StartFirst (Q_stricmp(sentry_lev1_model->string, "models/sentry/turret1/tris.md2") == 0) ? 1 : 0
#define Level1StartEnd (Q_stricmp(sentry_lev1_model->string, "models/sentry/turret1/tris.md2") == 0) ? 2 : 0
#define Level1AttackFirst (Q_stricmp(sentry_lev1_model->string, "models/sentry/turret1/tris.md2") == 0) ? 3 : 0
#define Level1AttackEnd (Q_stricmp(sentry_lev1_model->string, "models/sentry/turret1/tris.md2") == 0) ? 6 : 0
#define Level23Idle 0
#define Level23AttackFirst (Q_stricmp(sentry_lev1_model->string, "models/sentry/turret1/tris.md2") == 0) ? 1 : 0
#define Level23AttackEnd (Q_stricmp(sentry_lev1_model->string, "models/sentry/turret1/tris.md2") == 0) ? 2 : 0
#define StatusIdle 0
#define StatusStart (Q_stricmp(sentry_lev1_model->string, "models/sentry/turret1/tris.md2") == 0) ? 1 : 0
#define StatusAttack (Q_stricmp(sentry_lev1_model->string, "models/sentry/turret1/tris.md2") == 0) ? 2 : 0

/*
#define SENTRYGUN_COST sentry_cost->value//150
#define SENTRYGUN_UPGRADE sentry_upgrade->value//100
#define SENTRY_HEALTHUP sentry_healthup->value//50
#define SENTRY_ARMORUP sentry_armorup->value//25
#define SENTRY_BULLETSUP sentry_bulletup->value//5
#define SENTRY_ROCKETSUP sentry_rocketup->value//20
*/
#define DIRECTION_LEFT		1
#define DIRECTION_CENTER	2
#define DIRECTION_RIGHT		3
/*
#define LEVEL1_DAMAGE sentry_level1_damage->value//6
#define LEVEL2_DAMAGE sentry_level2_damage->value//8
#define LEVEL3_DAMAGE sentry_level3_damage->value//12
*/
#define SENTRY1_MODEL sentry_lev1_model->string
#define SENTRY2_MODEL sentry_lev2_model->string
#define SENTRY3_MODEL sentry_lev3_model->string

#define max(a,b)    (((a) > (b)) ? (a) : (b))

void BecomeExplosion1 (edict_t *self);
void SP_Turret (edict_t *self);
//void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread);
void TurretFire1(edict_t *self);
void turret_self_remove(edict_t *ent);
void turret_remove(edict_t *ent);
void SentryMenuReload(edict_t *ent, pmenu_t *p);
void SentryMenuUpgrade(edict_t *ent, pmenu_t *p);
void SentryMenuRepair(edict_t *ent, pmenu_t *p);

int PlayerChangeScore(edict_t *self, int points);


//Add or subtract ammo from sentry gun
void UpdateSentryAmmo(edict_t *ent, int change_amt)
{
	ent->light_level += change_amt;
	if (ent->light_level == 25)
	{
		gi.cprintf(ent->creator, PRINT_HIGH, "Warning: Your sentry gun is low on ammo.\n");
	}

}

void turret_fire_rocket (edict_t *ent, int direction)
{
	vec3_t forward, right, start, target, dir;
	int damage = SENTRY_INITIAL_ROCKETDAMAGE;
	int speed = SENTRY_INITIAL_ROCKETSPEED;

	if ((ent->creator) && (ent->creator->client)) {
		damage += SENTRY_ADDON_ROCKETDAMAGE * ent->creator->myskills.build_sentry;
		speed += SENTRY_ADDON_ROCKETSPEED * ent->creator->myskills.build_sentry;
	}

	if(ent->light_level<1)
		return;

	UpdateSentryAmmo(ent, -1);

	AngleVectors (ent->s.angles, forward, right, NULL);
	if (direction == DIRECTION_LEFT)
	{
		start[0] = ent->s.origin[0] + forward[0] * 3 + right[0] * -0.5;
		start[1] = ent->s.origin[1] + forward[1] * 3 + right[1] * -0.5;
		start[2] = ent->s.origin[2] + forward[2] * 3 + right[2] * -0.5 +4;
	}
	else if (direction == DIRECTION_CENTER)
	{
		start[0] = ent->s.origin[0] + forward[0] * 3 ;
		start[1] = ent->s.origin[1] + forward[1] * 3 ;
		start[2] = ent->s.origin[2] + forward[2] * 3+4;
	}
	else		// Right
	{
		start[0] = ent->s.origin[0] + forward[0] * 3 + right[0] * 0.5;
		start[1] = ent->s.origin[1] + forward[1] * 3 + right[1] * 0.5;
		start[2] = ent->s.origin[2] + forward[2] * 3 + right[2] * 0.5+4;
	}

	// calc direction to where we targeted
	VectorMA (ent->enemy->s.origin, -0.05, ent->enemy->velocity, target);
	VectorSubtract (target, start, dir);
	VectorNormalize (dir);

	fire_rocket (ent, start, dir, damage, speed, damage, damage);
	//fire_blaster (ent, start, dir, damage, speed, EF_HYPERBLASTER, true);

	// send muzzle flash

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent - g_edicts);
	gi.WriteByte (MZ_SHOTGUN );
	gi.multicast (start, MULTICAST_PVS);
//	gi.sound(ent, CHAN_VOICE, gi.soundindex("boss3/xfire.wav"), 1, ATTN_NORM, 0);

}



//New turret bullet firing code.  Replaces fire_bullet and fire_rail
//Fires at ent->enemy
//direction = 1 (left), 2 (center), 3 (right)
void turret_fire_bullet (edict_t *ent, int direction)
{

	vec3_t		forward, right, start, target, dir;
	vec3_t		from;
	vec3_t		end;
	trace_t		tr;
	edict_t		*ignore;
	int			mask;
	qboolean	water;
	int			kick = 0;
	int			mod = MOD_SENTRY;
	int			damage;
	float		chance_of_hit;
	float		maxvelocity;
	float		r;


	if(ent->light_level<1)  //do we have ammo?
		return;

	if (!ent->enemy)
		return;

	UpdateSentryAmmo(ent, -1);

	//Calculate starting position
	AngleVectors (ent->s.angles, forward, right, NULL);

	if (direction == DIRECTION_LEFT)
	{
		start[0] = ent->s.origin[0] + forward[0] * 3 + right[0] * -0.5;
		start[1] = ent->s.origin[1] + forward[1] * 3 + right[1] * -0.5;
		start[2] = ent->s.origin[2] + forward[2] * 3 + right[2] * -0.5 +4;
	}
	else if (direction == DIRECTION_CENTER)
	{
		start[0] = ent->s.origin[0] + forward[0] * 3 ;
		start[1] = ent->s.origin[1] + forward[1] * 3 ;
		start[2] = ent->s.origin[2] + forward[2] * 3+4;
	}
	else		// Right
	{
		start[0] = ent->s.origin[0] + forward[0] * 3 + right[0] * 0.5;
		start[1] = ent->s.origin[1] + forward[1] * 3 + right[1] * 0.5;
		start[2] = ent->s.origin[2] + forward[2] * 3 + right[2] * 0.5+4;
	}

	// calc direction to where we targeted
	VectorMA (ent->enemy->s.origin, -0.05, ent->enemy->velocity, target);

	VectorSubtract (target, start, dir);
	VectorNormalize (dir);

	VectorMA (start, 8192, dir, end);
	VectorCopy (start, from);
	ignore = ent;
	water = false;
	mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;

	tr = gi.trace (from, NULL, NULL, end, ignore, mask);

	if (tr.contents & (CONTENTS_SLIME|CONTENTS_LAVA))
	{
		mask &= ~(CONTENTS_SLIME|CONTENTS_LAVA);
		water = true;
	}

	//Do damage

	//Damage based on sentry level
	damage = (int) (SENTRY_ADDON_BULLETDAMAGE * ent->creator->myskills.build_sentry);
	chance_of_hit = 1;
	if (ent->count == 1)
	{
		damage *= SENTRY_LEVEL1_DAMAGE;
		chance_of_hit = 0.5;
	}
	else if (ent->count == 2)
	{
		damage *= SENTRY_LEVEL2_DAMAGE;
		chance_of_hit = 0.75;
	}
	else if (ent->count == 3)
	{
		damage *= SENTRY_LEVEL3_DAMAGE;
		chance_of_hit = 0.95;
	}
	else
	{
		damage = 1;		//don't know what level it is
		chance_of_hit = 1.0;		//always hit
	}
	
	chance_of_hit += 0.01 * ent->creator->myskills.build_sentry;
//	damage = 0;

	//Moving reduces chance of being hit
	maxvelocity = max( abs (ent->enemy->velocity[0]), abs (ent->enemy->velocity[1]) );

	if (maxvelocity >= 320)		//wierd setting for grappling
	{
		chance_of_hit = chance_of_hit  * .80;
	}
	else if (maxvelocity >= 250)		//running
	{
		chance_of_hit = chance_of_hit  * .85;
	}
	else if (maxvelocity >= 150) //walking
	{
		chance_of_hit = chance_of_hit  * .95;
	}
	if (chance_of_hit > 0.95)
		chance_of_hit = 0.95;
	r = random();


	if ((ent->enemy->takedamage) && (r <= chance_of_hit))
        T_Damage (ent->enemy, ent, ent, dir, tr.endpos, tr.plane.normal, damage, 2*damage, 0, MOD_SENTRY);

	//Do gunshot effect
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_GUNSHOT);
	gi.WritePosition (tr.endpos);
	gi.WriteDir (tr.plane.normal);
	gi.multicast (tr.endpos, MULTICAST_PVS);

	if (ent->client)
		PlayerNoise(ent, tr.endpos, PNOISE_IMPACT);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent - g_edicts);
	gi.WriteByte (MZ_SHOTGUN );
	gi.multicast (start, MULTICAST_PVS);
}




void TurretAnimate(edict_t *ent)
{
	if(ent->count==1)
	{
		if(ent->delay==StatusIdle)
		{
			ent->s.frame = Level1Idle;
			return;
		}
		else if(ent->delay==StatusStart)
		{
			if(ent->s.frame == Level1StartEnd)
			{
				ent->delay=StatusAttack;
				ent->s.frame=Level1AttackFirst;
				return;
			}
			else if(ent->s.frame==Level1Idle)
			{
				ent->s.frame = Level1StartFirst;
				return;
			}
			else if(ent->s.frame==Level1StartFirst)
			{
				ent->s.frame++;
				return;
			}
			else
			{
				ent->s.frame = Level1Idle;
				return;
			}
		}
		else
		{
			if(ent->delay==StatusAttack)
			{
				if(ent->s.frame==Level1AttackEnd)
				{
					ent->s.frame--;
					return;
				}
				else
				{
					ent->s.frame++;
					return;
				}
			}
		}
	}
	else
	{
		if(ent->delay ==StatusIdle)
		{
			ent->s.frame = Level23Idle;
			return;
		}
		else
		{
			if(ent->s.frame==Level23AttackEnd)
			{
				ent->s.frame = Level23AttackFirst;
				return;
			}
			ent->s.frame++;
			return;
		}
	}
}


void FireTurretLeft(edict_t *ent)
{
	float f;


	//Fire bullet.  For level 3 sentry's, fire rocket every 10th round
	f = ent->light_level / 20;
	if (((f * 20) == ent->light_level) && (ent->count == 3))
		turret_fire_rocket (ent, DIRECTION_LEFT);
	else
		turret_fire_bullet (ent, DIRECTION_LEFT);
}


void FireTurretCenter(edict_t *ent)
{
	float f;


	//Fire bullet.  For level 3 sentry's, fire rocket every 10th round
	f = ent->light_level / 20;
	if (((f * 20) == ent->light_level) && (ent->count == 3))
		turret_fire_rocket (ent, DIRECTION_CENTER);
	else
		turret_fire_bullet (ent, DIRECTION_CENTER);
}

void FireTurretRight(edict_t *ent)
{
	float f;


	//Fire bullet.  For level 3 sentry's, fire rocket every 10th round
	f = ent->light_level / 20;
	if (((f * 20) == ent->light_level) && (ent->count == 3))
		turret_fire_rocket (ent, DIRECTION_RIGHT);
	else
		turret_fire_bullet (ent, DIRECTION_RIGHT);
}


void Turret_Think(edict_t *self)
{
	static char Sentry[32];
	static char Armor[32];
	static char Ammo[32];
	static char Level[32];
	int		range;
	int		light_level;
	int		max_dist;
	edict_t *blip = NULL;
	int	contents;
	int ideal_yaw = 0;
	vec3_t v;
	trace_t		tr;
//	gitem_t	*ammo;
//	int max;
	float checkyaw;
	blip = NULL;

	//If there is no client attached to this turret any more, it should be removed
	//from the game
	if (!self->creator)				//There is no creator
	{
		turret_self_remove(self);
	}

	if (!self->creator->client)		//Creator isn't a client
	{
		turret_self_remove(self);
	}

	//If sentry or stand is in something solid, remove it
	contents = (int)gi.pointcontents(self->s.origin);

	if (contents & CONTENTS_SOLID)
	{
		gi.cprintf(self->creator, PRINT_HIGH, "Your sentry was in a bad map position, so it was removed.\n");
		self->creator->client->pers.inventory[power_cube_index] += SENTRY_COST;
		turret_self_remove(self);
		return;
	}

	if (self->sentry)	//stand
	{
		contents = (int)gi.pointcontents(self->sentry->s.origin);

		if (contents & CONTENTS_SOLID)
		{
			gi.cprintf(self->creator, PRINT_HIGH, "Your sentry stand in a bad map position so it was removed.\n");
			self->creator->client->pers.inventory[power_cube_index] += SENTRY_COST;
			turret_self_remove(self);
			return;
		}
	}

	self->s.effects &= ~(EF_COLOR_SHELL|EF_POWERSCREEN);
	self->s.renderfx &= ~(RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE|RF_SHELL_YELLOW);

	if ((self->aura & AURA_THORNS || self->aura & AURA_SALVATION) && self->auratime > level.time)
	{
		self->s.effects |= EF_COLOR_SHELL;
		self->s.renderfx |= RF_SHELL_CYAN;
	}
	if (ctf->value){
		self->s.effects |= EF_COLOR_SHELL;
		if (self->creator->client->resp.ctf_team == 1)
			self->s.renderfx |= RF_SHELL_RED;
		else
			self->s.renderfx |= RF_SHELL_BLUE;
	}
	if (self->count == 1)
	{
		range = 500;
		light_level = 10;
		max_dist = 300;
	}
	else if (self->count == 2)
	{
		range = 800;
		light_level = 7;
		max_dist = 500;
	}

	else if (self->count == 3)
	{
		range = 900;
		light_level = 4;
		max_dist = 700;
	}

	// Don't shoot frozen, dead, or non visible enemies
	if ((self->enemy) &&
		((self->enemy->health <= 0) || (!visible(self, self->enemy)) || (self->enemy->cursed & CURSE_FROZEN)))
	{
		self->enemy = NULL;
		self->s.angles[YAW] = self->sentry->s.angles[YAW];
	}

	if (!self->enemy)
	{
		// If we are going back to idle from some other status,
		// copy the angles back to original
		self->delay=StatusIdle;

		while ((blip = findclosestradius(blip, self->s.origin, range)) != NULL)
		{
			//dist = VectorLength(v);
			if (!blip->inuse)
				continue;
			if (!blip->takedamage)
				continue;
			if (blip->solid == SOLID_NOT)
				continue;	//don't see observers
			if (blip->health <= 0)
				continue;
			if (OnSameTeam(self, blip))
				continue;
			if (blip->cursed & CURSE_FROZEN)
				continue;
			if (blip == self->creator)
				continue; // Don't attack the owner
			if (!infront(self, blip))
				continue;
			if (!visible(self, blip) /*&& dist > max_dist*/)
				continue;

			//If we are in a spree war and this monster is owned by non-spreer only attack the spree person and his monsters/sentries
			if (SPREE_WAR == true && self->creator != SPREE_DUDE) {
				if (blip->client && blip != SPREE_DUDE)
					continue;
				if (blip->creator && blip->creator != SPREE_DUDE)
					continue;
				if (blip->activator && blip->activator != SPREE_DUDE)
					continue;
			}

			//If invincible, skip for now
			if (blip->client && blip->client->invincible_framenum > level.framenum)
				continue;

			tr = gi.trace (self->s.origin, NULL, NULL, blip->s.origin, self, MASK_SOLID);
			if (tr.fraction != 1.0)
				continue;

			self->enemy = blip;
			if (self->enemy) {
				self->nextthink = level.time + 0.1; //Time before sentry shoots target
				gi.sound (self, CHAN_VOICE, gi.soundindex("weapons/turrspot.wav"), 1, ATTN_NORM, 0);
				break;
			}
		}
	}
	else
	{
		VectorSubtract (self->enemy->s.origin, self->s.origin, v);

		self->ideal_yaw = vectoyaw(v);

		M_ChangeYaw(self);

		checkyaw = anglemod(self->s.angles[YAW])-self->ideal_yaw;

		if (checkyaw>-25 && checkyaw<25)
		{
			//if (self->light_level>0)
			//{
				if (visible(self, self->enemy))
				{
//					if (self->s.frame != 1)
//					{
						if (self->delay==StatusIdle)
							self->delay = StatusStart;

/*						if (self->count == 1)
						{
							FireTurretRight(self);
							FireTurretLeft(self);
						}
						else if (self->count == 2)
						{
							FireTurretRight(self);
							FireTurretLeft(self);
						}

						else if (self->count == 3)

						{
							FireTurretRight(self);
							FireTurretLeft(self);
						}*/
						FireTurretRight(self);
						FireTurretLeft(self);

//					}
				}
				else
				{
					self->delay=StatusIdle;
					self->enemy=NULL;
				}
			//}
		}
		else if (!infront(self, self->enemy))
		{
			self->delay=StatusIdle;
			self->enemy=NULL;
		}
	}


	//Move the stand? (GREGG)
	if (self->sentry)
		VectorCopy(self->s.origin,self->sentry->s.origin);

	TurretAnimate(self);
	if (self->nextthink <= level.time)
		self->nextthink = level.time + 0.1;

	if(self->delay==StatusIdle)
	{
		if(self->sentrydelay<level.time)
		{
			if(self->PlasmaDelay)
				self->PlasmaDelay=0;
			else
				self->PlasmaDelay=1;
			gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/gunidle1.wav"), 0.1, ATTN_NORM, 0);
			self->sentrydelay=level.time + 2;
		}
		if(self->PlasmaDelay)
			self->s.angles[YAW] += 2.0;
		else
			self->s.angles[YAW] -= 2.0;
		if(self->s.angles[YAW]<0)
			self->s.angles[YAW]+=360;
		else if(self->s.angles[YAW]>360)
			self->s.angles[YAW]-=360;
	}
}


//Ent = turret entity
void turret_self_remove(edict_t *ent)
{

	//Clear client's pointer to sentry gun
	if (ent->creator)
	{
		ent->creator->sentry = NULL;
		ent->creator->num_sentries -= 3;
		if (ent->creator->num_sentries < 0)
			ent->creator->num_sentries = 0;
	}

	//first free stand
	if (ent->sentry) G_FreeEdict(ent->sentry);

	//Then free the sentry gun
	BecomeExplosion1 (ent);


}

void turret_remove(edict_t *ent)
{
	//gi.dprintf("DEBUG: turret_remove() start\n");
	if (ent->creator)
	{
		//gi.cprintf(ent->creator, PRINT_HIGH, "Sentry Gun off.\n");
		ent->creator->num_sentries -= 3;
		if (ent->creator->num_sentries < 0)
			ent->creator->num_sentries = 0;
	}

	//First free the stand
	if (ent->sentry)
	{
		//gi.dprintf("DEBUG: turret_remove() attempted to free stand\n");
		G_FreeEdict(ent->sentry);
		ent->sentry = NULL;
	}


	//Then free the sentry gun
	BecomeExplosion1 (ent);
	//gi.dprintf("DEBUG: turret_remove() end\n");
	return;
}

void turret_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{

	vec3_t origin;

	edict_t *blip = NULL;
	
	gi.cprintf(self->creator, PRINT_HIGH, "Sentry Gun Destroyed.\n"); 

	//Give a frag to the attacker
	if (attacker->client)
	{
		//K03 Begin
		if (attacker != self->creator)
		{
	//	Add_exp(attacker, self);
		if (attacker->client)
			attacker->myskills.sentry_frags++;
		//K03 End
		if (attacker->client && self->creator->client)
			gi.bprintf(PRINT_HIGH, "%s destroyed %s's sentry gun.\n", attacker->client->pers.netname, self->creator->client->pers.netname);

		}

	}

	VectorCopy (self->s.origin,origin);
	origin[2]+= 0.5;
	self->takedamage = DAMAGE_NO;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (origin);
	gi.multicast (origin, MULTICAST_PVS);

	turret_self_remove(self);

}

void SentryReload(edict_t *ent);
void SentryMenuReload(edict_t *ent, pmenu_t *p)
{
	SentryReload(ent);
}
void SentryReload(edict_t *ent)
{	
	int max, armorfill;
	float dist;
	vec3_t distance;

	if (!ent->selectedsentry)
		return;

	distance[0]=ent->s.origin[0] - ent->selectedsentry->s.origin[0];
	distance[1]=ent->s.origin[1] - ent->selectedsentry->s.origin[1];
	distance[2]=ent->s.origin[2] - ent->selectedsentry->s.origin[2];
	dist=VectorLength(distance);
	if(dist>100)
	{
		gi.cprintf(ent, PRINT_HIGH, "Sentry too far away.\n"); 
		PMenu_Close(ent);
		return;
	}
	if (ent->selectedsentry->light_level!=ent->selectedsentry->gib_health)
	{
		max = ent->selectedsentry->gib_health;
		armorfill=ent->selectedsentry->gib_health-ent->selectedsentry->light_level;
		if(armorfill > 50)
			armorfill = 50;
//		if(armorfill>25)
//			armorfill=25;
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] < 25)
		{
 			gi.cprintf(ent, PRINT_HIGH, "You need 25 Bullets to reload the sentry gun.\n");
			if (ent->selectedsentry) gi.sound(ent->selectedsentry, CHAN_VOICE, gi.soundindex("misc/keytry.wav"), 1, ATTN_NORM, 0);
			PMenu_Close(ent);
			return;
		}

		//25 player bullets will give 75 bullets to sentry gun (Gregg)
//		if(armorfill > ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))])
//			armorfill = ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))];

		UpdateSentryAmmo(ent->selectedsentry, armorfill);
//		ent->selectedsentry->light_level += armorfill;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] -= 25;
		if (ent->selectedsentry->light_level > max)
			ent->selectedsentry->light_level = max;
		gi.sound(ent->selectedsentry, CHAN_ITEM, gi.soundindex("misc/w_pkup.wav"), 1, ATTN_NORM, 0);
	}
	PMenu_Close(ent);
}

void AddArmorToSentry(edict_t *ent, int amt)
{
	int max, armor;

	if (ent->health < ent->max_health)
	{
		max = ent->max_health;

		//How much armor is needed?
		armor = ent->max_health - ent->health;

		if (amt < armor) 
			armor = amt;
		else 
			armor = 0;

		ent->health+= armor;
		if (ent->health > max)
			ent->health = max;

		if (ent->selectedsentry)
			gi.sound(ent->selectedsentry, CHAN_ITEM, gi.soundindex("misc/w_pkup.wav"), 1, ATTN_NORM, 0);
	}
}

void SentryRepair(edict_t *ent);
void SentryMenuRepair(edict_t *ent, pmenu_t *p)
{
	SentryRepair(ent);
}

void SentryRepair(edict_t *ent)
{	
	int maxh, maxa, armor, health;
	int cells = 0, maxcellh, maxcella, currcells;
	float dist;
	vec3_t distance;

	if (!ent->selectedsentry)
		return;

	distance[0] = ent->s.origin[0] - ent->selectedsentry->s.origin[0];
	distance[1] = ent->s.origin[1] - ent->selectedsentry->s.origin[1];
	distance[2] = ent->s.origin[2] - ent->selectedsentry->s.origin[2];
	dist = VectorLength(distance);
	if(dist > 100)
	{
		gi.cprintf(ent, PRINT_HIGH, "Sentry too far away.\n"); 
		PMenu_Close(ent);
		return;
	}

	maxh = SENTRY_INITIAL_HEALTH + SENTRY_ADDON_HEALTH * ent->myskills.build_sentry;
	maxa = SENTRY_INITIAL_ARMOR + SENTRY_ADDON_ARMOR * ent->myskills.build_sentry;

	if ((ent->selectedsentry->health < maxh) || (ent->selectedsentry->monsterinfo.power_armor_power < maxa))
	{
		//How much health is needed?
		health = maxh - ent->selectedsentry->health;
		armor = maxa - ent->selectedsentry->monsterinfo.power_armor_power;

		//Can't give more than 50 points armor in one shot
		if (armor > 50) 
			armor = 50;
		if (health > 50)
			health = 50;

		//Each cell gives 4 points armor
		currcells = ent->client->pers.inventory[power_cube_index];
		maxcella = armor / 4;
		maxcellh = health / 4;

		if (currcells == 0)
		{
 			gi.cprintf(ent, PRINT_HIGH, "You need power cubes to repair the sentry gun.\n");
			if (ent->selectedsentry) gi.sound(ent->selectedsentry, CHAN_VOICE, gi.soundindex("misc/keytry.wav"), 1, ATTN_NORM, 0);

			PMenu_Close(ent);
			return;
		}

		//If we don't have enough cells, adjust the amount of armor to give
		if(currcells < maxcellh)
		{
			cells = currcells;
			health = cells * 4;
			armor = 0;
		}
		else if (currcells < maxcella)
		{
			cells = currcells;
			armor = cells * 4;
			health = 0;
		}
		else
		{
			cells = maxcellh + maxcella;
		}

		gi.sound(ent->selectedsentry, CHAN_ITEM, gi.soundindex("misc/w_pkup.wav"), 1, ATTN_NORM, 0);
		ent->selectedsentry->health += health;
		if (ent->selectedsentry->health > maxh)
			ent->selectedsentry->health = maxh;
		ent->selectedsentry->monsterinfo.power_armor_power += armor;
		if (ent->selectedsentry->monsterinfo.power_armor_power > maxa)
			ent->selectedsentry->monsterinfo.power_armor_power = maxa;
		ent->client->pers.inventory[power_cube_index] -= cells;
	}
	PMenu_Close(ent);
}

void SentryUpgrade(edict_t *ent);
void SentryMenuUpgrade(edict_t *ent, pmenu_t *p)
{
	SentryUpgrade(ent);
}

void SentryUpgrade(edict_t *ent)
{
	float dist;
	vec3_t distance;

	if (!ent->selectedsentry)
		return;

	distance[0]=ent->s.origin[0] - ent->selectedsentry->s.origin[0];
	distance[1]=ent->s.origin[1] - ent->selectedsentry->s.origin[1];
	distance[2]=ent->s.origin[2] - ent->selectedsentry->s.origin[2];
	dist=VectorLength(distance);
	if(dist>100)
	{
		gi.cprintf(ent, PRINT_HIGH, "Sentry too far away.\n"); 
		PMenu_Close(ent);
		return;
	}
	if (ent->selectedsentry->count == 3)
	{
//		gi.cprintf(ent, PRINT_HIGH, "Sentry gun already at level 3\n");
		return;
	}

	if (ent->client->pers.inventory[power_cube_index] < SENTRY_UPGRADE)
	{
		gi.cprintf(ent, PRINT_HIGH, "You need %d more power cubes to upgrade sentry gun.\n", (SENTRY_UPGRADE - ent->client->pers.inventory[power_cube_index]));
		PMenu_Close(ent);
		return;
	}

	if (ent->selectedsentry->count < 3)
		ent->selectedsentry->count++;
	ent->client->pers.inventory[power_cube_index] -= SENTRY_UPGRADE;
	
	ent->selectedsentry->health = SENTRY_INITIAL_HEALTH + SENTRY_ADDON_HEALTH * ent->myskills.build_sentry;
	ent->selectedsentry->max_health = ent->selectedsentry->health;

	ent->selectedsentry->gib_health = SENTRY_INITIAL_AMMO + SENTRY_ADDON_AMMO * ent->myskills.build_sentry;//Max Ammo
	ent->selectedsentry->light_level = ent->selectedsentry->gib_health;//Ammo Total
	ent->selectedsentry->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
	ent->selectedsentry->monsterinfo.power_armor_power = SENTRY_INITIAL_ARMOR + SENTRY_ADDON_ARMOR * ent->myskills.build_sentry;
	if (ent->selectedsentry->count == 2)
	{
//		ent->selectedsentry->health= 200 + SENTRY_HEALTHUP * ent->myskills.build_sentry;
//		ent->selectedsentry->max_health =200 + SENTRY_HEALTHUP * ent->myskills.build_sentry;
//		ent->selectedsentry->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
//		ent->selectedsentry->monsterinfo.power_armor_power = 100 + SENTRY_ARMORUP * ent->myskills.build_sentry;

		ent->selectedsentry->s.modelindex = gi.modelindex (SENTRY2_MODEL);
		ent->selectedsentry->delay = StatusIdle;
		ent->selectedsentry->s.frame = 0;
		ent->selectedsentry->yaw_speed = 20;
//		ent->selectedsentry->gib_health = 150+ SENTRY_BULLETSUP * ent->myskills.build_sentry;//Max Ammo
//		ent->selectedsentry->light_level = 150+ SENTRY_BULLETSUP * ent->myskills.build_sentry;
/*
		if (!ctf->value) {
			ent->selectedsentry->s.renderfx &= ~RF_SHELL_GREEN;
			ent->selectedsentry->s.renderfx |= (RF_SHELL_RED|RF_SHELL_BLUE);
		}
		*/
		//ent->selectedsentry->turretsoundcountdown = 0;
		gi.sound(ent->selectedsentry, CHAN_VOICE, gi.soundindex("weapons/turrset.wav"), 1, ATTN_NORM, 0);
	}
	
	if (ent->selectedsentry->count == 3)
	{
//		ent->selectedsentry->health = 300 + SENTRY_HEALTHUP * ent->myskills.build_sentry;;
//		ent->selectedsentry->max_health = 300 + SENTRY_HEALTHUP * ent->myskills.build_sentry;;
//		ent->selectedsentry->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
//		ent->selectedsentry->monsterinfo.power_armor_power = 100 + SENTRY_ARMORUP * ent->myskills.build_sentry;
		ent->selectedsentry->s.modelindex = gi.modelindex (SENTRY3_MODEL);
		ent->selectedsentry->delay = StatusIdle;
		ent->selectedsentry->s.frame = 0;			
		ent->selectedsentry->yaw_speed = 28;
//		ent->selectedsentry->gib_health = 225+ SENTRY_BULLETSUP * ent->myskills.build_sentry;//Max Ammo
//		ent->selectedsentry->light_level = 225+ SENTRY_BULLETSUP * ent->myskills.build_sentry;
		/*
		if (!ctf->value) {
			ent->selectedsentry->s.renderfx &= ~RF_SHELL_GREEN;
			ent->selectedsentry->s.renderfx &= ~RF_SHELL_BLUE;
			ent->selectedsentry->s.renderfx |= (RF_SHELL_RED);
		}*/
		//ent->selectedsentry->turretsoundcountdown = 0;
		gi.sound(ent->selectedsentry, CHAN_VOICE, gi.soundindex("weapons/turrset.wav"), 1, ATTN_NORM, 0);
	}
	PMenu_Close(ent);
}

void Sentry_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{ 
	if (other->client && ent->creator == other) {
		if(other->sentrydelay > level.time)
			return;
		other->selectedsentry = ent;
		SentryUpgrade(other);
		SentryRepair(other);
		SentryReload(other);
		other->sentrydelay = level.time + 1.5;
		gi.cprintf(other, PRINT_HIGH, "Sentry repaired and rearmed\nHealth: (%d/%d)   Armor: (%d/%d)   Ammo: (%d/%d)\n",
			ent->health, SENTRY_INITIAL_HEALTH + SENTRY_ADDON_HEALTH * other->myskills.build_sentry, ent->monsterinfo.power_armor_power, SENTRY_INITIAL_ARMOR + SENTRY_ADDON_ARMOR * other->myskills.build_sentry, ent->light_level, ent->gib_health);
	}
	return;

	if (other->client && ent->creator == other) 
		other->selectedsentry = ent;

	//Skip if they already have a menu up
	if ((other->client) && (other->client->menu)) 
		if (other->client->menu->entries != other->client->iddisplay)
			return;

	if(other->sentrydelay>level.time)
		return;

	//Only show menu a max of every 1 second

	//Only allow team mates access to menu
	if (other->client && other->myskills.build_sentry > 0)
	{
		sprintf(other->client->wfsentrystr[0], "*%s's Sentry", ent->creator->client->pers.netname);
		sprintf(other->client->wfsentrystr[1], "  Armor:%d/%d", ent->health, ent->max_health);
		sprintf(other->client->wfsentrystr[2], "  Ammo:%d/%d", ent->light_level,ent->gib_health);
		sprintf(other->client->wfsentrystr[3], "  Level:%d", ent->count);
		other->client->sentrymenu[0].text = other->client->wfsentrystr[0];
		other->client->sentrymenu[0].SelectFunc = NULL;
		other->client->sentrymenu[0].align = PMENU_ALIGN_CENTER;
		other->client->sentrymenu[0].arg = 0;
		other->client->sentrymenu[1].text = other->client->wfsentrystr[1];
		other->client->sentrymenu[1].SelectFunc = NULL;
		other->client->sentrymenu[1].align = PMENU_ALIGN_LEFT;
		other->client->sentrymenu[1].arg = 0;
		other->client->sentrymenu[2].text = other->client->wfsentrystr[2];
		other->client->sentrymenu[2].SelectFunc = NULL;
		other->client->sentrymenu[2].align = PMENU_ALIGN_LEFT;
		other->client->sentrymenu[2].arg = 0;
		other->client->sentrymenu[3].text = other->client->wfsentrystr[3];
		other->client->sentrymenu[3].SelectFunc = NULL;
		other->client->sentrymenu[3].align = PMENU_ALIGN_LEFT;
		other->client->sentrymenu[3].arg = 0;
		other->client->sentrymenu[4].text = "1. Upgrade";
		other->client->sentrymenu[4].SelectFunc = SentryMenuUpgrade;
		other->client->sentrymenu[4].align = PMENU_ALIGN_LEFT;
		other->client->sentrymenu[4].arg = 0;
		other->client->sentrymenu[5].text = "2. Repair";
		other->client->sentrymenu[5].SelectFunc = SentryMenuRepair;
		other->client->sentrymenu[5].align = PMENU_ALIGN_LEFT;
		other->client->sentrymenu[5].arg = 0;
		other->client->sentrymenu[6].text = "3. Reload";
		other->client->sentrymenu[6].SelectFunc = SentryMenuReload;
		other->client->sentrymenu[6].align = PMENU_ALIGN_LEFT;
		other->client->sentrymenu[6].arg = 0;

//		other->selectedsentry = ent->standowner->sentry;
		other->selectedsentry = ent;
		if ((other->client) && (other->client->menu)) 
			if (other->client->menu->entries == other->client->iddisplay)
				PMenu_Close(other);

		PMenu_Open(other, other->client->sentrymenu, -1, sizeof(other->client->sentrymenu) / sizeof(pmenu_t), true, false);
		other->sentrydelay = level.time + 3;

		//Set timeout for menu (4 seconds)
		if (other->client->menu) other->client->menu->MenuTimeout = level.time + 4;
	}
}
//Stand think function
void Stand_Think (edict_t *ent)
{
	//For some reason, entity needs a think function
	if (ent->health <= 0)
		G_FreeEdict(ent);
	else
		ent->nextthink = level.time + 5.0;
}


//Build the turret
void place_turret (edict_t *ent)
{
	vec3_t		forward,up,right,wallp, pos,try1,try2,try3,try4;
	edict_t *blip = NULL;
	edict_t *scan = NULL;//GHz
	trace_t		tr;
	edict_t *sentry;
	edict_t *sentrystand;

	// valid ent ?
  	if ((!ent->client) || (ent->health<=0))
	   return;
	if (ent->num_sentries + 2 > MaxSentries) {
		gi.cprintf(ent, PRINT_HIGH, "You have reached the max of %d sentries\n", SENTRY_MAX);
		return;
	}

	if (ent->client->pers.inventory[power_cube_index] < SENTRY_COST) {
		gi.cprintf(ent, PRINT_HIGH, "You need %d more power cubes\n", (SENTRY_COST - ent->client->pers.inventory[power_cube_index]));
		return;
	}

	if (ctf->value)
	{
		while ((scan = findradius(scan, ent->s.origin, 1024)) != NULL)
		{
			if (ent->client->resp.ctf_team == CTF_TEAM1 && !strcmp(scan->classname, "item_flag_team2")
				&& !(scan->spawnflags & DROPPED_ITEM)){
				gi.cprintf(ent, PRINT_HIGH, "Building sentries in opponents base is not allowed!\n");
				return;
			}
			if (ent->client->resp.ctf_team == CTF_TEAM2 && !strcmp(scan->classname, "item_flag_team1")
				&& !(scan->spawnflags & DROPPED_ITEM)){
				gi.cprintf(ent, PRINT_HIGH, "Building sentries in opponents base is not allowed!\n");
				return;
			}

		}
	}

	if (ent->client->ability_delay > level.time) {
		gi.cprintf (ent, PRINT_HIGH, "You can't use abilities for another %2.1f seconds\n", ent->client->ability_delay - level.time);
		return;
	}
	ent->client->ability_delay = level.time + DELAY_SENTRY;

	// Setup "little look" to close wall
	VectorCopy(ent->s.origin,wallp);         

	// Cast along view angle
	AngleVectors (ent->client->v_angle, forward, right, up);

	// Setup end point
	pos[0]=ent->s.origin[0]+forward[0]*75;
	pos[1]=ent->s.origin[1]+forward[1]*75;
	pos[2]=ent->s.origin[2]+forward[2]*75+30;  
	wallp[0]=ent->s.origin[0]+forward[0]*150;
	wallp[1]=ent->s.origin[1]+forward[1]*150;
	wallp[2]=ent->s.origin[2]+forward[2]*150+30; 
	try1[0]=ent->s.origin[0]+forward[0]*150+right[0]*20;
	try1[1]=ent->s.origin[1]+forward[1]*150+right[1]*20;
	try1[2]=ent->s.origin[2]+forward[2]*150+30+right[2]*20; 
	try2[0]=ent->s.origin[0]+forward[0]*150+right[0]*-20;
	try2[1]=ent->s.origin[1]+forward[1]*150+right[1]*-20;
	try2[2]=ent->s.origin[2]+forward[2]*150+30+right[2]*-20; 
	try3[0]=ent->s.origin[0]+forward[0]*75+right[0]*20;
	try3[1]=ent->s.origin[1]+forward[1]*75+right[1]*20;
	try3[2]=ent->s.origin[2]+forward[2]*75+30+right[2]*20; 
	try4[0]=ent->s.origin[0]+forward[0]*75+right[0]*-20;
	try4[1]=ent->s.origin[1]+forward[1]*75+right[1]*-20;
	try4[2]=ent->s.origin[2]+forward[2]*75+30+right[2]*-20; 
	// trace
	tr = gi.trace (ent->s.origin, NULL, NULL, wallp, ent, MASK_SOLID);
	
		
	// Line complete ? (ie. no collision)
	if (tr.fraction != 1.0)
	{//42 ebc
	 	gi.cprintf (ent, PRINT_HIGH, "Not enough room.\n");
		return;
	}
	wallp[2]+=22;
	tr = gi.trace (pos, NULL, NULL, wallp, ent, MASK_SOLID);
	// Line complete ? (ie. no collision)
	if (tr.fraction != 1.0)
	{
	 	gi.cprintf (ent, PRINT_HIGH, "Not enough room. Try aiming lower\n");
		return;
	}
	wallp[2]-=40;
		tr = gi.trace (pos, NULL, NULL, wallp, ent, MASK_SOLID);
	// Line complete ? (ie. no collision)
	if (tr.fraction != 1.0)
	{
	 	gi.cprintf (ent, PRINT_HIGH, "Not enough room. Try aiming higher\n");
		return;
	}
	// trace
	tr = gi.trace (ent->s.origin, NULL, NULL, try1, ent, MASK_SOLID);
	if (tr.fraction != 1.0)
	{
	 	gi.cprintf (ent, PRINT_HIGH, "Not enough room.\n");
		return;
	}
	// trace
	tr = gi.trace (ent->s.origin, NULL, NULL, try2, ent, MASK_SOLID);
	if (tr.fraction != 1.0)
	{
	 	gi.cprintf (ent, PRINT_HIGH, "Not enough room.\n");
		return;
	}
	// trace
	tr = gi.trace (ent->s.origin, NULL, NULL, try3, ent, MASK_SOLID);
	if (tr.fraction != 1.0)
	{
	 	gi.cprintf (ent, PRINT_HIGH, "Not enough room.\n");
		return;
	}
	// trace
	tr = gi.trace (ent->s.origin, NULL, NULL, try4, ent, MASK_SOLID);
	if (tr.fraction != 1.0)
	{
	 	gi.cprintf (ent, PRINT_HIGH, "Not enough room.\n");
		return;
	}

	// Hit sky ?
	if ((tr.surface) && (tr.surface->flags & SURF_SKY))
		return;

	while (blip = findradius (blip, pos, 50))
	{
		 if ( (!strcmp(blip->classname, "worldspawn") )
		  || (!strcmp(blip->classname, "info_player_start") )
		  || (!strcmp(blip->classname, "info_player_deathmatch") )
		  || (!strcmp(blip->classname, "item_flagreturn_team1") )
		  || (!strcmp(blip->classname, "item_flagreturn_team2") )
		  || (!strcmp(blip->classname, "misc_teleporter_dest") )
		  || (!strcmp(blip->classname, "info_teleport_destination") ) )
		 {
		  	gi.cprintf (ent, PRINT_HIGH, "Too Close to Items.\n");
		  	return ;
		 }
	}

	ent->client->pers.inventory[power_cube_index] -= SENTRY_COST;


	gi.cprintf(ent, PRINT_HIGH, "Sentry Gun on.\n");
	sentry = G_Spawn();

	VectorClear (sentry->mins);
	VectorClear (sentry->maxs);
	VectorCopy (pos, sentry->s.origin);
	sentry->s.angles[0] = 0;
	sentry->s.angles[YAW] = ent->s.angles[YAW] - 20;
	sentry->movetype = MOVETYPE_TOSS;
	sentry->clipmask = MASK_PLAYERSOLID;
	sentry->mass = 400;
	ent->s.renderfx=RF_FRAMELERP|RF_TRANSLUCENT|RF_GLOW;
	sentry->solid = SOLID_BBOX;
	VectorSet(sentry->mins, -50,-20,-40);
	VectorSet(sentry->maxs, 30, 21, 22);
	sentry->takedamage=DAMAGE_YES;
	sentry->s.modelindex = gi.modelindex (SENTRY1_MODEL);
	sentry->creator = ent;
	sentry->sentrydelay = level.time + 1;
	sentry->standowner = ent;
	sentry->think = Turret_Think;
	sentry->nextthink = level.time + DELAY_SENTRY_SCAN;
	sentry->touch = Sentry_Touch;
	sentry->die = turret_die;
	sentry->health = SENTRY_INITIAL_HEALTH + SENTRY_ADDON_HEALTH * ent->myskills.build_sentry;
	sentry->max_health = sentry->health;

	sentry->count = 1;
	//sentry->s.sound = gi.soundindex ("weapons/rg_hum.wav");
	sentry->attenuation =1;
	sentry->volume = 0.5;
	sentry->classname = "SentryGun";

	sentry->s.effects |= EF_COLOR_SHELL;
//	sentry->s.renderfx |= (RF_SHELL_BLUE);
	sentry->flags &= FL_NO_KNOCKBACK;

	if (ctf->value){
		if (ent->client->resp.ctf_team == 1)
			sentry->s.renderfx |= (RF_SHELL_RED);
		else
			sentry->s.renderfx |= (RF_SHELL_BLUE);
	}
	//else
		//sentry->s.renderfx |= (RF_SHELL_BLUE);

	sentry->yaw_speed = 15;
	sentry->gib_health = SENTRY_INITIAL_AMMO + SENTRY_ADDON_AMMO * ent->myskills.build_sentry;//Max Ammo
	sentry->light_level = sentry->gib_health;//Ammo Total
	sentry->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
	sentry->monsterinfo.power_armor_power = SENTRY_INITIAL_ARMOR + SENTRY_ADDON_ARMOR * ent->myskills.build_sentry;
	sentry->delay=StatusIdle;

	gi.linkentity (sentry);


	sentrystand = G_Spawn();
	VectorClear (sentrystand->mins);
	VectorClear (sentrystand->maxs);
	VectorCopy (pos,sentrystand->s.origin);
	sentrystand->s.angles[0]=0;
	sentrystand->s.angles[YAW] = sentry->s.angles[YAW];
	sentrystand->movetype		= MOVETYPE_TOSS;
	sentrystand->mass = 400;
	sentrystand->solid = SOLID_NOT;
	VectorSet(sentrystand->mins, -45,-15,-40);
	VectorSet(sentrystand->maxs, 25, 18, 18);
	sentrystand->takedamage=DAMAGE_NO;
	sentrystand->health = 10;
	sentrystand->s.modelindex	= gi.modelindex ("models/stand/tris.md2");
	sentrystand->classname = "SentryStand";
	sentrystand->creator = sentry;
	sentrystand->standowner = ent;
	sentrystand->think = Stand_Think;
	sentrystand->nextthink = level.time + 0.5;
/*
	if (ctf->value){
		sentry->s.effects |= EF_COLOR_SHELL;
		if (ent->client->resp.ctf_team == 1)
			sentrystand->s.renderfx |= (RF_SHELL_RED);
		else
			sentrystand->s.renderfx |= (RF_SHELL_BLUE);
	}
*/		
	gi.linkentity (sentrystand);
	sentry->sentry = sentrystand;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/tnkatck4.wav"), 1, ATTN_NORM, 0);

	ent->num_sentries += 3;
	ent->holdtime = level.time + DELAY_SENTRY;

	//If you use this spell, you uncloak!
	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->cloaking = false;
	ent->client->cloakable = 0;
}

void UpgradeSentry(edict_t *self)
{
	edict_t *blip;
	trace_t		tr;
	qboolean found;

	blip = NULL;
	found = false;
	while (blip = findradius (blip, self->s.origin, 128))
	{
		if (Q_stricmp("SentryGun", blip->classname))
			continue;
        if (self->client->pers.inventory[power_cube_index] < SENTRY_UPGRADE)
		{//42 ebc
			gi.cprintf(self, PRINT_HIGH, "You need %d more power cubes to upgrade sentry gun.\n", (SENTRY_UPGRADE - self->client->pers.inventory[power_cube_index]));
			return;
		}

		found = true;

		tr = gi.trace (self->s.origin, NULL, NULL, blip->s.origin, self, MASK_SOLID);
		if (tr.fraction != 1.0)
			continue;
		if (blip->creator != self)
			continue;
		if (blip->count >= 3)//42 sbc
 			gi.cprintf(self, PRINT_HIGH, "Sentry gun already at level 3\n");

		if (blip->count < 3)
			blip->count++;
		
		self->client->pers.inventory[power_cube_index] -= SENTRY_UPGRADE;
		
		blip->health = SENTRY_INITIAL_HEALTH + SENTRY_ADDON_HEALTH * blip->creator->myskills.build_sentry;
		blip->max_health = blip->health;

		blip->gib_health = SENTRY_INITIAL_AMMO + SENTRY_ADDON_AMMO * blip->creator->myskills.build_sentry;//Max Ammo
		blip->light_level = blip->gib_health;//Ammo Total
		blip->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
		blip->monsterinfo.power_armor_power = SENTRY_INITIAL_ARMOR + SENTRY_ADDON_ARMOR * blip->creator->myskills.build_sentry;

		if (blip->count == 2)
		{

			blip->s.modelindex = gi.modelindex (SENTRY2_MODEL);
			blip->delay = StatusIdle;
			blip->s.frame = 0;
			blip->yaw_speed = 40;

		//	if (!ctf->value) {
			//	blip->s.renderfx &= ~RF_SHELL_GREEN;
				blip->s.renderfx |= (RF_SHELL_RED|RF_SHELL_BLUE);
		//	}
		}
		if (blip->count == 3)
		{

			blip->s.modelindex = gi.modelindex (SENTRY3_MODEL);
			blip->delay = StatusIdle;
			blip->s.frame = 0;			
			blip->yaw_speed = 45;

		//	if (!ctf->value) {
			//	blip->s.renderfx &= ~RF_SHELL_GREEN;
				//blip->s.renderfx &= ~RF_SHELL_BLUE;
			//	blip->s.renderfx |= (RF_SHELL_RED);
		//	}
		}

		gi.sound(self, CHAN_VOICE, gi.soundindex("misc/pc_up.wav"), 1, ATTN_NORM, 0);
	}

	if (found == false)//42 sbc
 		gi.cprintf(self, PRINT_HIGH, "Sorry, you aren't close enough.\n");
}

//test_Sentry() - see if there is a sentry close enough to work on

//Returns 0 if sentry does not exist or is too far away
//Returns 1 if sentry exists and is close enough
int test_Sentry (edict_t *ent)
{
	float dist;
	float tdist;
	edict_t *sentry;
	vec3_t distance;
	edict_t *blip;

	blip = NULL;
	dist = 0;
	sentry = NULL;
	while (blip = findradius (blip, ent->s.origin, 100))
	{
		if (strcmp(blip->classname, "SentryGun"))
			continue;	//Not a sentry gun
		if (blip->creator != ent)
			continue;

		distance[0]=ent->s.origin[0] - blip->s.origin[0];
		distance[1]=ent->s.origin[1] - blip->s.origin[1];
		distance[2]=ent->s.origin[2] - blip->s.origin[2];
		tdist=VectorLength(distance);
		if ((dist == 0) || (tdist < dist))
		{
			dist = tdist;
			sentry = blip;
		}
	}
	if(dist>100 || sentry == NULL)
	{
		gi.cprintf(ent, PRINT_HIGH, "Sentry too far away.\n"); 
		return 0;
	}
	else 
	{
		ent->selectedsentry = sentry;
		return 1;
	}

}

void cmd_Sentry (edict_t *ent)
{
	char    *string;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called cmd_Sentry()\n", ent->client->pers.netname);

	if ((deathmatch->value) && (level.time < 120)) {
		if (ent->client)
			gi.cprintf(ent, PRINT_HIGH, "You cannot use this ability in pre-game!\n");
		return;
	}

	string=gi.args();

	if (!ent->client) 
		return;
	if (ent->health <= 0)
		return;

	if (ent->myskills.build_sentry <= 0)
	{
		gi.cprintf(ent, PRINT_HIGH, "You can't make a sentry gun, you never trained in it!\n");
		return;
	}

	//argument = "build", "remove", "upgrade", "repair" and "reload"
	if (Q_stricmp ( string, "build") == 0)
	{
		place_turret(ent);
	}
	else if (Q_stricmp ( string, "remove") == 0)
	{
//		if (ent->sentry) 
		if (test_Sentry(ent))
			turret_remove(ent->selectedsentry);
//		else
//			gi.cprintf(ent, PRINT_HIGH, "Sentry not found.\n"); 
	}
	else if (Q_stricmp ( string, "upgrade") == 0)
	{
		if (test_Sentry(ent)) SentryUpgrade(ent);
	}
	else if (Q_stricmp ( string, "repair") == 0)
	{
		if (test_Sentry(ent)) SentryRepair(ent);
	}
	else if (Q_stricmp ( string, "reload") == 0)
	{
		if (test_Sentry(ent)) SentryReload(ent);
	}

	//Otherwise toggle on/off
	else if (Q_stricmp ( string, "") == 0)
	{
		place_turret(ent);
	}
	else
		gi.cprintf(ent, PRINT_HIGH, "Invalid sentry command.\n"); 

}
void MiniSentry_idle (edict_t *self)
{
	int max_yaw, min_yaw, max_gain, gain;
	//gi.dprintf("DEBUG: %f YAW\n", self->s.angles[YAW]);

	max_yaw = self->move_angles[YAW] + 45;
	min_yaw = self->move_angles[YAW] - 45;
			
	//GHz: Make sentry scan side to side
	if (self->style)
	{
		if (self->s.angles[YAW] < max_yaw)
			self->s.angles[YAW] += 5;
		else
			self->style = 0;
		if (self->s.angles[YAW] > max_yaw)
			self->s.angles[YAW] = max_yaw;
	}
	else
	{		
		if (self->s.angles[YAW] > min_yaw)
			self->s.angles[YAW] -= 5;
		else
			self->style = 1;
		if (self->s.angles[YAW] < min_yaw)
			self->s.angles[YAW] = min_yaw;
	}
	//GHz: Move to original angles
	if (self->s.angles[YAW] > max_yaw || self->s.angles[YAW] < min_yaw || self->s.angles[PITCH] != self->move_angles[PITCH])
		VectorCopy(self->move_angles, self->s.angles);
	//GHz: Play gun idle sound every few seconds
	if (self->sentrydelay + 5.0 <= level.time){
		gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/gunidle1.wav"), 0.1, ATTN_NORM, 0);
		self->sentrydelay = level.time;
	}

	if (self->delay > level.time)
	{
		// regenerate health, armor, and ammo
		gain = 0.04 * self->max_health;
		if (self->health < self->max_health)
			self->health += gain;
		if (self->health > self->max_health)
			self->health = self->max_health;
		max_gain = MINISENTRY_INITIAL_ARMOR + MINISENTRY_ADDON_ARMOR * self->creator->myskills.build_sentry;
		gain = 0.04 * max_gain;
		if (self->monsterinfo.power_armor_power < max_gain)
			self->monsterinfo.power_armor_power += gain;
		if (self->monsterinfo.power_armor_power > max_gain)
			self->monsterinfo.power_armor_power = max_gain;
		max_gain = MINISENTRY_INITIAL_AMMO + MINISENTRY_ADDON_AMMO * self->creator->myskills.build_sentry;
		gain = 0.04 * max_gain;
		if (self->light_level < max_gain)
			self->light_level += gain;
		if (self->light_level > max_gain)
			self->light_level = max_gain;
		self->delay = level.time + 1.0;
		//gi.dprintf("DEBUG: MiniSentry (%d) health (%d) armor (%d/%d) ammo\n", self->health, self->monsterinfo.power_armor_power, self->light_level, max_gain);
	}
}

void MiniSentrySetEffects (edict_t *self)
{
	//self->s.effects &= ~(EF_COLOR_SHELL|EF_POWERSCREEN);
//	self->s.renderfx &= ~(RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE|RF_SHELL_YELLOW);
	self->s.effects = 0;
	self->s.renderfx = 0;

	if (self->aura && self->auratime > level.time)
	{
		self->s.effects |= EF_COLOR_SHELL;
		self->s.renderfx |= RF_SHELL_CYAN;
	}
	if (ctf->value){
		self->s.effects |= EF_COLOR_SHELL;
		if (self->creator->client->resp.ctf_team == 1)
			self->s.renderfx |= RF_SHELL_RED;
		else
			self->s.renderfx |= RF_SHELL_BLUE;
	}
}

void MiniSentry_think (edict_t *self)
{
	int damage, speed, contents;
	vec3_t angle_to_enemy, point_to_hit, forward, start;
	vec3_t up, end;
	edict_t *blip = NULL;
	trace_t tr;

//	gi.dprintf("DEBUG: Began MiniSentry_think()\n");

	/*
	//GHz: Self-destruct if we have no owner
	if (!self->creator || !self->creator->inuse || self->creator->health <= 0){
		self->creator->num_sentries = 0;
		BecomeExplosion1(self);
	}
	*/

	//GHz: Remove a sentry that gets stuck in a solid object
	contents = (int)gi.pointcontents(self->s.origin);
	if (contents & CONTENTS_SOLID)
	{
		gi.dprintf("VRX: Removed sentry from solid object!\n");
		if (self->creator){
			self->creator->num_sentries--;
			if (self->creator->num_sentries < 0)
				self->creator->num_sentries = 0;
		}
		BecomeExplosion1(self);
		return;
	}

	//GHz: Try to stick to ceiling, otherwise just fall
	VectorCopy(self->s.origin, start);
	start[2] += 30;
	AngleVectors (self->move_angles, NULL, NULL, up);
	VectorMA(start, 50, up, end);
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);
	if (tr.fraction != 1.0 && !tr.ent->takedamage)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
		gi.WriteShort (self - g_edicts);
		gi.WritePosition (start);
		gi.WritePosition (end);
		gi.multicast (start, MULTICAST_PVS);

		self->movetype = MOVETYPE_NONE;
	}
	else if (!self->groundentity && (self->movetype == MOVETYPE_NONE))
		self->movetype = MOVETYPE_TOSS;

	//GHz: If we were just spawned, wait a bit before coming active
	if (self->PlasmaDelay >= level.time){
		self->nextthink = level.time + FRAMETIME;
		return;
	}
	MiniSentrySetEffects(self);

	if (!self->enemy)
	{
	//	gi.dprintf("DEBUG: looking for a target...\n");
		//GHz: Find a valid target
		while ((blip = findradius(blip, self->s.origin, 512)) != NULL)
		{
			if (!G_EntIsAlive(blip))
				continue;
			if (!visible(self, blip))
				continue;
			if (!infront(self, blip))
				continue;
			if (blip->cursed & CURSE_FROZEN)
				continue;
			if (OnSameTeam(self, blip))
				continue;
			if (blip == self)
				continue;

			//GHz: Only attack spree dude and his summonables
			if (SPREE_WAR == true && self->creator != SPREE_DUDE) {
				if (blip->client && blip != SPREE_DUDE)
					continue;
				if (blip->creator && blip->creator != SPREE_DUDE)
					continue;
				if (blip->activator && blip->activator != SPREE_DUDE)
					continue;
			}

			//If invincible, skip for now
			if (blip->client && blip->client->invincible_framenum > level.framenum)
				continue;

			self->enemy = blip;
			gi.sound (self, CHAN_VOICE, gi.soundindex("weapons/turrspot.wav"), 1, ATTN_NORM, 0);
		//	gi.dprintf("DEBUG: MiniSentry_think() found a target\n");
		}	
		//GHz: We found no target
		if (blip == NULL)
			MiniSentry_idle(self);
	}
	else if (G_EntIsAlive(self->enemy) && visible(self, self->enemy))
	{
		//gi.dprintf("DEBUG: target %s class %d health\n", self->enemy->classname, self->enemy->health);
		//GHz: Get starting position in front of gun
		AngleVectors (self->s.angles, forward, NULL, NULL);
		VectorMA(self->s.origin, 25, forward, start);
		start[2] += 8;

		//GHz: Calculate angle to enemy
		VectorCopy(self->enemy->s.origin, point_to_hit);
		point_to_hit[2] += self->enemy->viewheight;
		VectorSubtract(point_to_hit, self->s.origin, angle_to_enemy);
		VectorNormalize(angle_to_enemy);
		vectoangles(angle_to_enemy, angle_to_enemy);
		VectorCopy(angle_to_enemy, self->s.angles);

		AngleVectors (angle_to_enemy, forward, NULL, NULL);
	
		//GHz: Attack if enemy is in front of us and we have ammo
		if (infront(self, self->enemy) && self->light_level){
			damage = 5 + 0.25 * self->creator->myskills.build_sentry;
			self->light_level--;
			fire_bullet(self, start, forward, damage, damage, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_SENTRY);
			gi.sound (self, CHAN_VOICE, gi.soundindex("weapons/plaser.wav"), 1, ATTN_NORM, 0);
			
			gi.WriteByte (svc_muzzleflash2);
			gi.WriteShort (self - g_edicts);
			gi.WriteByte (MZ2_TANK_MACHINEGUN_1);
			gi.multicast (start, MULTICAST_PVS);
			

			//GHz: Fire a rocket every second
			if (self->lasthbshot + 2.0 <= level.time)
			{
				damage = 50 + 5 * self->creator->myskills.build_sentry;
				speed = 600 + 20 * self->creator->myskills.build_sentry;

				MonsterAim(self, 0.95, speed, true, MZ2_TANK_ROCKET_1, forward, start);
				fire_rocket (self, start, forward, damage, speed, damage, damage);
				
				gi.WriteByte (svc_muzzleflash2);
				gi.WriteShort (self - g_edicts);
				gi.WriteByte (MZ2_TANK_ROCKET_1);
				gi.multicast (start, MULTICAST_PVS);
				
				self->lasthbshot = level.time;
				
			}
			//gi.dprintf("DEBUG: Finished attack sequence\n");
		}
	}
	else
	{
		//gi.dprintf("DEBUG: Set NULL enemy\n");
		self->enemy = NULL;
	}

	self->velocity[0] *= 0.8;
	self->velocity[1] *= 0.8;

	self->nextthink = level.time + FRAMETIME;
	//gi.dprintf("DEBUG: MiniSentry_think() end\n");
}
void MiniSentry_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other && other->client && self->creator && self->creator == other){
		self->s.effects |= EF_PLASMA;
		self->PlasmaDelay = level.time + 0.5;
		
		//GHz: Rotate the sentry
		self->s.angles[YAW] += 2;
		if(self->s.angles[YAW] < 0)
			self->s.angles[YAW] += 360;
		else if(self->s.angles[YAW] > 360)
			self->s.angles[YAW] -= 360;
		self->move_angles[YAW] = self->s.angles[YAW];
	}
	//GHz: Become immobile once we have settled down
	if (self->movetype != MOVETYPE_NONE && self->groundentity)
		self->movetype = MOVETYPE_NONE;
}
void FL_make(edict_t *self);

void RemoveMiniSentry (edict_t *self)
{
	if (self->creator && self->creator->inuse){
		self->creator->num_sentries--;
	if (self->creator->num_sentries < 0)
		self->creator->num_sentries = 0;
	if (self->creator->client)
		gi.cprintf(self->creator, PRINT_HIGH, "Your mini sentry was destroyed. %d remaining\n", self->creator->num_sentries);
	}
	BecomeExplosion1(self);
}
void MiniSentry_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	//gi.dprintf("DEBUG: MiniSentry_die() called\n");
	self->takedamage = DAMAGE_NO;
	RemoveMiniSentry(self);
}

void BuildMiniSentry (edict_t *ent)
{
	char *string;
	edict_t *sentry, *scan = NULL;
	vec3_t forward, right, start, offset, end;
	trace_t tr;
	
	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called BuildMiniSentry()\n", ent->client->pers.netname);

	string = gi.args();

	if (level.time <= 120)
	{
		gi.cprintf(ent, PRINT_HIGH, "You can't use this ability in pre-game!\n", MaxSentries);
		return;
	}
	if (ent->myskills.build_sentry < 1)
	{
		gi.cprintf(ent, PRINT_HIGH, "You have not upgraded sentry.\n", MaxSentries);
		return;
	}
	if (Q_stricmp(string, "remove") == 0)
	{
		//GHz: Remove mini sentries
		while((scan = G_Find(scan, FOFS(classname), "MiniSentry")) != NULL){
		if (scan && scan->creator && scan->creator->client && (scan->creator == ent)){
			ent->num_sentries = 0;
			BecomeExplosion1(scan);
			} 
		}
		return;
	}
	if (ent->num_sentries >= MaxSentries)
	{
		gi.cprintf(ent, PRINT_HIGH, "You have reached the max of %d sentries\n", MaxSentries);
		return;
	}
	if (ent->client->pers.inventory[power_cube_index] < SENTRY_COST) {
		gi.cprintf(ent, PRINT_HIGH, "You need %d more power cubes\n", (SENTRY_COST - ent->client->pers.inventory[power_cube_index]));
		return;
	}
	if (ctf->value)
	{
		while ((scan = findradius(scan, ent->s.origin, 1024)) != NULL)
		{
			if (ent->client->resp.ctf_team == CTF_TEAM1 && !strcmp(scan->classname, "item_flag_team2")
				&& !(scan->spawnflags & DROPPED_ITEM)){
				gi.cprintf(ent, PRINT_HIGH, "Building sentries in opponents base is not allowed!\n");
				return;
			}
			if (ent->client->resp.ctf_team == CTF_TEAM2 && !strcmp(scan->classname, "item_flag_team1")
				&& !(scan->spawnflags & DROPPED_ITEM)){
				gi.cprintf(ent, PRINT_HIGH, "Building sentries in opponents base is not allowed!\n");
				return;
			}

		}
	}
	if (ent->client->ability_delay > level.time) {
		gi.cprintf (ent, PRINT_HIGH, "You can't use abilities for another %2.1f seconds\n", ent->client->ability_delay - level.time);
		return;
	}
	ent->client->ability_delay = level.time + 1.0;

	//GHz: Setup basic entity and related functions
	sentry = G_Spawn();
	sentry->creator = ent;
	sentry->owner = NULL;
	VectorCopy(ent->s.angles, sentry->s.angles);
	VectorCopy(ent->s.angles, sentry->move_angles);
	sentry->s.angles[PITCH] = 0;
	sentry->move_angles[PITCH] = 0;
	sentry->think = MiniSentry_think;
	sentry->die = MiniSentry_die;
	sentry->nextthink = level.time + FRAMETIME;
	sentry->PlasmaDelay = level.time + 3.0;//GHz: Time till sentry starts attacking
	sentry->s.modelindex = gi.modelindex ("models/weapons/g_bfg/tris.md2");
	sentry->solid = SOLID_BBOX;
	sentry->movetype = MOVETYPE_TOSS;
	sentry->clipmask = MASK_PLAYERSOLID;
	sentry->mass = 250;
	sentry->health = MINISENTRY_INITIAL_HEALTH + MINISENTRY_ADDON_HEALTH * ent->myskills.build_sentry;
	sentry->max_health = sentry->health;
	sentry->light_level = MINISENTRY_INITIAL_AMMO + MINISENTRY_ADDON_AMMO * ent->myskills.build_sentry;//GHz: Ammo
	sentry->classname = "MiniSentry";
	sentry->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
	sentry->monsterinfo.power_armor_power = MINISENTRY_INITIAL_ARMOR + MINISENTRY_ADDON_ARMOR * ent->myskills.build_sentry;
	sentry->takedamage = DAMAGE_YES;
	sentry->touch = MiniSentry_touch;
	sentry->viewheight = 16;
	VectorClear(sentry->mins);
	VectorClear(sentry->maxs);
	//GHz: Set bbox (width, length, height)
	VectorSet(sentry->mins, -16, -16, 0);
	VectorSet(sentry->maxs, 16, 24, 24);

	gi.linkentity(sentry);

	//GHz: Calculate end point and trace
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	VectorMA(start, 50, forward, end);
	tr = gi.trace(end, sentry->mins, sentry->maxs, end, ent, MASK_SHOT);

	//GHz: Dont build into a wall
	if (tr.contents & MASK_SHOT){
		gi.cprintf(ent, PRINT_HIGH, "Failed to build sentry.\n");
		G_FreeEdict(sentry);
		return;
	}
	else
		VectorCopy(tr.endpos, sentry->s.origin);

	sentry->s.effects |= EF_PLASMA;
	if (ctf->value) {
		sentry->s.effects |= EF_COLOR_SHELL;
		if (ent->client->resp.ctf_team == 1)
			sentry->s.renderfx |= (RF_SHELL_RED);
		else
			sentry->s.renderfx |= (RF_SHELL_BLUE);
	}

	ent->client->pers.inventory[power_cube_index] -= SENTRY_COST;
	ent->num_sentries++;

    gi.sound(sentry, CHAN_VOICE, gi.soundindex("weapons/turrset.wav"), 1, ATTN_NORM, 0);

	//GHz: Hold us while we create our sentry
	ent->holdtime = level.time + 1.0;

	//If you use this spell, you uncloak!
	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->cloaking = false;
	ent->client->cloakable = 0;

}

void mysentry_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	// blow up
	BecomeExplosion1(self);
}

void mysentry_think (edict_t *self)
{
	edict_t *target=NULL;
	vec3_t	v;
	
	// if we dont have an enemy, find one
	if (!self->enemy)
	{
		while ((target = findclosestradius(target, self->s.origin, 512)) != NULL)
		{
			if (!G_EntIsAlive(target))
				continue;
			if (OnSameTeam(self->creator, target))
				continue;
			if (!visible(self, target))
				continue;
			self->enemy = target;
		}
	}
	// make sure our enemy is still a valid target
	else if (G_EntIsAlive(self->enemy) && !OnSameTeam(self, self->enemy) && visible(self, self->enemy))
	{
		VectorSubtract(self->enemy->s.origin, self->s.origin, v);
		VectorNormalize(v);
		vectoangles(v, self->s.angles);
	}
	// otherwise set our enemy to NULL and copy our original angles back
	else
	{
		self->enemy = NULL;
		VectorCopy(self->move_angles, self->s.angles);
	}

	self->nextthink = level.time + FRAMETIME;	
}

void SpawnSentry (edict_t *ent)
{
	vec3_t	forward, end;
	edict_t *sentry;
	trace_t	tr;

	sentry = G_Spawn();
	sentry->creator = ent;
	sentry->classname = "mysentrygun";
	sentry->movetype = MOVETYPE_TOSS;
	sentry->clipmask = MASK_MONSTERSOLID;
	sentry->solid = SOLID_BBOX;
	sentry->mass = 300;
	sentry->takedamage = DAMAGE_YES;
	VectorSet(sentry->mins, -16, -16, 0);
	VectorSet(sentry->maxs, 16, 24, 24);
	sentry->s.modelindex = gi.modelindex ("models/weapons/g_bfg/tris.md2");
	sentry->viewheight = 16;
	VectorCopy(ent->s.angles, sentry->s.angles);
	VectorCopy(ent->s.angles, sentry->move_angles);
	sentry->s.angles[PITCH] = 0;
	sentry->move_angles[PITCH] = 0;
	sentry->think = mysentry_think;
	sentry->die = mysentry_die;
	sentry->health = 500;
	sentry->max_health = sentry->health;
	sentry->nextthink = level.time + FRAMETIME;

	// calculate starting position
	AngleVectors (ent->client->v_angle, forward, NULL, NULL);
	VectorMA(ent->s.origin, 48, forward, end);
	tr = gi.trace(ent->s.origin, sentry->mins, sentry->maxs, end, ent, MASK_SHOT);
	if (tr.contents & MASK_SHOT)
	{
		G_FreeEdict(sentry);
		return;
	}
	VectorCopy(tr.endpos, sentry->s.origin);
	gi.linkentity(sentry);
}
