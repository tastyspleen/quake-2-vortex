#include "g_local.h"

void Give_respawnitems(edict_t *ent)
{
//	gi.dprintf("Give_respawnitems()\n");

	ent->client->pers.inventory[ITEM_INDEX(Fdi_POWERCUBE)] = ent->myskills.inventory[ITEM_INDEX(Fdi_POWERCUBE)];
	ent->client->pers.inventory[ITEM_INDEX(Fdi_TBALL)] = ent->myskills.inventory[ITEM_INDEX(Fdi_TBALL)];
	ent->client->pers.inventory[ITEM_INDEX(Fdi_TBALL)] = ent->myskills.inventory[ITEM_INDEX(Fdi_TBALL)] += TBALLS_RESPAWN;
	
	if ((ent->myskills.class_num == CLASS_POLTERGEIST) && (ent->client->pers.inventory[power_cube_index] < 50))
			ent->client->pers.inventory[power_cube_index] += 50;
	else	ent->client->pers.inventory[ITEM_INDEX(Fdi_POWERCUBE)] = ent->myskills.inventory[ITEM_INDEX(Fdi_POWERCUBE)] += POWERCUBES_RESPAWN;

	if (ent->client->pers.inventory[ITEM_INDEX(Fdi_TBALL)] > 20)
		ent->client->pers.inventory[ITEM_INDEX(Fdi_TBALL)] = 20;
	if (ent->client->pers.inventory[ITEM_INDEX(Fdi_POWERCUBE)] > ent->client->pers.max_powercubes)
		ent->client->pers.inventory[ITEM_INDEX(Fdi_POWERCUBE)] = ent->client->pers.max_powercubes;
	ent->client->pers.inventory[ITEM_INDEX(Fdi_BLASTER)] = 1;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("Sword"))] = 1;

	//if (!ent->myskills.abilities[START_ARMOR].disable)
	//	ent->client->pers.inventory[ITEM_INDEX(FindItem("Body Armor"))] = ((MAX_ARMOR(ent))*0.2)*ent->myskills.abilities[START_ARMOR].current_level;
	if ((ent->myskills.level >= 10) || (ent->myskills.class_num == CLASS_KNIGHT)) // give starting armor to level 10+ and knight
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Body Armor"))] += 50;
	if (ent->myskills.boss > 0)
		 ent->client->pers.inventory[ITEM_INDEX(FindItem("Body Armor"))] = MAX_ARMOR(ent);
}

void giveAdditionalRespawnWeapon(edict_t *ent, int nextWeapon)
{
	int picks[MAX_WEAPONS] = {0};
	int nextEmptyslot = 0;

	picks[nextEmptyslot++] = ent->myskills.respawn_weapon;

	//Don't crash
	if(nextWeapon > MAX_WEAPONS)	return;
	/*
		This next bit of code is hard to read. I'll explain it here.
        
		Search through the player's upgradeable weapons for the
		one with the most upgrades. Any weapons already in the list
		are skipped. The resultant weapon is added to the list.
		This process is repeated until the list is large enough.

		ie: Sort the weapons in upgraded order until we find weapon
		number X on the list. Then give them respawn weapon X.
	*/
	
	do
	{
		int i, j;
		int max = 0;
		int maxvalue = V_WeaponUpgradeVal(ent, 0);

		//Skip any weapons already added to the list.
		for (i = 1; i < MAX_WEAPONS; ++i)
		{
			qboolean skip;
			int value;
			
			skip = false;
			for(j = 0; j < nextEmptyslot; ++j)
			{
				if(i == picks[j])
				{
					skip = true;
					break;
				}
			}
			if(skip)	continue;

			//Get the upgrade value and compare it to the current leader.
			value = V_WeaponUpgradeVal(ent, i);
			if(value > maxvalue)
			{
				maxvalue = value;
				max = i;
			}
		}

		//A maximum has been found. Add it to the list.
		picks[nextEmptyslot++] = max;
	}
	while(nextEmptyslot < nextWeapon+1);

	//Give the last item on the list as a respawn weapon.
	Give_respawnweapon(ent, picks[nextEmptyslot-1] + 1);
}

void Give_respawnweapon(edict_t *ent, int weaponID)
{
	//Give_respawnitems(ent);

	//3.02 begin new respawn weapon code
	//Give them the weapon
	switch (weaponID)
	{
	case 2:		ent->client->pers.inventory[ITEM_INDEX(Fdi_SHOTGUN)] = 1;				break;
	case 3:		ent->client->pers.inventory[ITEM_INDEX(Fdi_SUPERSHOTGUN)] = 1;			break;
	case 4:		ent->client->pers.inventory[ITEM_INDEX(Fdi_MACHINEGUN)] = 1;			break;
	case 5:		ent->client->pers.inventory[ITEM_INDEX(Fdi_CHAINGUN)] = 1;				break;
	case 6:		ent->client->pers.inventory[ITEM_INDEX(Fdi_GRENADELAUNCHER)] = 1;		break;
	case 7:		ent->client->pers.inventory[ITEM_INDEX(Fdi_ROCKETLAUNCHER)] = 1;		break;
	case 8:		ent->client->pers.inventory[ITEM_INDEX(Fdi_HYPERBLASTER)] = 1;			break;
	case 9:		ent->client->pers.inventory[ITEM_INDEX(Fdi_RAILGUN)] = 1;				break;	
	case 10:	ent->client->pers.inventory[ITEM_INDEX(Fdi_BFG)] = 1;					break;
	case 11:	ent->client->pers.inventory[ITEM_INDEX(Fdi_GRENADES)] = 1;				break;
	case 12:	ent->client->pers.inventory[ITEM_INDEX(FindItem("20mm Cannon"))] = 1;	break;
	case 13:	ent->client->pers.inventory[ITEM_INDEX(Fdi_BLASTER)] = 1;				break;
	default:	ent->client->pers.inventory[ITEM_INDEX(Fdi_BLASTER)] = 1;				break;
	}

	//Give them the ammo
	if (ent->myskills.level < 5)
		V_GiveAmmoClip(ent, 2, V_GetRespawnAmmoType(ent));
	else V_GiveAmmoClip(ent, 1, V_GetRespawnAmmoType(ent));
	//3.02 end new respawn weapon code

	Pick_respawnweapon(ent);

	//3.0 reset auto-tball flag
	ent->v_flags = SFLG_NONE;
}

void Pick_respawnweapon(edict_t *ent)
{
	gitem_t		*item;

		switch (ent->myskills.respawn_weapon)
		{
		case 1:
			item = FindItem("Sword");
			break;
		case 2:
			item = Fdi_SHOTGUN;
			break;
		case 3:
			item = Fdi_SUPERSHOTGUN;
			break;
		case 4:
			item = Fdi_MACHINEGUN;
			break;
		case 5:
			item = Fdi_CHAINGUN;
			break;
		case 6:
			item = Fdi_GRENADELAUNCHER;
			break;
		case 7:
			item = Fdi_ROCKETLAUNCHER;
			break;
		case 8:
			item = Fdi_HYPERBLASTER;
			break;
		case 9:
			item = Fdi_RAILGUN;
			break;
		case 10:
			item = Fdi_BFG;
			break;
		case 11:
			item = Fdi_GRENADES;
			break;
		case 12:
			item = FindItem ("20mm Cannon");
			break;
		case 13:
			item = Fdi_BLASTER;
			break;
		default:
			item = Fdi_BLASTER;
			break;
		}

	ent->client->pers.selected_item = ITEM_INDEX(item);
	ent->client->pers.weapon = item;
	ent->client->pers.lastweapon = item;
	ent->client->newweapon = item;
	ChangeWeapon(ent);
}


void KickPlayerBack(edict_t *ent)
{
	edict_t *other=NULL;

	while ((other = findradius(other, ent->s.origin, 175)) != NULL)
	{
		if (!other->takedamage)
			continue;
		if (!other->client)
			continue;
		if (!visible(ent, other))
			continue;
		if (other == ent)
			continue;
		ent->client->invincible_framenum = level.time + 2.0; // Add 2.0 seconds of invincibility
	}
}

int total_players(void)
{
	int		i, total=0;
	edict_t *cl_ent;
	
	for (i=0 ; i<game.maxclients ; i++)
	 {
          cl_ent = g_edicts + 1 + i;
		  if (!cl_ent->inuse)
			  continue;
		  if (G_IsSpectator(cl_ent))
			  continue;
          //if (!G_EntExists(cl_ent)) 
           //    continue; 
          total++; 
     }
	return total;
}

void GetScorePosition (void) 
{ 
     int i, j, k; 
     int sorted[MAX_CLIENTS]; 
	 int sortedscores[MAX_CLIENTS] = { 0 };
     int score, total, last_score, last_pos=1; 
     //gclient_t *cl; 
     edict_t *cl_ent; 

     // sort the clients by score 
     total = 0; 
     for (i=0 ; i<game.maxclients ; i++)
	 {
          cl_ent = g_edicts + 1 + i; 
          if (!cl_ent->inuse) 
               continue; 
          score = game.clients[i].resp.score; 
          for (j=0 ; j<total ; j++)
		  {
               if (score > sortedscores[j]) 
               break; 
          } 
          for (k=total ; k>j ; k--) 
          { 
               sorted[k] = sorted[k-1]; 
               sortedscores[k] = sortedscores[k-1]; 
          } 
          sorted[j] = i; 
          sortedscores[j] = score; 
          total++; 
     }
	 last_score = sortedscores[0];

     for (i=0 ; i<total ; i++)
	 {
          //cl = &game.clients[sorted[i]]; 
          cl_ent = g_edicts + 1 + sorted[i];

		  if (last_score != sortedscores[i])
			last_pos++;

		  //cl_ent->client->ps.stats[STAT_RANK] = last_pos;
		  //cl_ent->client->ps.stats[STAT_TOTALPLAYERS] = total;

     } 
} 

int GetRandom(int min, int max)
{
	return (rand() % (max + 1 - min) + min);
}

qboolean findspawnpoint (edict_t *ent)
{
  vec3_t loc = {0,0,0};
  vec3_t floor;
  int i;
  int j = 0;
  int k = 0;
  trace_t tr;
  do {
    j++;
    for (i = 0; i < 3; i++)
      loc[i] = rand() % (8192 + 1) - 4096;
    if (gi.pointcontents(loc) == 0)
    {
      VectorCopy(loc, floor);
      floor[2] = -4096;
      tr = gi.trace (loc, vec3_origin, vec3_origin, floor, NULL, MASK_SOLID);
      k++;
      if (tr.contents & MASK_WATER)
        continue; 
	  if (tr.contents & CONTENTS_SOLID)
		continue;//Don't start the entity in a solid. It could be troublesome.
      VectorCopy (tr.endpos, loc);
      loc[2] += ent->maxs[2] - ent->mins[2]; // make sure the entity can fit!
    }
  } while (gi.pointcontents(loc) > 0 && j < 1000 && k < 500);
  if (j >= 1000 || k >= 500)
    return false;
  VectorCopy(loc,ent->s.origin);
  VectorCopy(loc,ent->s.old_origin);
 // gi.dprintf("found valid spot on try %d\n", j);
  return true;
}

qboolean FindValidSpawnPoint (edict_t *ent, qboolean air)
{
	int		i, j=0, mask;
	vec3_t	start, end, forward, right;
	trace_t	tr;

	//gi.dprintf("FindValidSpawnPoint()\n");

	mask = (MASK_MONSTERSOLID|MASK_PLAYERSOLID|MASK_SOLID);

	for (j=0;j<50000;j++)
	{
		// get a random position within a map
		for (i=0;i<3;i++)
			start[i] = rand() % (8192 + 1) - 4096;
		// is the point good?
		if (gi.pointcontents(start) != 0)
			continue;
		if (!air)
		{
			// then trace to the floor
			VectorCopy(start, end);
			end[2] -= 8192;
			tr = gi.trace(start, NULL, NULL, end, NULL, mask);
			// dont spawn on someone's head!
			if (tr.ent && tr.ent->inuse && (tr.ent->health > 0))
				continue;
			// add the ent's height
			VectorCopy(tr.endpos, start);
			start[2] += fabsf(ent->mins[2]) + 1;
			// is the point good?
			if (gi.pointcontents(start) != 0)
				continue;
		}

		// dont spawn outside of map!
		// check beneath us
		VectorCopy(start, end);
		end[2] -= 8192;
		tr = gi.trace(start, NULL, NULL, end, NULL, MASK_SOLID);
		if (tr.surface->flags & (SURF_SKY|SURF_LIGHT)) //3.49 light flag indicates no-monster area
			continue;
		// check in front of us
		AngleVectors(ent->s.angles, forward, right, NULL);
		VectorMA(start, 8192, forward, end);
		tr = gi.trace(start, NULL, NULL, end, NULL, MASK_SOLID);
		if (tr.surface->flags & SURF_SKY)
			continue;
		// check behind
		VectorMA(start, -8192, forward, end);
		tr = gi.trace(start, NULL, NULL, end, NULL, MASK_SOLID);
		if (tr.surface->flags & SURF_SKY)
			continue;
		// check right
		VectorMA(start, 8192, right, end);
		tr = gi.trace(start, NULL, NULL, end, NULL, MASK_SOLID);
		if (tr.surface->flags & SURF_SKY)
			continue;
		// check left
		VectorMA(start, -8192, right, end);
		tr = gi.trace(start, NULL, NULL, end, NULL, MASK_SOLID);
		if (tr.surface->flags & SURF_SKY)
			continue;

		// then check our final position
		tr = gi.trace(start, ent->mins, ent->maxs, start, NULL, mask);
		if (tr.startsolid || tr.allsolid || tr.fraction != 1.0)
			continue;
		if (tr.contents & mask)
			continue;
		break;
	}

	// avoid spending too much time looking for a spot
	if (j >= 50000)
		return false;

	VectorCopy(start, ent->s.origin);
	VectorCopy(start, ent->s.old_origin);
	gi.linkentity(ent);
	return true;
}

qboolean TeleportNearArea (edict_t *ent, vec3_t point, int area_size, qboolean air)
{
	int		i, j;
	vec3_t	start, end;
	trace_t	tr;

	for (i=0; i<50000; i++) {
		for (j=0; j<3; j++) {
			VectorCopy(point, start);
			start[j] += rand() % (area_size + 1) - 0.5*area_size;
			if (gi.pointcontents(start) != 0)
				continue;
			if (!air)
			{
				// then trace to the floor
				VectorCopy(start, end);
				end[2] -= 8192;
				tr = gi.trace(start, NULL, NULL, end, NULL, MASK_SHOT);
				// dont spawn on someone's head!
				if (tr.ent && tr.ent->inuse && (tr.ent->health > 0))
					continue;
				// add the ent's height
				VectorCopy(tr.endpos, start);
				start[2] += fabsf(ent->mins[2]) + 1;
				// is the point good?
				if (gi.pointcontents(start) != 0)
					continue;
			}
			// can we "see" the point?
			tr = gi.trace(start, NULL, NULL, point, NULL, MASK_SOLID);
			if (tr.fraction < 1)
				continue;
			tr = gi.trace(start, ent->mins, ent->maxs, start, NULL, MASK_SHOT);
			if (tr.startsolid || tr.allsolid || tr.fraction != 1)
				continue;
			if (tr.contents & MASK_SHOT)
				continue;
			ent->s.event = EV_PLAYER_TELEPORT;
			VectorClear(ent->velocity);
			VectorCopy(start, ent->s.origin);
			VectorCopy(start, ent->s.old_origin);
			gi.linkentity(ent);
			//gi.dprintf("teleported near area\n");
			return true;
		}
	}

//	gi.dprintf("failed to teleport near area\n");
	return false;
}

#define VITALITY_MULT	0.1

int MAX_ARMOR(edict_t *ent)
{
	int vitlvl = 0;
	int talentlevel;
	int value;

	if(!ent->myskills.abilities[VITALITY].disable)
		vitlvl = ent->myskills.abilities[VITALITY].current_level;

switch (ent->myskills.class_num)
	{
	case CLASS_SOLDIER:		value = INITIAL_ARMOR_SOLDIER+LEVELUP_ARMOR_SOLDIER*ent->myskills.level;			break;
	case CLASS_VAMPIRE:		value = INITIAL_ARMOR_VAMPIRE+LEVELUP_ARMOR_VAMPIRE*ent->myskills.level;			break;
	case CLASS_ENGINEER:	value = INITIAL_ARMOR_ENGINEER+LEVELUP_ARMOR_ENGINEER*ent->myskills.level;			break;
	case CLASS_MAGE:		value = INITIAL_ARMOR_MAGE+LEVELUP_ARMOR_MAGE*ent->myskills.level;					break;
	case CLASS_NECROMANCER:	value = INITIAL_ARMOR_NECROMANCER+LEVELUP_ARMOR_NECROMANCER*ent->myskills.level;	break;
	case CLASS_POLTERGEIST:	value = INITIAL_ARMOR_POLTERGEIST+LEVELUP_ARMOR_POLTERGEIST*ent->myskills.level;	break;
	case CLASS_KNIGHT:		value = INITIAL_ARMOR_KNIGHT+LEVELUP_ARMOR_KNIGHT*ent->myskills.level;				break;
	case CLASS_CLERIC:		value = INITIAL_ARMOR_CLERIC+LEVELUP_ARMOR_CLERIC*ent->myskills.level;				break;
	case CLASS_WEAPONMASTER:value = INITIAL_ARMOR_WEAPONMASTER+LEVELUP_ARMOR_WEAPONMASTER*ent->myskills.level;	break;
	case CLASS_SHAMAN:		value = INITIAL_ARMOR_SHAMAN+LEVELUP_ARMOR_SHAMAN*ent->myskills.level;				break;
	case CLASS_ALIEN:		value = INITIAL_ARMOR_ALIEN+LEVELUP_ARMOR_ALIEN*ent->myskills.level;				break;
	default:				value = 100 + 5*ent->myskills.level;												break;
	}

	//Talent: Tactics	(weaponmaster)
	talentlevel = getTalentLevel(ent, TALENT_TACTICS);
	if(talentlevel > 0)		value += ent->myskills.level * talentlevel;	//+1 armor per upgrade

	value *= 1.0 + VITALITY_MULT * vitlvl;

	return value;
}

#define INITIAL_HEALTH_FC	200
#define ADDON_HEALTH_FC		30

int MAX_HEALTH(edict_t *ent)
{
	int vitlvl = 0;
	int talentlevel;
	int value;

	// if using special flag carrier rules in CTF then the fc
	// gets health based on level and nothing else
	if (ctf->value && ctf_enable_balanced_fc->value && HasFlag(ent))
		return INITIAL_HEALTH_FC+ADDON_HEALTH_FC*ent->myskills.level;

	if(!ent->myskills.abilities[VITALITY].disable)
		vitlvl = ent->myskills.abilities[VITALITY].current_level;

	switch (ent->myskills.class_num)
	{
	case CLASS_SOLDIER:		value = INITIAL_HEALTH_SOLDIER+LEVELUP_HEALTH_SOLDIER*ent->myskills.level;				break;
	case CLASS_VAMPIRE:		value = INITIAL_HEALTH_VAMPIRE+LEVELUP_HEALTH_VAMPIRE*ent->myskills.level;				break;
	case CLASS_ENGINEER:	value = INITIAL_HEALTH_ENGINEER+LEVELUP_HEALTH_ENGINEER*ent->myskills.level;			break;
	case CLASS_MAGE:		value = INITIAL_HEALTH_MAGE+LEVELUP_HEALTH_MAGE*ent->myskills.level;					break;
	case CLASS_NECROMANCER:	value = INITIAL_HEALTH_NECROMANCER+LEVELUP_HEALTH_NECROMANCER*ent->myskills.level;		break;
	case CLASS_POLTERGEIST:	value = INITIAL_HEALTH_POLTERGEIST+LEVELUP_HEALTH_POLTERGEIST*ent->myskills.level;		break;
	case CLASS_KNIGHT:		value = INITIAL_HEALTH_KNIGHT+LEVELUP_HEALTH_KNIGHT*ent->myskills.level;				break;
	case CLASS_CLERIC:		value = INITIAL_HEALTH_CLERIC+LEVELUP_HEALTH_CLERIC*ent->myskills.level;				break;
	case CLASS_WEAPONMASTER:value = INITIAL_HEALTH_WEAPONMASTER+LEVELUP_HEALTH_WEAPONMASTER*ent->myskills.level;	break;
	case CLASS_SHAMAN:		value = INITIAL_HEALTH_SHAMAN+LEVELUP_HEALTH_SHAMAN*ent->myskills.level;				break;
	case CLASS_ALIEN:		value = INITIAL_HEALTH_ALIEN+LEVELUP_HEALTH_ALIEN*ent->myskills.level;					break;
	default:				value = 100+5*ent->myskills.level;
	}

	//Note: These talents are all different so they can be tweaked individually.

	//Talent: Super Vitality	(vamps)
//	talentlevel = getTalentLevel(ent, TALENT_SUPERVIT);
//	if(talentlevel > 0)		value += ent->myskills.level * talentlevel;	//+1 health per upgrade
	
	//Talent: Durability		(knight)
	talentlevel = getTalentLevel(ent, TALENT_DURABILITY);
	if(talentlevel > 0)		value += ent->myskills.level * talentlevel;	//+1 health per upgrade

	//Talent: Tactics			(weaponmaster)
	talentlevel = getTalentLevel(ent, TALENT_TACTICS);
	if(talentlevel > 0)		value += ent->myskills.level * talentlevel;	//+1 health per upgrade

    //Talent: Improved Vitality	(necromancer)
//	talentlevel = getTalentLevel(ent, TALENT_IMP_VITALITY);
//	if(talentlevel > 0)		value += ent->myskills.level * talentlevel;	//+1 health per upgrade

	value *= 1.0 + VITALITY_MULT * vitlvl;

	return value;
}

int MAX_BULLETS(edict_t *ent)
{
	if(ent->myskills.abilities[MAX_AMMO].disable)
		return 0;
	return (100*ent->myskills.abilities[MAX_AMMO].current_level);
}

int MAX_SHELLS(edict_t *ent)
{
	if(ent->myskills.abilities[MAX_AMMO].disable)
		return 0;
	return (50*ent->myskills.abilities[MAX_AMMO].current_level);
}

int MAX_ROCKETS(edict_t *ent)
{
	if(ent->myskills.abilities[MAX_AMMO].disable)
		return 0;
	return (25*ent->myskills.abilities[MAX_AMMO].current_level);
}

int MAX_GRENADES(edict_t *ent)
{
	if(ent->myskills.abilities[MAX_AMMO].disable)
		return 0;
	return (25*ent->myskills.abilities[MAX_AMMO].current_level);
}

int MAX_CELLS(edict_t *ent)
{
	if(ent->myskills.abilities[MAX_AMMO].disable)
		return 0;
	return (100*ent->myskills.abilities[MAX_AMMO].current_level);
}

int MAX_SLUGS(edict_t *ent)
{
	if(ent->myskills.abilities[MAX_AMMO].disable)
		return 0;
	return (25*ent->myskills.abilities[MAX_AMMO].current_level);
}

int MAX_POWERCUBES(edict_t *ent)
{
	int value = 0, clvl;

	if(ent->myskills.abilities[MAX_AMMO].disable)
		return 0;

	clvl = ent->myskills.level;

	switch (ent->myskills.class_num)
	{
	case CLASS_SOLDIER: value=INITIAL_POWERCUBES_SOLDIER+ADDON_POWERCUBES_SOLDIER*clvl; break;
	case CLASS_VAMPIRE: value=INITIAL_POWERCUBES_VAMPIRE+ADDON_POWERCUBES_VAMPIRE*clvl; break;
	case CLASS_NECROMANCER: value=INITIAL_POWERCUBES_NECROMANCER+ADDON_POWERCUBES_NECROMANCER*clvl; break;
	case CLASS_KNIGHT: value=INITIAL_POWERCUBES_KNIGHT+ADDON_POWERCUBES_KNIGHT*clvl; break;
	case CLASS_MAGE: value=INITIAL_POWERCUBES_MAGE+ADDON_POWERCUBES_MAGE*clvl; break;
	case CLASS_POLTERGEIST: value=INITIAL_POWERCUBES_POLTERGEIST+ADDON_POWERCUBES_POLTERGEIST*clvl; break;
	case CLASS_CLERIC: value=INITIAL_POWERCUBES_CLERIC+ADDON_POWERCUBES_CLERIC*clvl; break;
	case CLASS_ALIEN: value=INITIAL_POWERCUBES_ALIEN+ADDON_POWERCUBES_ALIEN*clvl; break;
	case CLASS_SHAMAN: value=INITIAL_POWERCUBES_SHAMAN+ADDON_POWERCUBES_SHAMAN*clvl; break;
	case CLASS_ENGINEER: value=INITIAL_POWERCUBES_ENGINEER+ADDON_POWERCUBES_ENGINEER*clvl; break;
	}

	return value * (1 + (0.1 * ent->myskills.abilities[MAX_AMMO].current_level));
}

void modify_max(edict_t *ent)
{
	ent->max_health = MAX_HEALTH(ent);
	ent->client->pers.max_health = ent->max_health;

	ent->client->pers.max_bullets	= 200 + MAX_BULLETS(ent);
	ent->client->pers.max_shells	= 100 + MAX_SHELLS(ent);
	ent->client->pers.max_rockets	= 50 + MAX_ROCKETS(ent);
	ent->client->pers.max_grenades	= 50 + MAX_GRENADES(ent);
	ent->client->pers.max_cells		= 200 + MAX_CELLS(ent);
	ent->client->pers.max_slugs		= 50 + MAX_SLUGS(ent);

	ent->client->pers.max_powercubes = MAX_POWERCUBES(ent);

}
void modify_health(edict_t *ent)
{
	ent->health = MAX_HEALTH(ent);
	ent->client->pers.health = ent->health;
}

void Check_full(edict_t *ent)
{
	gitem_t	*item;
	int		index;

	item = FindItem("Bullets");
	if (item)
	{
		index = ITEM_INDEX(item);
		if (ent->client->pers.inventory[index] > ent->client->pers.max_bullets)
			ent->client->pers.inventory[index] = ent->client->pers.max_bullets;
	}

	item = FindItem("Shells");
	if (item)
	{
		index = ITEM_INDEX(item);
		if (ent->client->pers.inventory[index] > ent->client->pers.max_shells)
			ent->client->pers.inventory[index] = ent->client->pers.max_shells;
	}

	item = FindItem("Cells");
	if (item)
	{
		index = ITEM_INDEX(item);
		if (ent->client->pers.inventory[index] > ent->client->pers.max_cells)
			ent->client->pers.inventory[index] = ent->client->pers.max_cells;
	}

	item = FindItem("Grenades");
	if (item)
	{
		index = ITEM_INDEX(item);
		if (ent->client->pers.inventory[index] > ent->client->pers.max_grenades)
			ent->client->pers.inventory[index] = ent->client->pers.max_grenades;
	}

	item = FindItem("Rockets");
	if (item)
	{
		index = ITEM_INDEX(item);
		if (ent->client->pers.inventory[index] > ent->client->pers.max_rockets)
			ent->client->pers.inventory[index] = ent->client->pers.max_rockets;
	}

	item = FindItem("Slugs");
	if (item)
	{
		index = ITEM_INDEX(item);
		if (ent->client->pers.inventory[index] > ent->client->pers.max_slugs)
			ent->client->pers.inventory[index] = ent->client->pers.max_slugs;
	}
}

void Ammo_Regen(edict_t *ent)
{
	if(ent->myskills.abilities[AMMO_REGEN].disable)
		return;

	if (ent->client->buttons & BUTTON_ATTACK)
		return; // don't regenerate ammo while attacking

	if (level.time > ent->client->ammo_regentime)
	{
		V_GiveAmmoClip(ent, ent->myskills.abilities[AMMO_REGEN].current_level * 0.2, AMMO_SHELLS);		//20% of a pack per point
		V_GiveAmmoClip(ent, ent->myskills.abilities[AMMO_REGEN].current_level * 0.1, AMMO_BULLETS);		//10% of a pack per point
		V_GiveAmmoClip(ent, ent->myskills.abilities[AMMO_REGEN].current_level * 0.1, AMMO_CELLS);		//10% of a pack per point
		V_GiveAmmoClip(ent, ent->myskills.abilities[AMMO_REGEN].current_level * 0.2, AMMO_GRENADES);	//20% of a pack per point
		V_GiveAmmoClip(ent, ent->myskills.abilities[AMMO_REGEN].current_level * 0.2, AMMO_ROCKETS);		//20% of a pack per point
		V_GiveAmmoClip(ent, ent->myskills.abilities[AMMO_REGEN].current_level * 0.2, AMMO_SLUGS);		//20% of a pack per point

		ent->client->ammo_regentime = level.time + AMMO_REGEN_DELAY;
	}
}

void Special_Regen(edict_t *ent)
{
	gitem_t	*item;
	int index, temp;

	if (ent->myskills.abilities[POWER_REGEN].disable)
		return;

	item = Fdi_POWERCUBE;
	if (item)
	{
		index = ITEM_INDEX(item);
		if (ent->client->pers.inventory[index] < ent->client->pers.max_powercubes)
		{
			temp = 5*ent->myskills.abilities[POWER_REGEN].current_level;

			ent->client->pers.inventory[index] += temp;

			if (ent->client->pers.inventory[index] > ent->client->pers.max_powercubes)
				ent->client->pers.inventory[index] = ent->client->pers.max_powercubes;

			if (ent->client->pers.inventory[index] < 0)
				ent->client->pers.inventory[index] = 0;
		}
	}
	
}

float entdist(edict_t *ent1, edict_t *ent2)
{
	vec3_t	vec;

	VectorSubtract(ent1->s.origin, ent2->s.origin, vec);
	return VectorLength(vec);
}

// returns true if the player should be affected by newbie protection
qboolean IsNewbieBasher (edict_t *player) {
	return (newbie_protection->value && player->client && (total_players()>0.33*maxclients->value)
		&& (player->myskills.level>(newbie_protection->value*average_player_level)) 
		&& (player->myskills.level >= 8) && !ctf->value && !pvm->value && !ffa->value);
}

qboolean TeleportNearTarget (edict_t *self, edict_t *target, float dist)
{
	int		i;
	float	yaw;
	vec3_t	forward, start, end;
	trace_t	tr;

	// check 8 angles at 45 degree intervals
	for(i=0;i<8;i++)
	{
		yaw = anglemod(i*45);
		forward[0] = cos(DEG2RAD(yaw));
		forward[1] = sin(DEG2RAD(yaw));
		forward[2] = 0;
		// trace from target
		VectorMA(target->s.origin, (target->maxs[0]+self->maxs[0]+dist), forward, end);
		tr = gi.trace(target->s.origin, NULL, NULL, end, target, MASK_MONSTERSOLID);
		// trace to floor
		VectorCopy(tr.endpos, start);
		VectorCopy(tr.endpos, end);
		end[2] -= fabsf(self->mins[2]) + 32;
		tr = gi.trace(start, NULL, NULL, end, NULL, MASK_MONSTERSOLID);
		// we dont want to teleport off a ledge
		if (tr.fraction == 1.0 && !(self->flags & FL_FLY))
			continue;
		// check for valid position
		VectorCopy(tr.endpos, start);
		start[2] += fabsf(self->mins[2]) + 1;
		tr = gi.trace(start, self->mins, self->maxs, start, NULL, MASK_MONSTERSOLID);
		if (!(tr.contents & MASK_MONSTERSOLID))
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BOSSTPORT);
			gi.WritePosition (self->s.origin);
			gi.multicast (self->s.origin, MULTICAST_PVS);

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BOSSTPORT);
			gi.WritePosition (start);
			gi.multicast (start, MULTICAST_PVS);

			VectorCopy(start, self->s.origin);
			gi.linkentity(self);
			return true;
		}
	}
	return false;
}

qboolean TeleportNearPoint (edict_t *self, vec3_t point)
{
	int		i;
	float	yaw, dist=1;
	vec3_t	forward, start, end;
	trace_t	tr;

	// check 16 angles at 22.5 degree intervals
	while (dist < 256)
	{
		for(i=0;i<8;i++)
		{
			yaw = anglemod(i*22.5);
			forward[0] = cos(DEG2RAD(yaw));
			forward[1] = sin(DEG2RAD(yaw));
			forward[2] = 0;
			// trace from point
			VectorMA(point, (self->maxs[0] + fabsf(self->mins[0])+dist), forward, end);
			tr = gi.trace(point, NULL, NULL, end, NULL, MASK_SOLID);
			// trace to floor
			VectorCopy(tr.endpos, start);
			VectorCopy(tr.endpos, end);
			end[2] -= fabsf(self->mins[2]) + 128;
			tr = gi.trace(start, NULL, NULL, end, NULL, MASK_SOLID);
			// we dont want to teleport off a ledge
			if (tr.fraction == 1.0 && !(self->flags & FL_FLY))
				continue;
			// check for valid position
			VectorCopy(tr.endpos, start);
			start[2] += fabsf(self->mins[2]) + 1;
			tr = gi.trace(start, self->mins, self->maxs, start, NULL, (MASK_PLAYERSOLID|MASK_MONSTERSOLID));
			if (!(tr.contents & (MASK_PLAYERSOLID|MASK_MONSTERSOLID)))
			{
				self->s.event = EV_PLAYER_TELEPORT;
				VectorClear(self->velocity);
				VectorCopy(start, self->s.origin);
				gi.linkentity(self);
				return true;
			}
			dist += self->maxs[0] + fabsf(self->mins[0])+1;
		}
	}
	return false;
}

void WriteToLogFile(char* char_name, char* s)
{
	char     buf[512];
	char     path[256];
	FILE* fptr;

	if (!char_name || !s)
		return;

	if (strlen(char_name) < 1)
		return;

	//Create the log message
	Com_sprintf(buf, sizeof buf, "%s %s [%s]: %s", CURRENT_DATE, CURRENT_TIME, "Offline", s);

	//determine path
	Com_sprintf(path, sizeof path, "%s/%s.log", save_path->string, V_FormatFileName(char_name));

	if ((fptr = fopen(path, "a")) != NULL) // append text to log
	{
		//3.0 make sure there is a line feed
		if (buf[strlen(buf) - 1] != '\n')
			strcat(buf, "\n");

		fprintf(fptr, "%s", buf);
		fclose(fptr);
		return;
	}
	gi.dprintf("ERROR: Failed to write to player log.\n");
}

void WriteToLogfile(edict_t* ent, char* s)
{
	char* ip, buf[512];
	char     path[256];
	FILE* fptr;

	if (!s || !ent || !ent->client)
		return;
	
	if (strlen(ent->client->pers.netname) < 1)
		return;

	//Create the log message  
	ip = Info_ValueForKey(ent->client->pers.userinfo, "ip");
	Com_sprintf(buf, sizeof buf, "%s %s [%s]: %s", CURRENT_DATE, CURRENT_TIME, ip, s);

	//determine path  
	Com_sprintf(path, sizeof path, "%s/%s.log", save_path->string, V_FormatFileName(ent->client->pers.netname));

	if ((fptr = fopen(path, "a")) != NULL) // append text to log
	{
		//3.0 make sure there is a line feed
		if (buf[strlen(buf) - 1] != '\n')
			strcat(buf, "\n");

		fprintf(fptr, "%s", buf);
		fclose(fptr);
		return;
	}
	gi.dprintf("ERROR: Failed to write to player log.\n");
}

void WriteServerMsg(char* s, char* error_string, qboolean print_msg, qboolean save_to_logfile)
{
	cvar_t* port;
	char	buf[512];
	char	path[256];
	FILE* fptr;

	if (!s || !error_string || !save_to_logfile)
		return;

	// create the log message 
	Com_sprintf(buf, sizeof buf, "%s %s %s: %s", CURRENT_DATE, CURRENT_TIME, error_string, s);
	if (print_msg)
		gi.dprintf("* %s *\n", buf);

	port = gi.cvar("port", "0", CVAR_SERVERINFO);

	//determine path  
	Com_sprintf(path, sizeof path, "%s/%d.log", game_path->string, (int)port->value);

	if ((fptr = fopen(path, "a")) != NULL) // append text to log  
	{
		//3.0 make sure there is a line feed  
		if (buf[strlen(buf) - 1] != '\n')
			strcat(buf, "\n");

		fprintf(fptr, "%s", buf);
		fclose(fptr);
		return;
	}
	gi.dprintf("ERROR: Failed to write to server log.\n");
}

qboolean G_StuffPlayerCmds(edict_t* ent, char* s)
{
	if (!ent || !s)
		return false;

	char* dst = ent->client->resp.stuffbuf;

	if (strlen(s) + strlen(dst) > 500)
	{
		//gi.dprintf("buffer full\n");
		return false; // don't overfill the buffer
	}

	strcat(dst, s);

	//gi.dprintf("%s", dst);
	return true;
}

void StuffPlayerCmds (edict_t *ent)
{
	int		i, num;
	int		end=0, start=0;

	if (!ent)
		return;
	
	// if the buffer is empty, then no cmds need to be stuffed
	if (strlen(ent->client->resp.stuffbuf) < 1)
		return;

	// initialize pointer
	if (!ent->client->resp.stuffptr || (ent->client->resp.stuffptr == &ent->client->resp.stuffbuf[0]))
		ent->client->resp.stuffptr = &ent->client->resp.stuffbuf[500];

	// get position
	num = 500 - (&ent->client->resp.stuffbuf[500]-ent->client->resp.stuffptr);

	// search backwards
	for (i=num; i>=0; i--)
	{
		// find the end
		if (!end && (ent->client->resp.stuffbuf[i] == '\n'))
		{
			//gi.dprintf("found end at %d\n", i);
			end = i;
			continue;
		}

		// find the start
		if (!start && (ent->client->resp.stuffbuf[i] == '\n'))
		{
			start = i+1;
			//gi.dprintf("found start at %d\n", start);
			break;
		}
	}

	stuffcmd(ent, &ent->client->resp.stuffbuf[start]);
	//gi.dprintf("stuffed to client: %s", &ent->client->resp.stuffbuf[start]);

	for (i=end; i>=start; i--)
	{
	//	gi.dprintf("zeroed %d\n", i);
		ent->client->resp.stuffbuf[i] = 0;
	}

	// update pointer
	ent->client->resp.stuffptr = &ent->client->resp.stuffbuf[start];	
}

// Paril
// Fixed this so it only cprintf's once.
void V_PrintSayPrefix (edict_t *speaker, edict_t *listener, char *text)
{
	int groupnum;
	char temp[2048];

	if (!dedicated->value)
		return;

	if (G_IsSpectator(speaker))
	{
		gi.cprintf(listener, PRINT_CHAT, "(Spectator) %s", text);
		return;
	}

	temp[0] = 0;
	// if they have a title, print it
	if (strcmp(speaker->myskills.title, ""))
		Com_sprintf (temp, sizeof(temp), "%s ", speaker->myskills.title);
		//gi.cprintf(listener, PRINT_HIGH, "%s ", speaker->myskills.title);

	if (ctf->value && speaker->teamnum)
	{
		groupnum = CTF_GetGroupNum(speaker, NULL);
		if (listener && (speaker->teamnum == listener->teamnum))
		{
			if (groupnum == GROUP_ATTACKERS)
				Com_sprintf (temp, sizeof(temp), "%s(Offense) ", temp);
				//gi.cprintf(listener, PRINT_CHAT, "(Offense) ");
			else if (groupnum == GROUP_DEFENDERS)
				Com_sprintf (temp, sizeof(temp), "%s(Defense) ", temp);
//				gi.cprintf(listener, PRINT_CHAT, "(Defense) ");
		}
		else
		{
			Com_sprintf (temp, sizeof(temp), "%s(%s) ", temp, CTF_GetTeamString(speaker->teamnum));
			//gi.cprintf(listener, PRINT_CHAT, "(%s) ", CTF_GetTeamString(speaker->teamnum));
		}
	}
	Com_sprintf (temp, sizeof(temp), "%s%s", temp, text);

	gi.cprintf (listener, PRINT_CHAT, "%s", temp);
}

char *V_GetClassSkin (edict_t *ent)
{
	switch (ent->myskills.class_num)
	{
	case CLASS_SOLDIER: return class1_skin->string;
	case CLASS_POLTERGEIST: return class2_skin->string;
	case CLASS_VAMPIRE: return class3_skin->string;
	case CLASS_MAGE: return class4_skin->string;
	case CLASS_ENGINEER: return class5_skin->string;
	case CLASS_KNIGHT: return class6_skin->string;
	case CLASS_CLERIC: return class7_skin->string;
	case CLASS_WEAPONMASTER: return class8_skin->string;
	case CLASS_NECROMANCER: return class9_skin->string;
	case CLASS_SHAMAN: return class10_skin->string;
	case CLASS_ALIEN: return class11_skin->string;
	default: return "male/grunt";
	}
}

qboolean V_AssignClassSkin (edict_t *ent, char *s)
{
	int		playernum = ent-g_edicts-1;
	char	*p;
	char	t[64];

	if (!enforce_class_skins->value)
		return false;

	// don't assign class skins in teamplay modes
	if (ctf->value || domination->value || ptr->value)
		return false;

	Com_sprintf(t, sizeof(t), "%s", s);

	if ((p = strrchr(t, '/')) != NULL)
		p[1] = 0;
	else
		strcpy(t, "male/");

	gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", 
		ent->client->pers.netname, V_GetClassSkin(ent)) );

	return true;
}
