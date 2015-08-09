#include "g_local.h"
/*
#define RUNE_SPAWN_BASE			0.02	// base chance of a rune spawning, before level modifiers
#define RUNE_WEAPON_MAXVALUE	5		// maximum modifier for weapon runes	
#define RUNE_ABILITY_MAXVALUE	3		// maximum modifier for ability runes
#define	RUNE_CHANCE_BASE		0.5		// base chance of a +1 modifier
#define RUNE_CHANCE_ADDON		0.05	// addon chance for +1 modifier
#define RUNE_CHANCE_MAX			0.9		// maximum chance for +1 modifier
#define RUNE_COST_BASE			2500
#define RUNE_COST_ADDON			750

void RemoveWeaponRune (edict_t *ent)
{
	if(!ent->myskills.hand.type)
		return;
	
	switch (ent->myskills.hand.modifiers[0])
	{
		case ITEM_SWORD:
			ent->myskills.weapons[WEAPON_SWORD].mods[0].current_level -= ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_SWORD].mods[1].current_level -= ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_SWORD].mods[2].current_level -= ent->myskills.hand.modifiers[3];
			break;
		case ITEM_SHOTGUN:
			ent->myskills.weapons[WEAPON_SHOTGUN].mods[0].current_level -= ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_SHOTGUN].mods[1].current_level	 -= ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_SHOTGUN].mods[2].current_level -= ent->myskills.hand.modifiers[3];
			break;
		case ITEM_SSGUN:
			ent->myskills.weapons[WEAPON_SUPERSHOTGUN].mods[0].current_level -= ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_SUPERSHOTGUN].mods[2].current_level -= ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_SUPERSHOTGUN].mods[3].current_level -= ent->myskills.hand.modifiers[3];
			break;
		case ITEM_MGUN:
			ent->myskills.weapons[WEAPON_MACHINEGUN].mods[0].current_level -= ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_MACHINEGUN].mods[1].current_level -= ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_MACHINEGUN].mods[2].current_level -= ent->myskills.hand.modifiers[3];
			break;
		case ITEM_CGUN:
			ent->myskills.weapons[WEAPON_CHAINGUN].mods[0].current_level -= ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_CHAINGUN].mods[1].current_level -= ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_CHAINGUN].mods[2].current_level -= ent->myskills.hand.modifiers[3];
			break;
		case ITEM_GLAUNCHER:
			ent->myskills.weapons[WEAPON_GRENADELAUNCHER].mods[0].current_level -= ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_GRENADELAUNCHER].mods[2].current_level -= ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_GRENADELAUNCHER].mods[1].current_level -= ent->myskills.hand.modifiers[3];
			break;
		case ITEM_RLAUNCHER:
			ent->myskills.weapons[WEAPON_ROCKETLAUNCHER].mods[0].current_level -= ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_ROCKETLAUNCHER].mods[1].current_level -= ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_ROCKETLAUNCHER].mods[2].current_level -= ent->myskills.hand.modifiers[3];
			break;
		case ITEM_HYPERBLASTER:
			ent->myskills.weapons[WEAPON_HYPERBLASTER].mods[0].current_level -= ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_HYPERBLASTER].mods[1].current_level -= ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_HYPERBLASTER].mods[2].current_level -= ent->myskills.hand.modifiers[3];
			break;
		case ITEM_RAILGUN:
			ent->myskills.weapons[WEAPON_RAILGUN].mods[0].current_level -= ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_RAILGUN].mods[1].current_level -= ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_RAILGUN].mods[2].current_level -= ent->myskills.hand.modifiers[3];
			break;
		case ITEM_BFG:
			ent->myskills.weapons[WEAPON_BFG10K].mods[0].current_level -= ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_BFG10K].mods[1].current_level -= ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_BFG10K].mods[2].current_level -= ent->myskills.hand.modifiers[3];
			break;
		case ITEM_HGRENADES:
			ent->myskills.weapons[WEAPON_HANDGRENADE].mods[0].current_level -= ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_HANDGRENADE].mods[1].current_level -= ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_HANDGRENADE].mods[2].current_level -= ent->myskills.hand.modifiers[3];
			break;
		case ITEM_CANNON:
			ent->myskills.weapons[WEAPON_20MM].mods[0].current_level -= ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_20MM].mods[1].current_level -= ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_20MM].mods[2].current_level -= ent->myskills.hand.modifiers[3];
			break;
		case ITEM_BLASTER:
			ent->myskills.weapons[WEAPON_BLASTER].mods[0].current_level -= ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_BLASTER].mods[1].current_level -= ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_BLASTER].mods[2].current_level -= ent->myskills.hand.modifiers[3];
			break;
	}
}
void ApplyWeaponRune (edict_t *ent)
{
	if(!ent->myskills.hand.type) // Check for a rune again!
		return;
	
	switch (ent->myskills.hand.modifiers[0])
	{
		case ITEM_SWORD:
			ent->myskills.weapons[WEAPON_SWORD].mods[0].current_level += ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_SWORD].mods[1].current_level += ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_SWORD].mods[2].current_level += ent->myskills.hand.modifiers[3];
			break;
		case ITEM_SHOTGUN:
			ent->myskills.weapons[WEAPON_SHOTGUN].mods[0].current_level += ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_SHOTGUN].mods[1].current_level += ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_SHOTGUN].mods[2].current_level += ent->myskills.hand.modifiers[3];
			break;
		case ITEM_SSGUN:
			ent->myskills.weapons[WEAPON_SUPERSHOTGUN].mods[0].current_level += ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_SUPERSHOTGUN].mods[1].current_level += ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_SUPERSHOTGUN].mods[2].current_level += ent->myskills.hand.modifiers[3];
			break;
		case ITEM_MGUN:
			ent->myskills.weapons[WEAPON_MACHINEGUN].mods[0].current_level += ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_MACHINEGUN].mods[1].current_level += ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_MACHINEGUN].mods[2].current_level += ent->myskills.hand.modifiers[3];
			break;
		case ITEM_CGUN:
			ent->myskills.weapons[WEAPON_CHAINGUN].mods[0].current_level += ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_CHAINGUN].mods[1].current_level += ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_CHAINGUN].mods[2].current_level += ent->myskills.hand.modifiers[3];
			break;
		case ITEM_GLAUNCHER:
			ent->myskills.weapons[WEAPON_GRENADELAUNCHER].mods[0].current_level += ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_GRENADELAUNCHER].mods[2].current_level += ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_GRENADELAUNCHER].mods[1].current_level += ent->myskills.hand.modifiers[3];
			break;
		case ITEM_RLAUNCHER:
			ent->myskills.weapons[WEAPON_ROCKETLAUNCHER].mods[0].current_level += ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_ROCKETLAUNCHER].mods[1].current_level += ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_ROCKETLAUNCHER].mods[2].current_level += ent->myskills.hand.modifiers[3];
			break;
		case ITEM_HYPERBLASTER:
			ent->myskills.weapons[WEAPON_HYPERBLASTER].mods[0].current_level += ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_HYPERBLASTER].mods[1].current_level += ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_HYPERBLASTER].mods[2].current_level += ent->myskills.hand.modifiers[3];
			break;
		case ITEM_RAILGUN:
			ent->myskills.weapons[WEAPON_RAILGUN].mods[0].current_level += ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_RAILGUN].mods[1].current_level += ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_RAILGUN].mods[2].current_level += ent->myskills.hand.modifiers[3];
			break;
		case ITEM_BFG:
			ent->myskills.weapons[WEAPON_BFG10K].mods[0].current_level += ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_BFG10K].mods[1].current_level += ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_BFG10K].mods[2].current_level += ent->myskills.hand.modifiers[3];
			break;
		case ITEM_HGRENADES:
			ent->myskills.weapons[WEAPON_HANDGRENADE].mods[0].current_level += ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_HANDGRENADE].mods[1].current_level += ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_HANDGRENADE].mods[2].current_level += ent->myskills.hand.modifiers[3];
			break;
		case ITEM_CANNON:
			ent->myskills.weapons[WEAPON_20MM].mods[0].current_level += ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_20MM].mods[1].current_level += ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_20MM].mods[2].current_level += ent->myskills.hand.modifiers[3];
			break;
		case ITEM_BLASTER:
			ent->myskills.weapons[WEAPON_BLASTER].mods[0].current_level += ent->myskills.hand.modifiers[1];
			ent->myskills.weapons[WEAPON_BLASTER].mods[1].current_level += ent->myskills.hand.modifiers[2];
			ent->myskills.weapons[WEAPON_BLASTER].mods[2].current_level += ent->myskills.hand.modifiers[3];
			break;
	}
}

//************************************************************************************************
//************************************************************************************************

void ApplyAbilityRune (edict_t *ent)
{
	int i;
	if(!ent->myskills.neck.type) // Check for a rune again!
		return;

	//Reset skills to current upgrade level
	for (i = 0; i < MAX_ABILITIES; i++)
		ent->myskills.abilities[i].current_level = ent->myskills.abilities[i].level;

	for(i = 0; i < 8; i += 2)
	{
		int index = ent->myskills.neck.modifiers[i] - 1;
		upgrade_t *upgrade;
		if ((index < 0) || (index > MAX_ABILITIES - 1))
			continue;
		upgrade = ent->myskills.abilities + index;
		upgrade->current_level += ent->myskills.neck.modifiers[(i+1)];
		if (upgrade->current_level > (upgrade->max_level * 2))
			upgrade->current_level = upgrade->max_level* 2;
	}
}

//************************************************************************************************

void RemoveAbilityRune (edict_t *ent)
{
	int i;
	if(!ent->myskills.neck.type) // Check for a rune again!
		return;

	for(i = 0; i < 8; i += 2)
	{
		int index = ent->myskills.neck.modifiers[i] - 1;
		upgrade_t *upgrade;
		if ((index < 0) || (index > MAX_ABILITIES - 1))
			continue;
		upgrade = ent->myskills.abilities + index;
		upgrade->current_level = upgrade->level;
	}
}

//************************************************************************************************
//************************************************************************************************

void rune_getweaponstrings (items_t *item, char *s1, char *s2, char *s3, char *s4)
{
	switch (item->modifiers[0])
	{
	case ITEM_SWORD: strcpy(s1, "Sword"); strcpy(s2, "damage"); 
		strcpy(s3, "refire"); strcpy(s4, "range"); break;
	case ITEM_SHOTGUN: strcpy(s1, "Shotgun"); strcpy(s2, "damage"); 
		strcpy(s3, "refire"); strcpy(s4, "pellets"); break;
	case ITEM_SSGUN: strcpy(s1, "Super Shotgun"); strcpy(s2, "damage"); 
		strcpy(s3, "refire"); strcpy(s4, "pellets"); break;
	case ITEM_MGUN: strcpy(s1, "Machinegun"); strcpy(s2, "damage"); 
		strcpy(s3, "pierce"); strcpy(s4, "tracers"); break;
	case ITEM_CGUN: strcpy(s1, "Chaingun"); strcpy(s2, "damage"); 
		strcpy(s3, "refire"); strcpy(s4, "tracers"); break;
	case ITEM_GLAUNCHER: strcpy(s1, "Grenade Launcher"); strcpy(s2, "damage"); 
		strcpy(s3, "range"); strcpy(s4, "radius"); break;
	case ITEM_RLAUNCHER: strcpy(s1, "Rocket Launcher"); strcpy(s2, "damage"); 
		strcpy(s3, "radius"); strcpy(s4, "speed"); break;
	case ITEM_HYPERBLASTER: strcpy(s1, "Hyperblaster"); strcpy(s2, "damage"); 
		strcpy(s3, "refire"); strcpy(s4, "speed"); break;
	case ITEM_RAILGUN: strcpy(s1, "Railgun"); strcpy(s2, "damage"); 
		strcpy(s3, "refire"); strcpy(s4, "heat"); break;
	case ITEM_BFG: strcpy(s1, "BFG10k"); strcpy(s2, "damage"); 
		strcpy(s3, "stick"); strcpy(s4, "speed"); break;
	case ITEM_HGRENADES: strcpy(s1, "Hand Grenade"); strcpy(s2, "damage"); 
		strcpy(s3, "refire"); strcpy(s4, "radius"); break;
	case ITEM_CANNON: strcpy(s1, "20mm Cannon"); strcpy(s2, "damage"); 
		strcpy(s3, "range"); strcpy(s4, "recoil"); break;
	case ITEM_BLASTER: strcpy(s1, "Blaster"); strcpy(s2, "damage"); 
		strcpy(s3, "bounce"); strcpy(s4, "speed"); break;
	default: gi.dprintf("ERROR: rune_getweaponstrings() called with invalid item %d!\n", item->modifiers[0]);
	}
}
/*
qboolean Pickup_Rune (edict_t *ent, edict_t *other)
{

	items_t *slot;

	if (!other->client)
		return false;

	if (level.time > other->msg_time)
	{
		PrintItemProperties(other, &ent->myitem);
		other->msg_time = level.time+1;
	}

	if (!G_CanPickUpItem(other))
		return false;
	slot = G_FindFreeItemSlot(other);
	ItemCopy(&ent->myitem, slot);
	ent->myitem.owner = other;
	return true;
}

int rune_getmodifier (edict_t *targ, edict_t *attacker, int type)
{
	int		max, mod, val;
	float	chance, levelmod;

	// determine the absolute max value
	if (type == WEAPON_RUNE)
		val = RUNE_WEAPON_MAXVALUE;
	else
		val = RUNE_ABILITY_MAXVALUE;

	levelmod = (float) (targ->myskills.level+1) / (attacker->myskills.level+1);

	chance = RUNE_CHANCE_BASE + RUNE_CHANCE_ADDON*targ->myskills.level*levelmod;

	if (chance > RUNE_CHANCE_MAX)
		chance = RUNE_CHANCE_MAX;
	// determine the maximum possible value for the
	// rune modifier
	for (max=0; max<val; max++) 
	{
		if (random() > chance)
			break;
	}
	// calculate the final random value
	mod = GetRandom(0, max);
	if (mod > val)
		mod = val;
	return mod;
}
/*
void SpawnRune (edict_t *self, edict_t *attacker, qboolean debug)
{
	int		i, max_mods, num_mods;
	int		targ_level;
	float	temp=0;

	gitem_t *item;
	edict_t *rune;

	if (debuginfo->value)
		gi.dprintf("SpawnRune()\n");

	if (ptr->value)
		return;

	attacker = G_GetClient(attacker);
	if (attacker && (attacker != self) && (attacker != world))
	{
		temp = (float) (self->myskills.level + 1) / (attacker->myskills.level + 1);
		// miniboss has greater chance of dropping a rune
		if (IsNewbieBasher(self))
			temp *= 2;
		if (RUNE_SPAWN_BASE*temp < random())
			return;
	}
	else if (debug == false)
		return;

	item = FindItem("Rune"); // get the item properties
	rune = Drop_Item(self, item);// create the entity that holds those properties
	ItemClear(&rune->myitem); // initialize the rune
	targ_level = self->myskills.level;
	rune->myitem.quantity = 1;

	if(random() > 0.5)
	{
		rune->myitem.type = WEAPON_RUNE;
		// base number of possible mods on character level of killed player
		max_mods = 1 + (0.2 * targ_level);
		if (max_mods > 3)
			max_mods = 3;
		num_mods = GetRandom(1, max_mods);	
		rune->myitem.modifiers[0] = GetRandom(1, 13); // random weapon
		for (i=1; i<num_mods+1; i++)
		{
			// base quality of rune on attacker's level
			rune->myitem.modifiers[i] = rune_getmodifier(self, attacker, WEAPON_RUNE);
		}
		rune->s.effects |= EF_PENT;
	}
	else
	{
		rune->myitem.type = ABILITY_RUNE;
		// base number of possible mods on character level of killed player
		max_mods = 2 + (0.2 * targ_level);
		if (max_mods > 4)
			max_mods = 4;
		num_mods = GetRandom(1, max_mods);
		for (i=0; i<2*num_mods; i+=2)
		{
			// get a random ability to mod
			rune->myitem.modifiers[i] = GetRandom(1, 56);
			// base quality of rune on attacker's level
			rune->myitem.modifiers[(i+1)] = rune_getmodifier(self, attacker, ABILITY_RUNE);
		}
		rune->s.effects |= EF_QUAD;
	}

	strcpy(rune->myitem.item_id, GetRandomString(16)); // rune id
	rune->myitem.level = 0.5*GetItemValue(&rune->myitem);
	gi.dprintf("INFO: %s spawned a level %d rune (%s).\n", 
		self->client->pers.netname, rune->myitem.level, rune->myitem.item_id);
}

void PurchaseRandomRune (edict_t *ent)
{
	int		i, cost;
	items_t *slot;

	slot = G_FindFreeItemSlot(ent);
	if (!slot)
	{
		gi.cprintf(ent, PRINT_HIGH, "Not enough inventory space!\n");
		return;
	}

	cost = RUNE_COST_BASE+RUNE_COST_ADDON*ent->myskills.level;
	if (ent->myskills.credits < cost)
	{
		gi.cprintf(ent, PRINT_HIGH, "You need %d credits to purchase a rune.\n", cost);
		return;
	}

	ent->myskills.credits -= cost;
	
	if (random() > 0.5)
	{
		slot->type = WEAPON_RUNE;
		slot->modifiers[0] = GetRandom(1, 13);
		for (i=1; i<4; i++) {
			slot->modifiers[i] = rune_getmodifier(ent, ent, WEAPON_RUNE);
		}
	}
	else
	{
		slot->type = ABILITY_RUNE;
		for (i=0; i<8; i+=2) {
			slot->modifiers[i] = GetRandom(1, 48);
			slot->modifiers[i+1] = rune_getmodifier(ent, ent, ABILITY_RUNE);
		}
	}

	slot->quantity = 1;
	strcpy(slot->item_id, GetRandomString(16));
	slot->level = 0.5*GetItemValue(slot);

	gi.dprintf("INFO: %s purchased a level %d rune (%s).\n", 
		ent->client->pers.netname, slot->level, slot->item_id);
	WriteToLogfile(ent, va("Purchased a level %d rune (%s) for %d credits. Player has %d credits left.\n",
		slot->level, slot->item_id, cost, ent->myskills.credits));
}

*/