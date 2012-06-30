#include "g_local.h"
#include <sys/stat.h>
#include "sqlite3.h"


#define SAVE_VERSION "v1.0"

// az begin

// ===  CVARS  ===
cvar_t *savemethod;

// ===  CONSTS ===
#define TOTAL_TABLES 11
#define TOTAL_INSERTONCE 5
#define TOTAL_RESETTABLES 6

// === QUERIES ===
// these are simply too large to put in the function itself
// and are marked SQLITE because this would be terrible in a big hueg rdbms like mysql

const char* SQLITE_CREATEDBQUERY[TOTAL_TABLES] = 
{
	{"CREATE TABLE [abilities] ([index] INTEGER, [level] INTEGER, [max_level] INTEGER, [hard_max] INTEGER, [modifier] INTEGER,   [disable] INTEGER,   [general_skill] INTEGER)"},
	{"CREATE TABLE [ctf_stats] (  [flag_pickups] INTEGER,   [flag_captures] INTEGER,   [flag_returns] INTEGER,   [flag_kills] INTEGER,   [offense_kills] INTEGER,   [defense_kills] INTEGER,   [assists] INTEGER)"},
	{"CREATE TABLE [game_stats] (  [shots] INTEGER,   [shots_hit] INTEGER,   [frags] INTEGER,   [fragged] INTEGER,   [num_sprees] INTEGER,   [max_streak] INTEGER,   [spree_wars] INTEGER,   [broken_sprees] INTEGER,   [broken_spreewars] INTEGER,   [suicides] INT,   [teleports] INTEGER,   [num_2fers] INTEGER)"},
	{"CREATE TABLE [point_data] (  [exp] INTEGER,   [exptnl] INTEGER,   [level] INTEGER,   [classnum] INTEGER,   [skillpoints] INTEGER,   [credits] INTEGER,   [weap_points] INTEGER,   [resp_weapon] INTEGER,   [tpoints] INTEGER)"},
	{"CREATE TABLE [runes_meta] ([index] INTEGER, [itemtype] INTEGER, [itemlevel] INTEGER, [quantity] INTEGER, [untradeable] INTEGER, [id] CHAR(16), [name] CHAR(24), [nummods] INTEGER, [setcode] INTEGER, [classnum] INTEGER)"},
	{"CREATE TABLE [runes_mods] (  [rune_index] INTEGER, [type] INTEGER, [mod_index] INTEGER, [value] INTEGER, [set] INTEGER)"},
	{"CREATE TABLE [talents] ([id] INTEGER, [upgrade_level] INTEGER, [max_level] INTEGER)"},
	{"CREATE TABLE [userdata] ([title] CHAR(24), [playername] CHAR(64), [password] CHAR(24), [email] CHAR(64), [owner] CHAR(24), [member_since] CHAR(30), [last_played] CHAR(30), [playtime_total] INTEGER,[playingtime] INTEGER)"},
	{"CREATE TABLE [weapon_meta] ([index] INTEGER, [disable] INTEGER)"},
	{"CREATE TABLE [weapon_mods] ([weapon_index] INTEGER, [modindex] INTEGER, [level] INTEGER, [soft_max] INTEGER, [hard_max] INTEGER)"},
	{"CREATE TABLE [character_data] (  [respawns] INTEGER,   [health] INTEGER,   [maxhealth] INTEGER,   [armour] INTEGER,   [maxarmour] INTEGER,   [nerfme] INTEGER,   [adminlevel] INTEGER,   [bosslevel] INTEGER)"}
};

// SAVING
const char* SQLITE_INSERTONCE[TOTAL_INSERTONCE] = 
{
	{"INSERT INTO character_data VALUES (0,0,0,0,0,0,0,0)"},
	{"INSERT INTO ctf_stats VALUES (0,0,0,0,0,0,0)"},
	{"INSERT INTO game_stats VALUES (0,0,0,0,0,0,0,0,0,0,0,0)"},
	{"INSERT INTO point_data VALUES (0,0,0,0,0,0,0,0,0)"},
	{"INSERT INTO userdata VALUES (\"\",\"\",\"\",\"\",\"\",\"\",\"\",0,0)"}
};

const char* SQLITE_RESETTABLES[TOTAL_RESETTABLES] =
{
	{"DELETE FROM abilities;"},
	{"DELETE FROM talents;"},
	{"DELETE FROM runes_meta;"},
	{"DELETE FROM runes_mods;"},
	{"DELETE FROM weapon_meta;"},
	{"DELETE FROM weapon_mods;"}
};

// ab/talent

const char* SQLITE_INSERTABILITY = "INSERT INTO abilities VALUES (%d,%d,%d,%d,%d,%d,%d);";

const char* SQLITE_INSERTTALENT = "INSERT INTO talents VALUES (%d,%d,%d);";

// weapons

const char* SQLITE_INSERTWMETA = "INSERT INTO weapon_meta VALUES (%d,%d);";

const char* SQLITE_INSERTWMOD = "INSERT INTO weapon_mods VALUES (%d,%d,%d,%d,%d);";

// runes

const char* SQLITE_INSERTRMETA = "INSERT INTO runes_meta VALUES (%d,%d,%d,%d,%d,\"%s\",\"%s\",%d,%d,%d);";

const char* SQLITE_INSERTRMOD = "INSERT INTO runes_mods VALUES (%d,%d,%d,%d,%d);";

const char* SQLITE_UPDATECDATA = "UPDATE character_data SET respawns=%d, health=%d, maxhealth=%d, armour=%d, maxarmour=%d, nerfme=%d, adminlevel=%d, bosslevel=%d;";

const char* SQLITE_UPDATESTATS = "UPDATE game_stats SET shots=%d, shots_hit=%d, frags=%d, fragged=%d, num_sprees=%d, max_streak=%d, spree_wars=%d, broken_sprees=%d, broken_spreewars=%d, suicides=%d, teleports=%d, num_2fers=%d;";

const char* SQLITE_UPDATEUDATA = "UPDATE userdata SET title=\"%s\", playername=\"%s\", password=\"%s\", email=\"%s\", owner=\"%s\", member_since=\"%s\", last_played=\"%s\", playtime_total=%d, playingtime=%d;";

const char* SQLITE_UPDATEPDATA = "UPDATE point_data SET exp=%d, exptnl=%d, level=%d, classnum=%d, skillpoints=%d, credits=%d, weap_points=%d, resp_weapon=%d, tpoints=%d;";

const char* SQLITE_UPDATECTFSTATS = "UPDATE ctf_stats SET flag_pickups=%d, flag_captures=%d, flag_returns=%d, flag_kills=%d, offense_kills=%d, defense_kills=%d, assists=%d;";

// LOADING

// this is for tables with only one entry.
const char* SQLITE_VRXSELECT = "SELECT * FROM %s";

//az end

qboolean SavePlayer(edict_t *ent);	//Called by savePlayer(). Don't call this directly.

#if defined(_WIN32) || defined(WIN32)
#include <process.h>

/*
	_beginthread creates threads that automatically terminate when they are finished.
	This is according to the online documentation. If memory leaks are a problem, 
	try using _endthread()

	The best part of these thread calls is that windows.h is not required.

	syntax:	_beginthread(func, stacksize, args)
	func		= thread function. Must have __cdecl identifier.
	stacksize	= size of stack to use. Ususally set to the default value (0).
	args		= void pointer to any arguments you want to send the thread.

	syntax: _endthread()
*/

//Thread function for loading players
void __cdecl OpenPlayerThread(void *arg)
{
	edict_t *ent = (edict_t *)arg;

	if(debuginfo->value)
		gi.dprintf(va("Threading OpenPlayerThread() for %s.\n", ent->client->pers.netname));

	memset(&ent->myskills,0,sizeof(skills_t));
	if(openPlayer(ent))
	{
		gi.cprintf(ent, PRINT_HIGH, "File found! Entering the game.\n");
        ent->threadReturnVal = 0;		//success
	}
	else	ent->threadReturnVal = -1;	//failure

	ent->hThreadFinishTime = level.time;
}

//Thread function for saving players
void __cdecl SavePlayerThread(void *arg)
{
	edict_t *ent = (edict_t *)arg;

	if(debuginfo->value)
		gi.dprintf(va("Threading SavePlayerThread() for %s.\n", ent->client->pers.netname));

	if(SavePlayer(ent))
		ent->threadReturnVal = 0;		//success
	else	ent->threadReturnVal = -1;	//failure

	ent->hThreadFinishTime = level.time;
}
#endif

//Creates an OpenPlayerThread() for loading a player (GDS)
void createOpenPlayerThread(edict_t *ent)
{
#if defined(_WIN32) || defined(WIN32)
	ent->isLoading = true;
	ent->hThread = _beginthread(OpenPlayerThread, 0, ent);

	// Check the return value for success.
	if (ent->hThread == 0)
		gi.dprintf("ERROR: Thread not created.\n");

#else
	gi.dprintf("Multi-threading not supported for non-windows builds.\n");
#endif
} 

//Creates an SavePlayerThread() for saving a player (GDS)
void createSavePlayerThread(edict_t *ent)
{
#if defined(_WIN32) || defined(WIN32)
	ent->isSaving = true;
	ent->hThread = _beginthread(SavePlayerThread, 0, ent);

	// Check the return value for success.
	if (ent->hThread == 0)
		gi.dprintf("ERROR: Thread not created.\n");
	
#else
	gi.dprintf("Multi-threading not supported for non-windows builds.\n");
#endif
} 

//Encrypt the player's password
char *CryptPassword(char *text)
{
	int i;

	if (!text)
		return NULL;
	for (i=0; i<strlen(text) ; i++)
		if ((byte)text[i] > 127)
			text[i]=(byte)text[i]-128;
		else
			text[i]=(byte)text[i]+128;

	return text;
}

//***********************************************************************
//	Load player v 1.0
//***********************************************************************
qboolean ReadPlayer_v1(FILE * fRead, edict_t *player)
{
	int numAbilities, numWeapons, numRunes;
	int i;

	//player's title
    ReadString(player->myskills.title, fRead);	
	//player's in-game name
	ReadString(player->myskills.player_name, fRead);
	//password
	ReadString(player->myskills.password, fRead);
	//email address
	ReadString(player->myskills.email, fRead);
	//owner
	ReadString(player->myskills.owner, fRead);
	//creation date
	ReadString(player->myskills.member_since, fRead);
	//last played date
	ReadString(player->myskills.last_played, fRead);
	//playing time total
	player->myskills.total_playtime =  ReadInteger(fRead);
	//playing time today
	player->myskills.playingtime =  ReadInteger(fRead);

    //begin talents
	player->myskills.talents.count = ReadInteger(fRead);
	for (i = 0; i < player->myskills.talents.count; ++i)
	{
		//don't crash.
        if (i > MAX_TALENTS)
			return false;

		player->myskills.talents.talent[i].id = ReadInteger(fRead);
		player->myskills.talents.talent[i].upgradeLevel = ReadInteger(fRead);
		player->myskills.talents.talent[i].maxLevel = ReadInteger(fRead);
	}
	//end talents

	//begin abilities
	numAbilities = ReadInteger(fRead);
	for (i = 0; i < numAbilities; ++i)
	{
		int index;
		index = ReadInteger(fRead);

		if ((index >= 0) && (index < MAX_ABILITIES))
		{
			player->myskills.abilities[index].level			= ReadInteger(fRead);
			player->myskills.abilities[index].max_level		= ReadInteger(fRead);
			player->myskills.abilities[index].hard_max		= ReadInteger(fRead);
			player->myskills.abilities[index].modifier		= ReadInteger(fRead);
			player->myskills.abilities[index].disable		= (qboolean)ReadInteger(fRead);
			player->myskills.abilities[index].general_skill = (qboolean)ReadInteger(fRead);
		}
		else
		{
			gi.dprintf("Error loading player: %s. Ability index not loaded correctly!\n", player->client->pers.netname);
			WriteToLogfile(player, "ERROR during loading: Ability index not loaded correctly!");
			return false;
		}
	}
	//end abilities

	//begin weapons
    numWeapons = ReadInteger(fRead);
	for (i = 0; i < numWeapons; ++i)
	{
		int index;
		index = ReadInteger(fRead);

		if ((index >= 0 ) && (index < MAX_WEAPONS))
		{
			int j;
			player->myskills.weapons[index].disable = ReadInteger(fRead);

			for (j = 0; j < MAX_WEAPONMODS; ++j)
			{
				player->myskills.weapons[index].mods[j].level = ReadInteger(fRead);
				player->myskills.weapons[index].mods[j].soft_max = ReadInteger(fRead);
				player->myskills.weapons[index].mods[j].hard_max = ReadInteger(fRead);
			}
		}
		else
		{
			gi.dprintf("Error loading player: %s. Weapon index not loaded correctly!\n", player->myskills.player_name);
			WriteToLogfile(player, "ERROR during loading: Weapon index not loaded correctly!");
			return false;
		}
	}
	//end weapons

	//begin runes
	numRunes = ReadInteger(fRead);
	for (i = 0; i < numRunes; ++i)
	{
		int index;
		index = ReadInteger(fRead);
		if ((index >= 0) && (index < MAX_VRXITEMS))
		{
			int j;
			player->myskills.items[index].itemtype = ReadInteger(fRead);
			player->myskills.items[index].itemLevel = ReadInteger(fRead);
			player->myskills.items[index].quantity = ReadInteger(fRead);
			player->myskills.items[index].untradeable = ReadInteger(fRead);
			ReadString(player->myskills.items[index].id, fRead);
			ReadString(player->myskills.items[index].name, fRead);
			player->myskills.items[index].numMods = ReadInteger(fRead);
			player->myskills.items[index].setCode = ReadInteger(fRead);
			player->myskills.items[index].classNum = ReadInteger(fRead);

			for (j = 0; j < MAX_VRXITEMMODS; ++j)
			{
				player->myskills.items[index].modifiers[j].type = ReadInteger(fRead);
				player->myskills.items[index].modifiers[j].index = ReadInteger(fRead);
				player->myskills.items[index].modifiers[j].value = ReadInteger(fRead);
				player->myskills.items[index].modifiers[j].set = ReadInteger(fRead);
			}
		}
	}
	//end runes


	//*****************************
	//standard stats
	//*****************************

	//Exp
	player->myskills.experience =  ReadLong(fRead);
	//next_level
	player->myskills.next_level =  ReadLong(fRead);
	//Level
	player->myskills.level =  ReadInteger(fRead);
	//Class number
	player->myskills.class_num =  ReadInteger(fRead);
	//skill points
	player->myskills.speciality_points =  ReadInteger(fRead);
	//credits
	player->myskills.credits =  ReadInteger(fRead);
	//weapon points
	player->myskills.weapon_points =  ReadInteger(fRead);
	//respawn weapon
	player->myskills.respawn_weapon =  ReadInteger(fRead);
	//talent points
	player->myskills.talents.talentPoints =  ReadInteger(fRead);

	//*****************************
	//in-game stats
	//*****************************
	//respawns
	player->myskills.respawns =  ReadInteger(fRead);
	//health
	player->myskills.current_health =  ReadInteger(fRead);
	//max health
	player->myskills.max_health =  ReadInteger(fRead);
	//armour
	player->myskills.current_armor =  ReadInteger(fRead);
	//max armour
	player->myskills.max_armor =  ReadInteger(fRead);
	//nerfme			(cursing a player maybe?)
	player->myskills.nerfme =  ReadInteger(fRead);

	//*****************************
	//flags
	//*****************************
	//admin flag
	player->myskills.administrator =  ReadInteger(fRead);
	//boss flag
	player->myskills.boss =  ReadInteger(fRead);

	//*****************************
	//stats
	//*****************************
	//shots fired
	player->myskills.shots =  ReadInteger(fRead);
	//shots hit
	player->myskills.shots_hit =  ReadInteger(fRead);
	//frags
	player->myskills.frags =  ReadInteger(fRead);
	//deaths
	player->myskills.fragged =  ReadInteger(fRead);
	//number of sprees
	player->myskills.num_sprees =  ReadInteger(fRead);
	//max spree
	player->myskills.max_streak =  ReadInteger(fRead);
	//number of wars
	player->myskills.spree_wars =  ReadInteger(fRead);
	//number of sprees broken
	player->myskills.break_sprees =  ReadInteger(fRead);
	//number of wars broken
	player->myskills.break_spree_wars =  ReadInteger(fRead);
	//suicides
	player->myskills.suicides =  ReadInteger(fRead);
	//teleports			(link this to "use tballself" maybe?)
	player->myskills.teleports =  ReadInteger(fRead);
	//number of 2fers
	player->myskills.num_2fers =  ReadInteger(fRead);

	//CTF statistics
	player->myskills.flag_pickups =  ReadInteger(fRead);
	player->myskills.flag_captures =  ReadInteger(fRead);
	player->myskills.flag_returns =  ReadInteger(fRead);
	player->myskills.flag_kills =  ReadInteger(fRead);
	player->myskills.offense_kills =  ReadInteger(fRead);
	player->myskills.defense_kills =  ReadInteger(fRead);
	player->myskills.assists =  ReadInteger(fRead);
	//End CTF

	//standard iD inventory
	fread(player->myskills.inventory, sizeof(int), MAX_ITEMS, fRead);

	//Apply runes
	V_ResetAllStats(player);
	for (i = 0; i < 3; ++i)
		V_ApplyRune(player, &player->myskills.items[i]);

	//Apply health
	if (player->myskills.current_health > MAX_HEALTH(player))
		player->myskills.current_health = MAX_HEALTH(player);

	//Apply armor
	if (player->myskills.current_armor > MAX_ARMOR(player))
		player->myskills.current_armor = MAX_ARMOR(player);
	player->myskills.inventory[body_armor_index] = player->myskills.current_armor;

	//done
	return true;
}

//***********************************************************************
//	Save player v 1.0
//***********************************************************************
void WritePlayer_v1(FILE * fWrite, char *playername, edict_t *player)
{
	int i;
	int numAbilities = CountAbilities(player);
	int numWeapons = CountWeapons(player);
	int numRunes = CountRunes(player);

	//save header
	WriteString(fWrite, "Vortex Player File v1.0");
	//player's title
	WriteString(fWrite, player->myskills.title);
	//player's in-game name
	WriteString(fWrite, playername);
	//password
	WriteString(fWrite, player->myskills.password);
	//email address
	WriteString(fWrite, player->myskills.email);
	//owner
	WriteString(fWrite, player->myskills.owner);
	//creation date
	WriteString(fWrite, player->myskills.member_since);
	//last played date
	WriteString(fWrite, player->myskills.last_played);
	//playing time total
	WriteInteger(fWrite, player->myskills.total_playtime);
	//playing time today
	WriteInteger(fWrite, player->myskills.playingtime);

	//begin talents
	WriteInteger(fWrite, player->myskills.talents.count);
	for (i = 0; i < player->myskills.talents.count; ++i)
	{
		WriteInteger(fWrite, player->myskills.talents.talent[i].id);
		WriteInteger(fWrite, player->myskills.talents.talent[i].upgradeLevel);
		WriteInteger(fWrite, player->myskills.talents.talent[i].maxLevel);
	}
	//end talents

	//begin abilities
	WriteInteger(fWrite, numAbilities);
	for (i = 0; i < numAbilities; ++i)
	{
		int index = FindAbilityIndex(i+1, player);
		if (index != -1)
		{
			WriteInteger(fWrite, index);
			WriteInteger(fWrite, player->myskills.abilities[index].level);
			WriteInteger(fWrite, player->myskills.abilities[index].max_level);
			WriteInteger(fWrite, player->myskills.abilities[index].hard_max);
			WriteInteger(fWrite, player->myskills.abilities[index].modifier);
			WriteInteger(fWrite, (int)player->myskills.abilities[index].disable);
			WriteInteger(fWrite, (int)player->myskills.abilities[index].general_skill);
		}
	}
	//end abilities

	//begin weapons
    WriteInteger(fWrite, numWeapons);
	for (i = 0; i < numWeapons; ++i)
	{
		int index = FindWeaponIndex(i+1, player);
		if (index != -1)
		{
			int j;
			WriteInteger(fWrite, index);
			WriteInteger(fWrite, player->myskills.weapons[index].disable);

			for (j = 0; j < MAX_WEAPONMODS; ++j)
			{
				WriteInteger(fWrite, player->myskills.weapons[index].mods[j].level);
				WriteInteger(fWrite, player->myskills.weapons[index].mods[j].soft_max);
				WriteInteger(fWrite, player->myskills.weapons[index].mods[j].hard_max);
			}
		}
	}
	//end weapons

	//begin runes
    WriteInteger(fWrite, numRunes);
	for (i = 0; i < numRunes; ++i)
	{
		int index = FindRuneIndex(i+1, player);
		if (index != -1)
		{
			int j;
			WriteInteger(fWrite, index);
			WriteInteger(fWrite, player->myskills.items[index].itemtype);
			WriteInteger(fWrite, player->myskills.items[index].itemLevel);
			WriteInteger(fWrite, player->myskills.items[index].quantity);
			WriteInteger(fWrite, player->myskills.items[index].untradeable);
			WriteString(fWrite, player->myskills.items[index].id);
			WriteString(fWrite, player->myskills.items[index].name);
			WriteInteger(fWrite, player->myskills.items[index].numMods);
			WriteInteger(fWrite, player->myskills.items[index].setCode);
			WriteInteger(fWrite, player->myskills.items[index].classNum);

			for (j = 0; j < MAX_VRXITEMMODS; ++j)
			{
				WriteInteger(fWrite, player->myskills.items[index].modifiers[j].type);
				WriteInteger(fWrite, player->myskills.items[index].modifiers[j].index);
				WriteInteger(fWrite, player->myskills.items[index].modifiers[j].value);
				WriteInteger(fWrite, player->myskills.items[index].modifiers[j].set);
			}
		}
	}
	//end runes

	//*****************************
	//standard stats
	//*****************************

	//Exp
	WriteLong(fWrite, player->myskills.experience);
	//next_level
	WriteLong(fWrite, player->myskills.next_level);
	//Level
	WriteInteger(fWrite, player->myskills.level);
	//Class number
	WriteInteger(fWrite, player->myskills.class_num);
	//skill points
	WriteInteger(fWrite, player->myskills.speciality_points);
	//credits
	WriteInteger(fWrite, player->myskills.credits);
	//weapon points
	WriteInteger(fWrite, player->myskills.weapon_points);
	//respawn weapon
	WriteInteger(fWrite, player->myskills.respawn_weapon);
	//talent points
	WriteInteger(fWrite, player->myskills.talents.talentPoints);

	//*****************************
	//in-game stats
	//*****************************
	//respawns
	WriteInteger(fWrite, player->myskills.respawns);
	//health
	WriteInteger(fWrite, player->myskills.current_health);
	//max health
	WriteInteger(fWrite, MAX_HEALTH(player));
	//armour
	WriteInteger(fWrite, player->client->pers.inventory[body_armor_index]);
	//max armour
	WriteInteger(fWrite, MAX_ARMOR(player));
	//nerfme			(cursing a player maybe?)
	WriteInteger(fWrite, player->myskills.nerfme);

	//*****************************
	//flags
	//*****************************
	//admin flag
	WriteInteger(fWrite, player->myskills.administrator);
	//boss flag
	WriteInteger(fWrite, player->myskills.boss);

	//*****************************
	//stats
	//*****************************
	//shots fired
	WriteInteger(fWrite, player->myskills.shots);
	//shots hit
	WriteInteger(fWrite, player->myskills.shots_hit);
	//frags
	WriteInteger(fWrite, player->myskills.frags);
	//deaths
	WriteInteger(fWrite, player->myskills.fragged);
	//number of sprees
	WriteInteger(fWrite, player->myskills.num_sprees);
	//max spree
	WriteInteger(fWrite, player->myskills.max_streak);
	//number of wars
	WriteInteger(fWrite, player->myskills.spree_wars);
	//number of sprees broken
	WriteInteger(fWrite, player->myskills.break_sprees);
	//number of wars broken
	WriteInteger(fWrite, player->myskills.break_spree_wars);
	//suicides
	WriteInteger(fWrite, player->myskills.suicides);
	//teleports			(link this to "use tball self" maybe?)
	WriteInteger(fWrite, player->myskills.teleports);
	//number of 2fers
	WriteInteger(fWrite, player->myskills.num_2fers);

	//CTF statistics
	WriteInteger(fWrite, player->myskills.flag_pickups);
	WriteInteger(fWrite, player->myskills.flag_captures);
	WriteInteger(fWrite, player->myskills.flag_returns);
	WriteInteger(fWrite, player->myskills.flag_kills);
	WriteInteger(fWrite, player->myskills.offense_kills);
	WriteInteger(fWrite, player->myskills.defense_kills);
	WriteInteger(fWrite, player->myskills.assists);
	//End CTF

	//Don't let the player have > max cubes
	if (player->client->pers.inventory[power_cube_index] > player->client->pers.max_powercubes)
		player->client->pers.inventory[power_cube_index] = player->client->pers.max_powercubes;

	//standard iD inventory
	fwrite(player->client->pers.inventory, sizeof(int), MAX_ITEMS, fWrite);
}

//***********************************************************************
//		Basic file I/O functions
//***********************************************************************

//Returns the number of lines in a text file
int textFile_CountLines(FILE *fptr, long size)
{
	int count = 0;
	char temp;
	int i = 0;
	
	do
	{
		temp = getc(fptr);
		if (temp == '\n')
			count++;
	}
	while (++i < size);

	rewind(fptr);
	return count;
}

//***********************************************************************

//Iterates to a specific line in a text file (or EOF)
void textFile_GotoLine(FILE *fptr, int linenumber, long size)
{
	int count = 0;
	char temp;
	int i = 0;
	
	do
	{
		temp = fgetc(fptr);
		if (temp == '\n')
			count++;
		if (count == linenumber)
			return;
	}
	while (++i < size);
		return;
}

//************************************************

//***********************************************************************
//		Utility functions
//***********************************************************************

//************************************************

int CountAbilities(edict_t *player)
{
	int i;
	int count = 0;
	for (i = 0; i < MAX_ABILITIES; ++i)
	{
		if (!player->myskills.abilities[i].disable)
			++count;
	}
	return count;
}

//************************************************

int FindAbilityIndex(int index, edict_t *player)
{
    int i;
	int count = 0;
	for (i = 0; i < MAX_ABILITIES; ++i)
	{
		if (!player->myskills.abilities[i].disable)
		{
			++count;
			if (count == index)
				return i;
		}
	}
	return -1;	//just in case something messes up
}

//************************************************
//************************************************

int CountWeapons(edict_t *player)
{
	int i;
	int count = 0;
	for (i = 0; i < MAX_WEAPONS; ++i)
	{
		if (V_WeaponUpgradeVal(player, i) > 0)
			count++;
	}
	return count;
}

//************************************************

int FindWeaponIndex(int index, edict_t *player)
{
	int i;
	int count = 0;
	for (i = 0; i < MAX_WEAPONS; ++i)
	{
		if (V_WeaponUpgradeVal(player, i) > 0)
		{
			count++;
			if (count == index)
				return i;
		}
	}
	return -1;	//just in case something messes up
}

//************************************************
//************************************************

int CountRunes(edict_t *player)
{
	int count = 0;
	int i;

	for (i = 0; i < MAX_VRXITEMS; ++i)
	{
		if (player->myskills.items[i].itemtype != ITEM_NONE)
			++count;
	}
	return count;
}

//************************************************

int FindRuneIndex(int index, edict_t *player)
{
	int i;
	int count = 0;

	for (i = 0; i < MAX_VRXITEMS; ++i)
	{
		if (player->myskills.items[i].itemtype != ITEM_NONE)
		{
			++count;
			if (count == index)
			{
				return i;
			}
		}
	}
	return -1;	//just in case something messes up
}

//************************************************
//************************************************
// az begin
qboolean VSF_SavePlayer(edict_t *player, char *path, qboolean fileexists, char* playername)
{
	sqlite3* db;
	int r, i;
	int numAbilities = CountAbilities(player);
	int numWeapons = CountWeapons(player);
	int numRunes = CountRunes(player);

	r = sqlite3_open(path, &db);

	if (!fileexists)
	{
		sqlite3_stmt *statement;
		// Create initial database.
		
		gi.dprintf("SQLite: creating initial database [%d]... ", r);
		for (i = 0; i < TOTAL_TABLES; i++)
		{
			r = sqlite3_prepare_v2(db, SQLITE_CREATEDBQUERY[i], strlen(SQLITE_CREATEDBQUERY[i]), &statement, NULL);
			r = sqlite3_step(statement);
			sqlite3_finalize(statement); // finish using statement
		}

		for (i = 0; i < TOTAL_INSERTONCE; i++)
		{
			int len = strlen(SQLITE_INSERTONCE[i]);
			r = sqlite3_prepare_v2(db, SQLITE_INSERTONCE[i], len, &statement, NULL); // reset tables
			r = sqlite3_step(statement);
			sqlite3_finalize(statement); // finish using statement
		}
		gi.dprintf("inserted bases.\n", r);
	}

	{ // real saving
		sqlite3_stmt *statement;
		char *format;

		// reset tables (remove records for reinsertion)
		for (i = 0; i < TOTAL_RESETTABLES; i++)
		{
			r = sqlite3_prepare_v2(db, SQLITE_RESETTABLES[i], strlen(SQLITE_RESETTABLES[i]), &statement, NULL);
			r = sqlite3_step(statement);
			sqlite3_finalize(statement); // finish using statement
		}

		format = strdup(va(SQLITE_UPDATEUDATA, 
		 player->myskills.title,
		 playername,
		 player->myskills.password,
		 player->myskills.email,
		 player->myskills.owner,
 		 player->myskills.member_since,
		 player->myskills.last_played,
		 player->myskills.total_playtime,
 		 player->myskills.playingtime));

 		 r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL);
 		 r = sqlite3_step(statement);
		 sqlite3_finalize(statement); // finish using statement
		 free (format);

		// talents
		for (i = 0; i < player->myskills.talents.count; ++i)
		{
			format = strdup(va(SQLITE_INSERTTALENT, player->myskills.talents.talent[i].id,
				player->myskills.talents.talent[i].upgradeLevel,
				player->myskills.talents.talent[i].maxLevel));
			
			r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL);
			r = sqlite3_step(statement);
			sqlite3_finalize(statement);
			free (format);
		}

		// abilities
	
		for (i = 0; i < numAbilities; ++i)
		{
			int index = FindAbilityIndex(i+1, player);
			if (index != -1)
			{
				format = strdup(va(SQLITE_INSERTABILITY, index, 
					player->myskills.abilities[index].level,
					player->myskills.abilities[index].max_level,
					player->myskills.abilities[index].hard_max,
					player->myskills.abilities[index].modifier,
					(int)player->myskills.abilities[index].disable,
					(int)player->myskills.abilities[index].general_skill));
				
				r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL); // insert ability
				if (r == SQLITE_ERROR)
				{
					format = sqlite3_errmsg(db);
					gi.dprintf(format);
				}
				r = sqlite3_step(statement);
				sqlite3_finalize(statement);
				free (format); // this will be slow...
			}
		}
		// gi.dprintf("saved abilities");
		
		//*****************************
		//in-game stats
		//*****************************

		format = strdup(va(SQLITE_UPDATECDATA, 
		 player->myskills.respawns,
		 player->myskills.current_health,
		 MAX_HEALTH(player),
		 player->client->pers.inventory[body_armor_index],
  		 MAX_ARMOR(player),
 		 player->myskills.nerfme,
		 player->myskills.administrator, // flags
		 player->myskills.boss));

		r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL);
		r = sqlite3_step(statement);
		sqlite3_finalize(statement);
		free (format);

		//*****************************
		//stats
		//*****************************

		format = strdup(va(SQLITE_UPDATESTATS, 
		 player->myskills.shots,
		 player->myskills.shots_hit,
		 player->myskills.frags,
		 player->myskills.fragged,
  		 player->myskills.num_sprees,
 		 player->myskills.max_streak,
		 player->myskills.spree_wars,
		 player->myskills.break_sprees,
		 player->myskills.break_spree_wars,
		 player->myskills.suicides,
		 player->myskills.teleports,
		 player->myskills.num_2fers));

		r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL); // insert ability
		r = sqlite3_step(statement);
		sqlite3_finalize(statement);
		free (format);
		
		//*****************************
		//standard stats
		//*****************************
		
		format = strdup(va(SQLITE_UPDATEPDATA, 
		 player->myskills.experience,
		 player->myskills.next_level,
         player->myskills.level,
		 player->myskills.class_num,
		 player->myskills.speciality_points,
 		 player->myskills.credits,
		 player->myskills.weapon_points,
		 player->myskills.respawn_weapon,
		 player->myskills.talents.talentPoints));
		
		r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL); // insert ability
		r = sqlite3_step(statement);
		sqlite3_finalize(statement);
		free (format);

		//begin weapons
		for (i = 0; i < numWeapons; ++i)
		{
			int index = FindWeaponIndex(i+1, player);
			if (index != -1)
			{
				int j;
				format = strdup(va(SQLITE_INSERTWMETA, 
				 index,
				 player->myskills.weapons[index].disable));			
				
				r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL); // insert ability
				r = sqlite3_step(statement);
				sqlite3_finalize(statement);
				free (format);

				for (j = 0; j < MAX_WEAPONMODS; ++j)
				{
					format = strdup(va(SQLITE_INSERTWMOD, 
						index,
						j,
					    player->myskills.weapons[index].mods[j].level,
					    player->myskills.weapons[index].mods[j].soft_max,
					    player->myskills.weapons[index].mods[j].hard_max));

					r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL); // insert ability
					r = sqlite3_step(statement);
					sqlite3_finalize(statement);
					free (format);
				}
			}
		}
		//end weapons

		//begin runes
		for (i = 0; i < numRunes; ++i)
		{
			int index = FindRuneIndex(i+1, player);
			if (index != -1)
			{
				int j;

				format = strdup(va(SQLITE_INSERTRMETA, 
				 index,
				 player->myskills.items[index].itemtype,
				 player->myskills.items[index].itemLevel,
				 player->myskills.items[index].quantity,
				 player->myskills.items[index].untradeable,
				 player->myskills.items[index].id,
				 player->myskills.items[index].name,
				 player->myskills.items[index].numMods,
				 player->myskills.items[index].setCode,
				 player->myskills.items[index].classNum));

				r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL); // insert ability
				r = sqlite3_step(statement);
				sqlite3_finalize(statement);
				free (format);

				for (j = 0; j < MAX_VRXITEMMODS; ++j)
				{
					format = strdup(va(SQLITE_INSERTRMOD, 
						index,
					    player->myskills.items[index].modifiers[j].type,
					    player->myskills.items[index].modifiers[j].index,
					    player->myskills.items[index].modifiers[j].value,
					    player->myskills.items[index].modifiers[j].set));
					
					r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL); // insert ability
					r = sqlite3_step(statement);
					sqlite3_finalize(statement);
					free (format);
				}
			}
		}
		//end runes

		format = strdup(va(SQLITE_UPDATECTFSTATS, 
			player->myskills.flag_pickups,
			player->myskills.flag_captures,
			player->myskills.flag_returns,
			player->myskills.flag_kills,
			player->myskills.offense_kills,
			player->myskills.defense_kills,
			player->myskills.assists));

		r = sqlite3_prepare_v2(db, format, strlen(format), &statement, NULL); // insert ability
		r = sqlite3_step(statement);
		sqlite3_finalize(statement);
		free (format);

	} // end saving

	if (player->client->pers.inventory[power_cube_index] > player->client->pers.max_powercubes)
		player->client->pers.inventory[power_cube_index] = player->client->pers.max_powercubes;

	sqlite3_close(db); // finalize
	return true;
}
// az end

//***********************************************************************
//		Save player to file
//		Max value of a signed int32 (4 bytes) = 2147483648
//		That should be plenty for vrx	-doomie
//***********************************************************************

//Called by savePlayer(). Don't call this directly.
qboolean SavePlayer(edict_t *ent)
{
	char	path[100];
	FILE	*fwrite;
	struct stat exist;
	int file_exists = 0;

	//Make sure this is a client
	if (!ent->client)
	{
		gi.dprintf("ERROR: entity not a client!! (%s)\n",ent->classname);
		return false;
	}

	if(debuginfo->value)
		gi.dprintf("savePlayer called to save: %s\n", ent->client->pers.netname);

	//determine path
	if (savemethod->value == 1)
	{
		#if defined(_WIN32) || defined(WIN32)
			sprintf(path, "%s\\%s.vrx", save_path->string, V_FormatFileName(ent->client->pers.netname));
		#else
			sprintf(path, "%s/%s.vrx", save_path->string, V_FormatFileName(ent->client->pers.netname));
		#endif
	}else
	{
		#if defined(_WIN32) || defined(WIN32)
			sprintf(path, "%s\\%s.vsf", save_path->string, V_FormatFileName(ent->client->pers.netname));
		#else
			sprintf(path, "%s/%s.vsf", save_path->string, V_FormatFileName(ent->client->pers.netname));
		#endif
	}

	if (stat (path, &exist) == 0)
		file_exists = 1;
	

	//Open file for saving
	if (savemethod->value == 1)
		if ((fwrite = fopen(path, "wb")) == NULL)
		{
			gi.dprintf("ERROR: savePlayer() can't open %s.\n", path);
			return false;		
		}
	
	// save the player
	if (savemethod->value == 1)
		WritePlayer_v1(fwrite, ent->client->pers.netname, ent);	
	else
		VSF_SavePlayer(ent, path, file_exists, ent->client->pers.netname);
	

	//done
	if (savemethod->value == 1)
		fclose(fwrite);
	return true;
}

//***********************************************************************
//		Save player (via multi-threading or via local access)
//***********************************************************************

qboolean savePlayer(edict_t *ent)
{
	if(gds->value)
	{
		//Don't create more than one save thread.
		if(!ent->isSaving)
			createSavePlayerThread(ent);

		return true;
	}
	
	return SavePlayer(ent);
}

qboolean VSF_LoadPlayer(edict_t *player, char* path)
{
	sqlite3* db;
	sqlite3_stmt* stmt, *stmt_mods;
	char* format;
	int numAbilities, numWeapons, numRunes;
	int i, r;

	sqlite3_open(path, &db);
	
	format = "SELECT * FROM userdata";
	
	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

    strcpy(player->myskills.title, sqlite3_column_text(stmt, 0));
	strcpy(player->myskills.player_name, sqlite3_column_text(stmt, 1));
	strcpy(player->myskills.password, sqlite3_column_text(stmt, 2));
	strcpy(player->myskills.email, sqlite3_column_text(stmt, 3));
	strcpy(player->myskills.owner, sqlite3_column_text(stmt, 4));
	strcpy(player->myskills.member_since, sqlite3_column_text(stmt, 5));
	strcpy(player->myskills.last_played, sqlite3_column_text(stmt, 6));
	player->myskills.total_playtime =  sqlite3_column_int(stmt, 7);

	player->myskills.playingtime =  sqlite3_column_int(stmt, 8);

	sqlite3_finalize(stmt);

	format = "SELECT COUNT(*) FROM talents";
	
	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

    //begin talents
	player->myskills.talents.count = sqlite3_column_int(stmt, 0);

	sqlite3_finalize(stmt);

	format = "SELECT * FROM talents";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

	for (i = 0; i < player->myskills.talents.count; ++i)
	{
		//don't crash.
        if (i > MAX_TALENTS)
			return false;

		player->myskills.talents.talent[i].id = sqlite3_column_int(stmt, 0);
		player->myskills.talents.talent[i].upgradeLevel = sqlite3_column_int(stmt, 1);
		player->myskills.talents.talent[i].maxLevel = sqlite3_column_int(stmt, 2);


		if ( (r = sqlite3_step(stmt)) == SQLITE_DONE)
			break;
	}
	//end talents
	
	sqlite3_finalize(stmt);

	format = "SELECT COUNT(*) FROM abilities";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

	//begin abilities
	numAbilities = sqlite3_column_int(stmt, 0);

	sqlite3_finalize(stmt);

	format = "SELECT * FROM abilities";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

	for (i = 0; i < numAbilities; ++i)
	{
		int index;
		index = sqlite3_column_int(stmt, 0);

		if ((index >= 0) && (index < MAX_ABILITIES))
		{
			player->myskills.abilities[index].level			= sqlite3_column_int(stmt, 1);
			player->myskills.abilities[index].max_level		= sqlite3_column_int(stmt, 2);
			player->myskills.abilities[index].hard_max		= sqlite3_column_int(stmt, 3);
			player->myskills.abilities[index].modifier		= sqlite3_column_int(stmt, 4);
			player->myskills.abilities[index].disable		= sqlite3_column_int(stmt, 5);
			player->myskills.abilities[index].general_skill = (qboolean)sqlite3_column_int(stmt, 6);

			if ( (r = sqlite3_step(stmt)) == SQLITE_DONE)
				break;
		}
		else
		{
			gi.dprintf("Error loading player: %s. Ability index not loaded correctly!\n", player->client->pers.netname);
			WriteToLogfile(player, "ERROR during loading: Ability index not loaded correctly!");
			return false;
		}
	}
	//end abilities

	sqlite3_finalize(stmt);

	format = "SELECT COUNT(*) FROM weapon_meta";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

	//begin weapons
    numWeapons = sqlite3_column_int(stmt, 0);
	
	sqlite3_finalize(stmt);

	format = "SELECT * FROM weapon_meta";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

	for (i = 0; i < numWeapons; ++i)
	{
		int index;
		index = sqlite3_column_int(stmt, 0);

		if ((index >= 0 ) && (index < MAX_WEAPONS))
		{
			int j;
			player->myskills.weapons[index].disable = sqlite3_column_int(stmt, 1);

			format = strdup(va("SELECT * FROM weapon_mods WHERE weapon_index=%d", index));

			r = sqlite3_prepare_v2(db, format, strlen(format), &stmt_mods, NULL);
			r = sqlite3_step(stmt_mods);

			for (j = 0; j < MAX_WEAPONMODS; ++j)
			{
				
				player->myskills.weapons[index].mods[j].level = sqlite3_column_int(stmt_mods, 2);
				player->myskills.weapons[index].mods[j].soft_max = sqlite3_column_int(stmt_mods, 3);
				player->myskills.weapons[index].mods[j].hard_max = sqlite3_column_int(stmt_mods, 4);
				
				if ((r = sqlite3_step(stmt_mods)) == SQLITE_DONE)
					break;
			}

			free (format);
			sqlite3_finalize(stmt_mods);
		}
		else
		{
			gi.dprintf("Error loading player: %s. Weapon index not loaded correctly!\n", player->myskills.player_name);
			WriteToLogfile(player, "ERROR during loading: Weapon index not loaded correctly!");
			return false;
		}

		if ((r = sqlite3_step(stmt)) == SQLITE_DONE)
			break;

	}

	sqlite3_finalize(stmt);
	//end weapons

	//begin runes

	format = "SELECT COUNT(*) FROM runes_meta";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

	numRunes = sqlite3_column_int(stmt, 0);

	sqlite3_finalize(stmt);

	format = "SELECT * FROM runes_meta";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

	for (i = 0; i < numRunes; ++i)
	{
		int index;
		index = sqlite3_column_int(stmt, 0);
		if ((index >= 0) && (index < MAX_VRXITEMS))
		{
			int j;
			player->myskills.items[index].itemtype = sqlite3_column_int(stmt, 1);
			player->myskills.items[index].itemLevel = sqlite3_column_int(stmt, 2);
			player->myskills.items[index].quantity = sqlite3_column_int(stmt, 3);
			player->myskills.items[index].untradeable = sqlite3_column_int(stmt, 4);
			strcpy(player->myskills.items[index].id, sqlite3_column_text(stmt, 5));
			strcpy(player->myskills.items[index].name, sqlite3_column_text(stmt, 6));
			player->myskills.items[index].numMods = sqlite3_column_int(stmt, 7);
			player->myskills.items[index].setCode = sqlite3_column_int(stmt, 8);
			player->myskills.items[index].classNum = sqlite3_column_int(stmt, 9);

			format = strdup(va("SELECT * FROM runes_mods WHERE rune_index=%d", index));

			r = sqlite3_prepare_v2(db, format, strlen(format), &stmt_mods, NULL);
			r = sqlite3_step(stmt_mods);

			for (j = 0; j < MAX_VRXITEMMODS; ++j)
			{
				player->myskills.items[index].modifiers[j].type = sqlite3_column_int(stmt_mods, 1);
				player->myskills.items[index].modifiers[j].index = sqlite3_column_int(stmt_mods, 2);
				player->myskills.items[index].modifiers[j].value = sqlite3_column_int(stmt_mods, 3);
				player->myskills.items[index].modifiers[j].set = sqlite3_column_int(stmt_mods, 4);

				if ((r = sqlite3_step(stmt_mods)) == SQLITE_DONE)
					break;
			}

			free (format);
			sqlite3_finalize(stmt_mods);
		}

		if ((r = sqlite3_step(stmt)) == SQLITE_DONE)
			break;
	}

	sqlite3_finalize(stmt);
	//end runes


	//*****************************
	//standard stats
	//*****************************

	format = "SELECT * FROM point_data";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

	//Exp
	player->myskills.experience =  sqlite3_column_int(stmt, 0);
	//next_level
	player->myskills.next_level =  sqlite3_column_int(stmt, 1);
	//Level
	player->myskills.level =  sqlite3_column_int(stmt, 2);
	//Class number
	player->myskills.class_num = sqlite3_column_int(stmt, 3);
	//skill points
	player->myskills.speciality_points = sqlite3_column_int(stmt, 4);
	//credits
	player->myskills.credits = sqlite3_column_int(stmt, 5);
	//weapon points
	player->myskills.weapon_points = sqlite3_column_int(stmt, 6);
	//respawn weapon
	player->myskills.respawn_weapon = sqlite3_column_int(stmt, 7);
	//talent points
	player->myskills.talents.talentPoints = sqlite3_column_int(stmt, 8);

	sqlite3_finalize(stmt);

	format = "SELECT * FROM character_data";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);


	//*****************************
	//in-game stats
	//*****************************
	//respawns
	player->myskills.respawns = sqlite3_column_int(stmt, 0);
	//health
	player->myskills.current_health = sqlite3_column_int(stmt, 1);
	//max health
	player->myskills.max_health = sqlite3_column_int(stmt, 2);
	//armour
	player->myskills.current_armor = sqlite3_column_int(stmt, 3);
	//max armour
	player->myskills.max_armor = sqlite3_column_int(stmt, 4);
	//nerfme			(cursing a player maybe?)
	player->myskills.nerfme = sqlite3_column_int(stmt, 5);

	//*****************************
	//flags
	//*****************************
	//admin flag
	player->myskills.administrator =  sqlite3_column_int(stmt, 6);
	//boss flag
	player->myskills.boss = sqlite3_column_int(stmt, 7);

	//*****************************
	//stats
	//*****************************

	sqlite3_finalize(stmt);

	format = "SELECT * FROM game_stats";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

	//shots fired
	player->myskills.shots = sqlite3_column_int(stmt, 0);
	//shots hit
	player->myskills.shots_hit = sqlite3_column_int(stmt, 1);
	//frags
	player->myskills.frags = sqlite3_column_int(stmt, 2);
	//deaths
	player->myskills.fragged = sqlite3_column_int(stmt, 3);
	//number of sprees
	player->myskills.num_sprees = sqlite3_column_int(stmt, 4);
	//max spree
	player->myskills.max_streak = sqlite3_column_int(stmt, 5);
	//number of wars
	player->myskills.spree_wars =  sqlite3_column_int(stmt, 6);
	//number of sprees broken
	player->myskills.break_sprees =  sqlite3_column_int(stmt, 7);
	//number of wars broken
	player->myskills.break_spree_wars = sqlite3_column_int(stmt, 8);
	//suicides
	player->myskills.suicides = sqlite3_column_int(stmt, 9);
	//teleports			(link this to "use tballself" maybe?)
	player->myskills.teleports =  sqlite3_column_int(stmt, 10);
	//number of 2fers
	player->myskills.num_2fers = sqlite3_column_int(stmt, 11);

	sqlite3_finalize(stmt);

	format = "SELECT * FROM ctf_stats";

	r = sqlite3_prepare_v2(db, format, strlen(format), &stmt, NULL);
	r = sqlite3_step(stmt);

	//CTF statistics
	player->myskills.flag_pickups =  sqlite3_column_int(stmt, 0);
	player->myskills.flag_captures =  sqlite3_column_int(stmt, 1);
	player->myskills.flag_returns =  sqlite3_column_int(stmt, 2);
	player->myskills.flag_kills =  sqlite3_column_int(stmt, 3);
	player->myskills.offense_kills =  sqlite3_column_int(stmt, 4);
	player->myskills.defense_kills =  sqlite3_column_int(stmt, 5);
	player->myskills.assists =  sqlite3_column_int(stmt, 6);
	//End CTF

	sqlite3_finalize(stmt);

	//Apply runes
	V_ResetAllStats(player);
	for (i = 0; i < 3; ++i)
		V_ApplyRune(player, &player->myskills.items[i]);

	//Apply health
	if (player->myskills.current_health > MAX_HEALTH(player))
		player->myskills.current_health = MAX_HEALTH(player);

	//Apply armor
	if (player->myskills.current_armor > MAX_ARMOR(player))
		player->myskills.current_armor = MAX_ARMOR(player);
	player->myskills.inventory[body_armor_index] = player->myskills.current_armor;

	//done
	sqlite3_close(db);
	return true;
}

//***********************************************************************
//		open player from file
//***********************************************************************

qboolean openPlayer(edict_t *ent)
{
	char	path[100];
	FILE	*fread;
	int		i;
	int		loadmode = 0;
	char	version[64];

	//Make sure this is a client
	if (!ent->client)
	{
		gi.dprintf("ERROR: entity not a client!! (%s)\n",ent->classname);
		return false;
	}

	if(debuginfo->value)
		gi.dprintf("openPlayer called to open: %s\n", ent->client->pers.netname);

	//Reset the player's skills_t
	memset(&ent->myskills,0,sizeof(skills_t));

	//determine path
	#if defined(_WIN32) || defined(WIN32)
		sprintf(path, "%s\\%s.vsf", save_path->string, V_FormatFileName(ent->client->pers.netname));
	#else
		sprintf(path, "%s/%s.vsf", save_path->string, V_FormatFileName(ent->client->pers.netname));
	#endif

	//Open file for loading
	if ((fread = fopen(path, "rb")) == NULL)
	{
		gi.dprintf("INFO: openPlayer can't open %s. This probably means the file does not exist.\n", path);
		#if defined(_WIN32) || defined(WIN32)
			sprintf(path, "%s\\%s.vrx", save_path->string, V_FormatFileName(ent->client->pers.netname));
		#else
			sprintf(path, "%s/%s.vrx", save_path->string, V_FormatFileName(ent->client->pers.netname));
		#endif

		if ((fread = fopen(path, "rb")) == NULL)
		{
			gi.dprintf("INFO: openPlayer can't open %s. There are no characters with this name.\n", path);
			return false;
		}

	}else
		loadmode = 1; // do sqlite

	//disable all abilities
	for (i = 0; i < MAX_ABILITIES; ++i)
	{
		ent->myskills.abilities[i].disable = true;
	}

	if (loadmode == 1)
	{
		return VSF_LoadPlayer(ent, path); // end right here- we're doing sqlite
	}

	// .vrx file format loading ahead (deprecated)
	//read header (player file version number)
	ReadString(version, fread);

	//check for correct version number
	
	//begin reading player, if there was an error, return false :)
	if (ReadPlayer_v1(fread, ent) == false)
	{
		fclose(fread);
		return false;
	}
	
	
	return true;
}

void SaveAllPlayers()
{
	int i;
	edict_t* ent;
	for (i = 0; i < maxclients->value; i++) {
			ent = &g_edicts[i];
			if (!G_EntExists(ent))
				continue;
			SaveCharacter(ent);
		}
		gi.dprintf("INFO: All players saved.\n");
}
