#include "g_local.h"

//Function Prototypes required for this .c file:
void PlayerBossPoints (edict_t *attacker, edict_t *target);

int mypower(int x, int y)
{
	if (y > 0)
	{
		y--;
		return (x * mypower(x,y) );
	}
	return (1);
}

char *LoPrint(char *text)
{
	int i;

	if (!text)
		return NULL;
	for (i=0; i<strlen(text) ; i++)
		if ((byte)text[i] > 127)
			text[i]=(byte)text[i]-128;

	return text;
}

char *HiPrint(char *text)
{
	int i;

	if (!text)
		return NULL;
	for (i=0; i<strlen(text) ; i++)
		if ((byte)text[i] <= 127)
			text[i]=(byte)text[i]+128;
	return text;
}

// this needs to match UpdateFreeAbilities() in v_utils.c
void NewLevel_Addons(edict_t *ent)
{
	if ((ent->myskills.level % 5) == 0)
	{
		if (ent->myskills.abilities[MAX_AMMO].level < ent->myskills.abilities[MAX_AMMO].max_level)
		{
			ent->myskills.abilities[MAX_AMMO].level++;
			ent->myskills.abilities[MAX_AMMO].current_level++;
		}
		else ent->myskills.speciality_points++;

		if (ent->myskills.abilities[VITALITY].level < ent->myskills.abilities[VITALITY].max_level)
		{
			ent->myskills.abilities[VITALITY].level++;
			ent->myskills.abilities[VITALITY].current_level++;
		}
		else ent->myskills.speciality_points++;
	}

	// free ID at level 5
	if (ent->myskills.level == 5)
	{
		if (!ent->myskills.abilities[ID].level)
		{
			ent->myskills.abilities[ID].level++;
			ent->myskills.abilities[ID].current_level++;
		}
		else
			ent->myskills.speciality_points += ent->myskills.abilities[ID].level*2; // give back points spent on ID.
	}

	// free scanner at level 10
	if (ent->myskills.level == 10)
	{
		if (!ent->myskills.abilities[SCANNER].level)
		{
			ent->myskills.abilities[SCANNER].level++;
			ent->myskills.abilities[SCANNER].current_level++;
		}
		else
			ent->myskills.speciality_points += 2;
	}

    //Give the player talents if they are eligible.
	//if(ent->myskills.level >= TALENT_MIN_LEVEL && ent->myskills.level <= TALENT_MAX_LEVEL)
	if (ent->myskills.level > 1 && !(ent->myskills.level % 2)) // Give a talent point every two levels.
		ent->myskills.talents.talentPoints++;
}

void Add_credits(edict_t *ent, int targ_level)
{
	int level_diff		= 0;
	int credit_points	= 0;
	float temp			= 0.0;

	if (!ent->client)
		return;

	if (targ_level > ent->myskills.level)
	{
		level_diff = targ_level - ent->myskills.level;
		credit_points = GetRandom(CREDIT_HIGH, CREDIT_HIGH*2) + CREDIT_HIGH*level_diff;
	}
	else if (targ_level < ent->myskills.level)
	{
		level_diff = ent->myskills.level - targ_level;
		credit_points = (GetRandom(CREDIT_LOW, CREDIT_LOW*2) + CREDIT_LOW)/level_diff;
	}
	else credit_points = GetRandom(CREDIT_SAME, CREDIT_SAME*5);	//3.0 changed from: credit_points = CREDIT_SAME
	
	if (credit_points < 0)
		credit_points = 1;

	if (ent->myskills.streak >= SPREE_START)//GHz
	{
		temp = (ent->myskills.streak + 1 - SPREE_START) * SPREE_BONUS;
		if (temp > 4) temp = 4;
		if (ent->myskills.streak >= SPREE_WARS_START && SPREE_WARS > 0)
			temp *= SPREE_WARS_BONUS;
		credit_points *= 1 + temp;
	}

	if (credit_points > 100)
		credit_points = 100;

	credit_points *= vrx_creditmult->value;

	if (ent->myskills.credits+credit_points > MAX_CREDITS)
	{
		gi.cprintf(ent, PRINT_HIGH, "Maximum credits reached!\n");
		ent->myskills.credits = MAX_CREDITS;
	}
	else
		ent->myskills.credits += credit_points;

//	gi.dprintf("%s was awarded with %d credits\n", ent->client->pers.netname, credit_points);
}


gitem_t	*GetWeaponForNumber(int i)
{
	switch (i)
	{
	case 2 :
		return Fdi_SHOTGUN;
	case 3 :
		return Fdi_SUPERSHOTGUN;
	case 4 :
		return Fdi_MACHINEGUN;
	case 5 :
		return Fdi_CHAINGUN;
	case 6 :
		return Fdi_GRENADES;
	case 7 :
		return Fdi_GRENADELAUNCHER;
	case 8 :
		return Fdi_ROCKETLAUNCHER;
	case 9 :
		return Fdi_HYPERBLASTER;
	case 10 :
		return Fdi_RAILGUN;
	case 11 :
		return Fdi_BFG;
	}
	return Fdi_ROCKETLAUNCHER;
}

void check_for_levelup (edict_t *ent)
{
	double points_needed;
	int plateau_points = 50000;
	qboolean levelup = false;
	char *message;
	while (ent->myskills.experience >= ent->myskills.next_level)
	{
		levelup = true;

		// maximum level cap
		if (!ent->myskills.administrator && ent->myskills.level >= 30) // cap to 30
		{
			ent->myskills.next_level = ent->myskills.experience;
			return;
		}

		//Need between 80 to 120% more points for the next level.
		ent->myskills.level++;
		points_needed = start_nextlevel->value * pow(nextlevel_mult->value, ent->myskills.level);

		// the experience required to reach next level reaches a plateau
		if (points_needed > plateau_points)
		{
			// determine level where plateau_points is reached
			int plateau_level = (int)ceil(log(plateau_points / start_nextlevel->value) / log(nextlevel_mult->value));

			// calculate next level points based
			points_needed = plateau_points + 5000*(ent->myskills.level-plateau_level);
		}

		ent->myskills.next_level += points_needed;

        ent->myskills.speciality_points += 2;
		if (ent->myskills.class_num != CLASS_WEAPONMASTER)
			ent->myskills.weapon_points += 4;
		else
			ent->myskills.weapon_points += 6;

		NewLevel_Addons(ent);//Add any special addons that should be there!
		modify_max(ent);

		message = HiPrint(va("*****%s gained a level*****", ent->client->pers.netname));
		gi.bprintf(PRINT_HIGH, "%s\n", message);
		message = LoPrint(message);
		WriteToLogfile(ent, va("Player reached level %d\n", ent->myskills.level));

		// maximum level cap
		if (!ent->myskills.administrator && ent->myskills.level >= 30)
		{
			ent->myskills.next_level = ent->myskills.experience;
			break;
		}
	}

	if (levelup)
	{
		gi.centerprintf(ent, "Welcome to level %d!\n You need %d experience \nto get to the next level.\n", 
			ent->myskills.level, ent->myskills.next_level-ent->myskills.experience);
		gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/pc_up.wav"), 1, ATTN_STATIC, 0);
	}
}

int VortexAddCredits(edict_t *ent, float level_diff, int bonus, qboolean client)
{
	int add_credits		= 0;
	int streak;

	streak = ent->myskills.streak;

	if (client)
		add_credits = level_diff * (vrx_creditmult->value * vrx_pvpcreditmult->value * (CREDITS_PLAYER_BASE + streak));
	else
		add_credits = level_diff * (vrx_creditmult->value * vrx_pvmcreditmult->value * (CREDITS_OTHER_BASE));

	// vrxchile v1.5 no credit cap
	//if (add_credits > 250)
	//	add_credits = 250;

	add_credits += bonus;

	//FIXME: remove this after allocating more space
	if (ent->myskills.credits+add_credits > MAX_CREDITS)
		ent->myskills.credits = MAX_CREDITS;
	else
		ent->myskills.credits += add_credits;

	return add_credits;
}

void VortexSpreeAbilities (edict_t *attacker)
{
	// NOTE: Spree MUST be incremented before calling this function
	// otherwise the player will keep getting 10 seconds of quad/invuln
	
	//New quad/invin duration variables
	int base_duration = 50;	//5 seconds
	int kill_duration = 5;	//0.5 seconds per kill

	if (!attacker->client)
		return;
	if (attacker->myskills.streak < 6)
		return;
	if (HasFlag(attacker))
		return;
/*
    //Talent: Longer Powerups
	if(getTalentSlot(attacker, TALENT_LONG_POWERUPS) != -1)
	{
		int level = getTalentLevel(attacker, TALENT_LONG_POWERUPS);
		int baseBonus, killBonus;

		switch(level)
		{
		case 1:		baseBonus = 0;		killBonus = 10;		break;
		case 2:		baseBonus = 0;		killBonus = 15;		break;
		case 3:		baseBonus = 0;		killBonus = 20;		break;
		default:	baseBonus = 0;		killBonus = 0;		break;
		}

		base_duration += baseBonus;
		kill_duration += killBonus;
	}
*/
	// special circumstances if they have both create quad and create invin
	if ((attacker->myskills.abilities[CREATE_QUAD].current_level > 0) 
		&& (attacker->myskills.abilities[CREATE_INVIN].current_level > 0))
	{  
		// if they already have it, give them another second
		if (attacker->client->quad_framenum > level.framenum)
			attacker->client->quad_framenum += kill_duration;
		else if (attacker->client->invincible_framenum > level.framenum)
			attacker->client->invincible_framenum += kill_duration;
		else if (!(attacker->myskills.streak % 10))
		{
			// give them quad OR invin, not both!
			if (random() > 0.5)	
				attacker->client->quad_framenum = level.framenum + base_duration;
			else
				// (apple)
				// invincible_framenum would add up level.framenum + base_duration
				attacker->client->invincible_framenum = level.framenum + base_duration;
		}
	}
	// does the attacker have create quad?
	else if (attacker->myskills.abilities[CREATE_QUAD].current_level > 0)
	{
		if(!attacker->myskills.abilities[CREATE_QUAD].disable)
		{
			// every 10 frags, give them 5 seconds of quad
			if (!(attacker->myskills.streak % 10))
				attacker->client->quad_framenum = level.framenum + base_duration;
			// if they already have quad, give them another second
			else if (attacker->client->quad_framenum > level.framenum)
				attacker->client->quad_framenum += kill_duration;
		}
	}
	// does the attacker have create invin?
	else if (attacker->myskills.abilities[CREATE_INVIN].current_level > 0)
	{
		if(!attacker->myskills.abilities[CREATE_INVIN].disable)
		{
			// every 10 frags, give them 5 seconds of invin
			if (!(attacker->myskills.streak % 10))
				attacker->client->invincible_framenum = level.framenum + base_duration;
			// if they already have invin, give them another second
			else if (attacker->client->invincible_framenum > level.framenum)
				attacker->client->invincible_framenum += kill_duration;
		}
	}	
}

#define PLAYTIME_MIN_MINUTES		999.0	// minutes played before penalty begins
#define PLAYTIME_MAX_MINUTES		999.0	// minutes played before max penalty is reached
#define PLAYTIME_MAX_PENALTY		2.0		// reduce experience in half

int V_AddFinalExp (edict_t *player, int exp)
{
	float	mod, playtime_minutes;
	
	// reduce experience as play time increases
	playtime_minutes = player->myskills.playingtime/60.0;
	if (playtime_minutes > PLAYTIME_MIN_MINUTES)
	{
		mod = 1.0 + playtime_minutes/PLAYTIME_MAX_MINUTES;
	if (mod >= PLAYTIME_MAX_PENALTY)
		mod = PLAYTIME_MAX_PENALTY;
	exp /= mod;
	}

	// player must pay back "hole" experience first
	if (player->myskills.nerfme > 0)
	{
		if (player->myskills.nerfme > exp)
		{
			player->myskills.nerfme -= exp;
			return 0;
		}

		exp -= player->myskills.nerfme;
	}

	if (player->myskills.level < 30) // hasn't reached the cap
		player->myskills.experience += exp;
	player->client->resp.score += exp;
	check_for_levelup(player);

	return exp;
}

#define EXP_SHARED_FACTOR				0.5
#define PLAYER_MONSTER_MIN_PLAYERS		4

void AddMonsterExp (edict_t *player, edict_t *monster)
{
	int		base_exp, exp_points, max_exp, control_cost;
	float	level_diff, bonus;
	edict_t	*e;

	if (!player->client)
		return;
	// don't award points to spectators
	if (G_IsSpectator(player))
		return;
	// don't award points to dead players
	if (player->health < 1)
		return;
	// don't award points to players in chat-protect mode
	if (player->flags & FL_CHATPROTECT)
		return;
	// don't award points for monster that was just resurrected
	if (monster->monsterinfo.resurrected_time > level.time)
		return;

	// don't award exp for player-monsters
	if (PM_MonsterHasPilot(monster))
		return;

	// is this a world-spawned monster or a player-spawned monster?
	e = G_GetClient(monster);
	if (e)
	{
		if (total_players() < PLAYER_MONSTER_MIN_PLAYERS)
			return; // don't award player-spawned monster exp if there are too few players
		base_exp = EXP_PLAYER_MONSTER;
	}
	else
		base_exp = EXP_WORLD_MONSTER;

	// reduce experience for invasion mode because of secondary objective
	if (INVASION_OTHERSPAWNS_REMOVED)
		base_exp *= 0.5;

	// increase experience for monsters in FFA since it is harder to stay alive
	// if (ffa->value)
	//	base_exp *= 1.5;

	level_diff = (float) monster->monsterinfo.level / (player->myskills.level+1);

	control_cost = monster->monsterinfo.control_cost;
	if (control_cost < 1)
		control_cost = 1;

	// calculate spree bonus
	bonus = 1 + 0.04*player->myskills.streak;

	if (bonus > 2)
		bonus = 2;

	// calculate ally bonus
	if (allies->value && !numAllies(player))
	{
		if (e)
			bonus += 0.5*(float)numAllies(e);
	}
	
	exp_points = control_cost * (level_diff*base_exp);
	exp_points *= bonus;

	// cap max experience at 300% normal
	// vrxchile v1.3 Do not cap.
	/*max_exp = 3*base_exp*control_cost;
	if (exp_points > max_exp)
		exp_points = max_exp;*/

	// give your team some experience
	if (/*pvm->value ||*/ ((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		Add_ctfteam_exp(player, (int)(EXP_SHARED_FACTOR*exp_points));

	// give your allies some experience
	if (allies->value)
		AddAllyExp(player, exp_points);

	// add experience
	/*
	player->client->resp.score += exp_points;
	player->myskills.experience += exp_points;
	check_for_levelup(player);
	*/
	V_AddFinalExp(player, exp_points);

	// add credits
	VortexAddCredits(player, level_diff, 0, false);

	player->lastkill = level.time + 1.0;
	player->client->idle_frames = 0;

	// increment spree
	//if (pvm->value)
	if ((player->myskills.respawns & HOSTILE_MONSTERS) //4.5 PvM preference players only
		&& !(player->myskills.respawns & HOSTILE_PLAYERS))
	{
		player->myskills.streak++;
		VortexSpreeAbilities(player);
	}

	//gi.dprintf("AddMonsterExp(), %d exp, %d control_cost %d level\n", exp_points, control_cost, monster->monsterinfo.level);
}

void VortexAddExp(edict_t *attacker, edict_t *targ);
int PVM_TotalMonsters (edict_t *monster_owner);
void VortexAddMonsterExp(edict_t *attacker, edict_t *monster)
{
	int exp_points		= 0;
	int monsters		= 0;
	float level_diff	= 0;
	//char *message;
	//int i;
	//edict_t *player;

	if (IsABoss(monster))
	{
		PlayerBossPoints(attacker, monster);
		return;
	}

	if (IsABoss(attacker))
		return; // boss-player does not get awarded points!

	attacker = G_GetClient(attacker);

	//GHz: Abort if we have invalid data
	if (attacker == NULL || !monster->inuse /*|| !monster->activator || monster->activator->client*/)
		return;

	if (attacker->flags & FL_CHATPROTECT)
		return;

	//VortexSpreeAbilities(attacker);

	//GHz: Only award experience to one person if monster is not a boss
	if (monster->monsterinfo.control_cost <= 100)
	{
		if (INVASION_OTHERSPAWNS_REMOVED)
			INV_AwardMonsterKill(attacker, monster);
		else
			AddMonsterExp(attacker, monster);
/*
		//3.0 (If pvm was cleared of monsters, give everyone a small bonus)
		// search for other drones
		if(pvm->value)
		{
			edict_t *e = NULL;

			//find the monster spawn ent
			while((e = G_Find(e, FOFS(classname), "MonsterSpawn")) != NULL) 
			{
				if (e && e->inuse)
					break;
			}

			// get actual total world monsters
			if (e && e->inuse)
				monsters = PVM_TotalMonsters(e);

			//If the map is clear of monsters
			if ((e != NULL) && !monsters)
			{
				exp_points = GetRandom(25, 50);

				gi.dprintf("Giving players a PvM bonus (%d) killing all monsters.\n", exp_points, e->num_monsters);

				for_each_player(player, i)
				{
					if (player->solid == SOLID_NOT)
						continue;
					if (player->health < 0)
						continue;
					if (player->flags & FL_CHATPROTECT)
						continue;
					if (player->client->idle_frames > 100) //10 seconds of idling
						continue;

					player->client->resp.score += exp_points;
					player->myskills.experience += exp_points;
					check_for_levelup(player);
				}
			}
		}
*/

	}
	else
	{
		if (monster->mtype == M_JORG)//GHz: Award exp on Makron kill instead
			return;

		G_PrintGreenText(va("%s puts the smackdown on a world-spawned boss!", attacker->client->pers.netname));
		AwardBossKill(monster);
		/*
		SPREE_WAR = false;
		SPREE_DUDE = NULL;

		message = HiPrint(va("%s puts the smackdown on a world-spawned boss!", attacker->client->pers.netname));
		gi.bprintf(PRINT_HIGH, "%s\n", message);
		message = LoPrint(message);

		for_each_player(player, i)
		{
			if (player->solid == SOLID_NOT)
				continue;

			//GHz: Calculate level difference modifier
			level_diff = (float) (0.5*monster->monsterinfo.level) / (player->myskills.level + 1);

			exp_points = GetRandom(250, 500) * level_diff;
			if (exp_points > 1000)
				exp_points = 1000;

			V_AddFinalExp(player, exp_points);
			VortexAddCredits(player, level_diff, 0, false);
			
		}
		*/
	}
}

int PVP_AwardKill (edict_t *attacker, edict_t *targ, edict_t *target)
{
	int			max_points, clevel, base_exp;
	int			credits			= 0;
	int			exp_points		= 0;
	int			break_points	= 0;
	float		level_diff		= 0;
	float		bonus			= 1;
	float		dmgmod			= 1;
	float		damage;
	char		*message, name[50];
	int			minimum_points	= 1;
	qboolean	is_mini=false;
	
	// sanity check
	if (!attacker || !attacker->inuse || !attacker->client)
		return 0;

	if (attacker->client && targ->client && invasion->value > 1) // don't give exp for friendly fire in invasion hard mode
		return 0;

	// don't award points for monster that was just resurrected
	if (targ->monsterinfo.resurrected_time > level.time)
		return 0;

	name[0] = 0;// initialize

	// is this a mini-boss?
	if (IsNewbieBasher(targ))
	{
		is_mini = true;
		minimum_points = 75;
	}

	// calculate damage modifier
	damage = GetPlayerBossDamage(attacker, targ);
	if (damage < 1)
		return 0;
	dmgmod = damage / GetTotalBossDamage(targ);

	// calculate level difference modifier
	if (targ->client) // target is a player
		level_diff = (float) (target->myskills.level + 1) / (attacker->myskills.level + 1);
	else // target is a monster/summon
		level_diff = (float) (targ->monsterinfo.level + 1) / (attacker->myskills.level + 1);

	// calculate spree bonuses
	if (attacker->myskills.streak >= SPREE_START && !SPREE_WAR)
		bonus += 0.5;
	else if (attacker->myskills.streak >= SPREE_WARS_START && SPREE_WARS)
		bonus += 3.0;

	// players get extra points for killing an allied teammate
	// if they are not allied with anyone themselves
	if (allies->value && !numAllies(attacker) && target)
		bonus += (float) 0.5 * numAllies(target);

	// we killed another player
	if (targ->client) 
	{
		// spree break bonus points
		if (target->myskills.streak >= SPREE_START)
			break_points = 100;
		// you get the same bonus for killing a newbie basher as you would breaking a spree war
		else if (is_mini || (target->myskills.streak >= SPREE_WARS_START && SPREE_WARS))
			break_points = 200;

		// award 2fer bonus
		if (attacker->lastkill >= level.time)
		{
			
			if (attacker->nfer < 2)
				attacker->nfer = 2;
			else
				attacker->nfer++;

			bonus += 1 - ((attacker->lastkill - level.time) + 0.1) + attacker->nfer*25;
			attacker->myskills.num_2fers++;

			if (attacker->nfer == 4)
			{
				gi.sound(attacker, CHAN_VOICE, gi.soundindex("speech/threat.wav"), 1, ATTN_NORM, 0);
			}
			else if (attacker->nfer == 5)
			{
				gi.sound(attacker, CHAN_VOICE, gi.soundindex("speech/hey.wav"), 1, ATTN_NORM, 0);
			}
			else
			{
				gi.sound(target, CHAN_VOICE, gi.soundindex("speech/excellent.wav"), 1, ATTN_NORM, 0);
			}

		}else // We get a nfer, print the LAST kill that was done within the time frame.
		{
			if (attacker->nfer)
			{
				message = HiPrint(va("%s got a %d fer.", attacker->client->pers.netname, attacker->nfer));
				gi.bprintf(PRINT_HIGH, "%s\n", message);
				message = LoPrint(message);
			}
			attacker->nfer = 0;
		}

		base_exp = EXP_PLAYER_BASE;
		max_points = 250;

		// award credits for kill
		credits = dmgmod * VortexAddCredits(attacker, level_diff, 0, true);

		strcat(name, target->client->pers.netname);
		clevel = target->myskills.level;
	}
	// we killed something else
	else
	{
		base_exp = EXP_WORLD_MONSTER;
		max_points = 75;

		//4.5 monster bonus flags
		if (targ->monsterinfo.bonus_flags & BF_UNIQUE_FIRE 
			|| targ->monsterinfo.bonus_flags & BF_UNIQUE_LIGHTNING)
		{
			level_diff *= 15.0;
			max_points = 500;
		}
		else if (targ->monsterinfo.bonus_flags & BF_CHAMPION)
		{
			level_diff *= 3.0;
			max_points = 150;
		}

		if (targ->monsterinfo.bonus_flags & BF_GHOSTLY || targ->monsterinfo.bonus_flags & BF_FANATICAL
			|| targ->monsterinfo.bonus_flags & BF_BERSERKER || targ->monsterinfo.bonus_flags & BF_GHOSTLY
			|| targ->monsterinfo.bonus_flags & BF_STYGIAN)
			level_diff *= 1.5;
		
		// control cost bonus (e.g. tanks)
		if (targ->monsterinfo.control_cost > 33)
			level_diff *= (0.75 * targ->monsterinfo.control_cost) / 30;

		// award credits for kill
		credits = VortexAddCredits(attacker, (dmgmod * level_diff), 0, false);

		strcat(name, V_GetMonsterName(targ));
		clevel = targ->monsterinfo.level;

		// increment spree counter in invasion mode or if the attacker has PvM combat preferences in FFA mode
		if (invasion->value || (ffa->value && V_MatchPlayerPrefs(attacker, 1, 0)))
		{
			attacker->myskills.streak++;
			VortexSpreeAbilities(attacker);
		}
	}

	exp_points = dmgmod * (level_diff * (vrx_pointmult->value * (base_exp * bonus)) + break_points);

	if (targ->client) // chile v1.1: pvp has another value.
		exp_points *= vrx_pvppointmult->value;
	else if (ffa->value && !targ->client) // Nonplayer entities give more exp.
		exp_points *= vrx_pvmpointmult->value;

	//vrxchile v1.3 Don't cap.
	// min/max points awarded for a kill
	/*if (exp_points > max_points)
		exp_points = max_points;
	else */if (exp_points < minimum_points)
		exp_points = minimum_points;

	// award experience to allied players
	max_points = exp_points;
	if (invasion->value == 1 && attacker->myskills.level > 10) // 1/3 exp in easy invasion for level 10+.
		max_points *= 0.33;
	if (!allies->value || ((exp_points = AddAllyExp(attacker, max_points)) < 1))
	// award experience to non-allied players
		exp_points = V_AddFinalExp(attacker, max_points);

	gi.cprintf(attacker, PRINT_HIGH, "You dealt %.0f damage (%.0f%c) to %s (level %d), gaining %d experience and %d credits\n", 
		damage, (dmgmod * 100), '%', name, clevel, exp_points, credits);

	return exp_points;
}

void VortexAddExp(edict_t *attacker, edict_t *targ)
{
	int			i, exp_points;
	edict_t		*target, *player = NULL;

	// this is a player-monster boss
	if (IsABoss(attacker))
	{
		AddBossExp(attacker, targ);
		return; 
	}

	//if (ptr->value || pvm->value)
	//	return;

	if (domination->value)
	{
		dom_fragaward(attacker, targ);
		return;
	}

	if (ctf->value)
	{
		CTF_AwardFrag(attacker, targ);
		return;
	}

	attacker = G_GetClient(attacker);
	target = G_GetClient(targ);

	// world monster boss kill
	if (!target && targ->monsterinfo.control_cost >= 100)
	{
		if (targ->mtype == M_JORG)
			return;

		if (attacker)
			G_PrintGreenText(va("%s puts the smackdown on a world-spawned boss!", attacker->client->pers.netname));

		AwardBossKill(targ);
		return;
	}

	// sanity check
	//if (attacker == target || attacker == NULL || target == NULL || !attacker->inuse || !target->inuse)
	//	return;
	
	// make sure target is valid
	//if (!target || !target->inuse)
	//	return;

	// award experience and credits to anyone that has hurt the target
	for (i=0; i<game.maxclients; i++) 
	{
		player = g_edicts+1+i;

		// award experience and credits to non-spectator clients
		if (!player->inuse || G_IsSpectator(player) || player == target || player->flags & FL_CHATPROTECT)
			continue;

		PVP_AwardKill(player, targ, target);
	}

	// give your team some experience
	if ((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS))
	{
		exp_points = PVP_AwardKill(attacker, targ, target);
		Add_ctfteam_exp(attacker, (int)(0.5*exp_points));
		return;
	}
}

void VortexDeathCleanup(edict_t *attacker, edict_t *targ)
{
	int lose_points=0;
	float level_diff;
	char *message;

	if (IsABoss(attacker))
	{
		targ->myskills.streak = 0;
		return; // bosses don't get frags
	}

	attacker = G_GetClient(attacker);
	targ = G_GetClient(targ);

	//GHz: Ignore invalid data
	if (attacker == NULL || targ == NULL)
	{
		if (targ)
			targ->myskills.streak = 0;
		return;
	}

	//GHz: Handle suicides
	if (targ == attacker)
	{
		targ->myskills.streak = 0;
		targ->myskills.suicides++;
		
		// players don't lose points in PvM mode since it is easy to kill yourself
		if (!pvm->value)
		{
			lose_points = EXP_LOSE_SUICIDE * targ->myskills.level;

			// cap max exp lost at 50 points
			if (lose_points > 50)
				lose_points = 50;

			if (targ->client->resp.score - lose_points > 0) 
			{
				targ->client->resp.score -= lose_points;
				targ->myskills.experience -= lose_points;
			}
			else
			{	
				targ->myskills.experience -= targ->client->resp.score;
				targ->client->resp.score = 0;
			}
		}

		return;
	}
	
	if (invasion->value < 2)
	{
		targ->myskills.fragged++;

		attacker->myskills.frags++;
		attacker->client->resp.frags++;
		attacker->lastkill = level.time + 2;
	}

	if (!ptr->value && !domination->value && !pvm->value && !ctf->value
		&& (targ->myskills.streak >= SPREE_START))
	{
		//GHz: Reset spree properties for target and credit the attacker
		if (SPREE_WAR == true && targ == SPREE_DUDE)
		{
			SPREE_WAR = false;
			SPREE_DUDE = NULL;
			attacker->myskills.break_spree_wars++;
		}
		attacker->myskills.break_sprees++;
		gi.bprintf(PRINT_HIGH, "%s broke %s's %d frag killing spree!\n", attacker->client->pers.netname, targ->client->pers.netname, targ->myskills.streak);
	}

	targ->myskills.streak = 0;

	if (IsNewbieBasher(targ))
		gi.bprintf(PRINT_HIGH, "%s wasted a mini-boss!\n", attacker->client->pers.netname);

	level_diff = (float) (targ->myskills.level + 1) / (attacker->myskills.level + 1);
	// don't let 'em spree off players that offer no challenge!
	if (!(IsNewbieBasher(attacker) && (level_diff <= 0.5)))
	{
		attacker->myskills.streak++;
		VortexSpreeAbilities(attacker);
	}
	else
		return;

	if (ptr->value)
		return;
	if (domination->value)
		return;
	if (pvm->value)
		return;
	if (ctf->value)
		return;

	if (attacker->myskills.streak >= SPREE_START)
	{
		if (attacker->myskills.streak > attacker->myskills.max_streak)
			attacker->myskills.max_streak = attacker->myskills.streak;
		if (attacker->myskills.streak == SPREE_START)
			attacker->myskills.num_sprees++;

		if (attacker->myskills.streak == SPREE_WARS_START && SPREE_WARS > 0)
			attacker->myskills.spree_wars++;

		if ((attacker->myskills.streak >= SPREE_WARS_START) && SPREE_WARS && (!numAllies(attacker))
			&& !attacker->myskills.boss && !IsNewbieBasher(attacker))
		{
			if (SPREE_WAR == false)
			{
				SPREE_DUDE = attacker;
				SPREE_WAR = true;
				SPREE_TIME = level.time;
				gi.cprintf(attacker, PRINT_HIGH, "You have 2 minutes to war. Get as many frags as you can!\n");
			}

			if (attacker == SPREE_DUDE)
			{
				message = HiPrint(va("%s SPREE WAR: %d frag spree!", attacker->client->pers.netname, attacker->myskills.streak));
				gi.bprintf(PRINT_HIGH, "%s\n", message);
				message = LoPrint(message);
			}
		}
		else if (attacker->myskills.streak >= 10 && GetRandom(1,2) == 1)
		{
			message = HiPrint(va("%s rampage: %d frag spree!", attacker->client->pers.netname, attacker->myskills.streak));
			gi.bprintf(PRINT_HIGH, "%s\n", message);
			message = LoPrint(message);
		}
		else if (attacker->myskills.streak >= 10)
		{
			message = HiPrint(va("%s is god-like: %d frag spree!", attacker->client->pers.netname, attacker->myskills.streak));
			gi.bprintf(PRINT_HIGH, "%s\n", message);
			message = LoPrint(message);
		}
		else if (attacker->myskills.streak >= SPREE_START)
		{
			message = HiPrint(va("%s is on a %d frag spree!", attacker->client->pers.netname, attacker->myskills.streak));
			gi.bprintf(PRINT_HIGH, "%s\n", message);
			message = LoPrint(message);
		}
	}
}

/*
===========
Add_exp_by_damage

Adds experience to attacker based on damage done to targ

CURRENTLY DISABLED
============
*/
/*
void Add_exp_by_damage (edict_t *attacker, edict_t *targ, int damage)
{
	float	exp_points=0;
	float	lose_points=0;
	float	level_diff=0;
	float	bonus=1;
	edict_t *player;

	attacker = GetTheClient(attacker);
	player = GetTheClient(targ);

	if (attacker == NULL || player == NULL || damage <= 0)
		return;

	// ignore invalid data
	if (targ == attacker || player->deadflag || !player->takedamage)
		return;

	// calc level difference modifier
	level_diff = (float) (player->myskills.level + 1) / (attacker->myskills.level + 1);
	
	if (!ctf->value)
	{
		// calc spree bonuses
		if (attacker->myskills.streak >= SPREE_START)
			bonus += 0.02 * attacker->myskills.streak;
		else if (attacker->myskills.streak >= SPREE_WARS_START && SPREE_WARS)
			bonus += 0.03 * attacker->myskills.streak;

		// calc spree break bonuses
		if (player->myskills.streak >= SPREE_START && !SPREE_WAR)
			bonus += 0.01 * player->myskills.streak;
		else if (player->myskills.streak >= SPREE_WARS_START && SPREE_WARS)
			bonus += 0.015 * player->myskills.streak;
		
		// cap maximum points to 200%
		if (bonus > 2.0)
			bonus = 2.0;
	}

	if (targ->client)
		exp_points = damage * (0.25 * (level_diff * bonus));
	else
		exp_points = damage * (0.025 * (level_diff * bonus));

	if (targ->client)
		lose_points = 0.25 * exp_points;

//	gi.dprintf("attacker exp + %f, attacker cache =%f\n", exp_points, attacker->myskills.exp_cache);
//	gi.dprintf("targ exp - %f, targ cache =%f\n", lose_points, targ->myskills.exp_cache);

	// add non-integer values to cache
	if (exp_points < 1 && attacker->myskills.exp_cache < 1)
	{
		attacker->myskills.exp_cache += exp_points;
		return;
	}
	if (lose_points < 1 && player->myskills.exp_cache < 1)
	{
		player->myskills.exp_cache += lose_points;
		return;
	}

	// empty cache
	if (attacker->myskills.exp_cache >= 1)
	{
		exp_points += attacker->myskills.exp_cache;
		attacker->myskills.exp_cache = 0;
	}
	if (player->myskills.exp_cache >= 1)
	{
		lose_points += player->myskills.exp_cache;
		player->myskills.exp_cache = 0;
	}

	// give your team some experience
	if (ctf->value)
		Add_ctfteam_exp(attacker, (int)(0.5*exp_points));

	// modify attacker's score
	attacker->client->resp.score += (int) exp_points;
	attacker->myskills.experience += (int) exp_points;

	// reduce target's score
	if (player->client->resp.score - lose_points > 0)
	{
		player->client->resp.score -= (int) lose_points;
		player->myskills.experience -= (int) lose_points;
	}
	
	check_for_levelup(attacker);
}
*/
