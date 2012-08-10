#ifndef NO_GDS

#include <my_global.h>
#include <mysql.h>
#include "g_local.h"
#include "gds.h"

#ifndef GDS_NOMULTITHREAD
#include <pthread.h>
#endif

#ifdef _WIN32
#pragma warning ( disable : 4090 ; disable : 4996 )
#endif

// *********************************
// Definitions
// *********************************

#define DEFAULT_DATABASE "127.0.0.1"
#define MYSQL_PW "c4r1t4"
#define MYSQL_USER "root"
#define MYSQL_DBNAME "vrxcltest"

/* 
These are modified versions of the sqlite version
the difference here is that these include ID insertion
which is of course, important when you can't create one database per character. -az
*/

// abilities

const char* MYSQL_INSERTABILITY = "INSERT INTO abilities VALUES (%d,%d,%d,%d,%d,%d,%d,%d);";

// talents

const char* MYSQL_INSERTTALENT = "INSERT INTO talents VALUES (%d,%d,%d,%d);";

// weapons

const char* MYSQL_INSERTWMETA = "INSERT INTO weapon_meta VALUES (%d,%d,%d);";

const char* MYSQL_INSERTWMOD = "INSERT INTO weapon_mods VALUES (%d,%d,%d,%d,%d,%d);";

// runes

const char* MYSQL_INSERTRMETA = "INSERT INTO runes_meta VALUES (%d,%d,%d,%d,%d,%d,\"%s\",\"%s\",%d,%d,%d);";

const char* MYSQL_INSERTRMOD = "INSERT INTO runes_mods VALUES (%d,%d,%d,%d,%d,%d);";

// update stuff

const char* MYSQL_UPDATECDATA = "UPDATE character_data SET respawns=%d, health=%d, maxhealth=%d, armour=%d, maxarmour=%d, nerfme=%d, adminlevel=%d, bosslevel=%d WHERE char_idx=%d;";

const char* MYSQL_UPDATESTATS = "UPDATE game_stats SET shots=%d, shots_hit=%d, frags=%d, fragged=%d, num_sprees=%d, max_streak=%d, spree_wars=%d, broken_sprees=%d, broken_spreewars=%d, suicides=%d, teleports=%d, num_2fers=%d WHERE char_idx=%d;";

const char* MYSQL_UPDATEUDATA = "UPDATE userdata SET title=\"%s\", playername=\"%s\", password=\"%s\", email=\"%s\", owner=\"%s\", member_since=\"%s\", last_played=\"%s\", playtime_total=%d, playingtime=%d WHERE char_idx=%d;";

const char* MYSQL_UPDATEPDATA = "UPDATE point_data SET exp=%d, exptnl=%d, level=%d, classnum=%d, skillpoints=%d, credits=%d, weap_points=%d, resp_weapon=%d, tpoints=%d WHERE char_idx=%d;";

const char* MYSQL_UPDATECTFSTATS = "UPDATE ctf_stats SET flag_pickups=%d, flag_captures=%d, flag_returns=%d, flag_kills=%d, offense_kills=%d, defense_kills=%d, assists=%d WHERE char_idx=%d;";

/* 
	This global instance is used in our only thread- where everything is queued.
	I personally prefer it this way as it simplifies things,
	decreases the amount of connections to the DB and 
	makes the multithread business actually easy to sync.
	It might be slower- it IS slower, but it takes a lot of
	work to make it the kots2007 way. Anyone's welcome to do it the kots2007 way
	as long as you make it work. But do it yourself. I'm a hobbyist, not a professional. -az
*/

// MYSQL
MYSQL* GDS_MySQL = NULL;

// Queue
typedef struct
{
	edict_t *ent;
	int operation;
	int id;
	void *next;
	skills_t *myskills;
} gds_queue_t;

gds_queue_t *first;
gds_queue_t *last;

gds_queue_t *free_first = NULL;
gds_queue_t *free_last = NULL;
// CVARS
cvar_t *gds_debug;

// Threading
#ifndef GDS_NOMULTITHREAD

pthread_t QueueThread;
pthread_attr_t attr;
pthread_mutex_t QueueMutex;
pthread_mutex_t StatusMutex;
#endif

// Prototypes
void V_GDS_Save(gds_queue_t *myskills, MYSQL* db);
qboolean V_GDS_Load(gds_queue_t *current, MYSQL *db);

// *********************************
// QUEUE functions
// *********************************
void V_GDS_FreeQueue_Add(gds_queue_t *current)
{
	if (!free_first)
	{
		free_last = free_first = current;
	}
	else
	{
		free_last->next = current;
		free_last = current;
		free_last->next = NULL;
	}
}

void V_GDS_FreeMemory_Queue()
{
	gds_queue_t *next;
	while (free_first)
	{
		if (free_first->operation == GDS_SAVE)
			gi.TagFree(free_first->myskills);

		next = free_first->next;
		gi.TagFree(free_first);
		free_first = next;
	}
}

void V_GDS_Queue_Add(edict_t *ent, int operation)
{
	if ((!ent || !ent->client) && operation != GDS_EXITTHREAD)
	{
		gi.dprintf("V_GDS_Queue_Add: Null Entity or Client!\n");
	}

	if (operation != GDS_SAVE && operation != GDS_LOAD && operation != GDS_EXITTHREAD)
	{
		gi.dprintf("V_GDS_Queue_Add: Called with no operation!\n");
		return;
	}

	if (operation == GDS_EXITTHREAD && ent)
	{
		//if (gds_debug->value)
			gi.dprintf("V_GDS_Queue_Add: Called with an entity on an exit thread operation?\n");
	}

#ifndef GDS_NOMULTITHREADING
	pthread_mutex_lock(&QueueMutex);
#endif

	if (!first)
	{
		first = gi.TagMalloc(TAG_GAME, sizeof(gds_queue_t));
		last = first;
		first->ent = NULL;
	}
	
	if (first->ent) // warning: we assume first is != NULL!
	{
		gds_queue_t *newest = last; // Start by the last slot
		
		while (newest->next) // Find a slot that doesn't have a next node.
		{
			newest = newest->next;
		}

		// Use this empty next node 
		// for a new node
		newest->next = gi.TagMalloc(TAG_GAME, sizeof(gds_queue_t)); 

		last = newest->next; // Use the latest allocated slot

	}

	last->ent = ent;
	last->next = NULL; // make sure last node has null pointer 
	last->operation = operation;
	last->id = ent? ent->PlayerID : 0;

	if (operation == GDS_SAVE)
	{
		last->myskills = gi.TagMalloc(TAG_GAME, sizeof(skills_t));
		memcpy(last->myskills, &ent->myskills, sizeof(skills_t));
	}
	
#ifndef GDS_NOMULTITHREADING
	pthread_mutex_unlock(&QueueMutex);
#endif
}

gds_queue_t *V_GDS_Queue_PopFirst()
{
	gds_queue_t *node;

#ifndef GDS_NOMULTITHREADING
	pthread_mutex_lock(&QueueMutex);
#endif
	node = first; // save node on top to delete
	
	if (first && first->operation) // does our node have something to do?
		first = first->next; // set first to next node

	// give me the assgined entity of the first node
	// node must be freed by caller
#ifndef GDS_NOMULTITHREADING
	pthread_mutex_unlock(&QueueMutex);
#endif
	return node; 
}

// *********************************
// Database Thread
// *********************************

void *ProcessQueue(void *unused)
{
	gds_queue_t *current = NULL;
#ifndef GDS_NOMULTITHREADING
	int Work = 1;

	while (Work)
#else
	while (current)
#endif
	{
		current = V_GDS_Queue_PopFirst();

		if (!current)
			continue;
		
		if (current->operation == GDS_LOAD)
		{
			V_GDS_Load(current, GDS_MySQL);
		}else if (current->operation == GDS_SAVE)
		{
			V_GDS_Save(current, GDS_MySQL);
		}
#ifndef GDS_NOMULTITHREAD
		else if (current->operation == GDS_EXITTHREAD)
			Work = 0;
#endif
		V_GDS_FreeQueue_Add(current);
	}

#ifndef GDS_NOMULTITHREADING
	pthread_exit(NULL);
#endif
	return NULL;
}

// *********************************
// MYSQL functions
// *********************************

#define QUERY(a, ...) format = strdup(va(a, __VA_ARGS__));\
	mysql_query(db, format);\
	free (format);

#define GET_RESULT result = mysql_store_result(db);\
	row = mysql_fetch_row(result);

#define FREE_RESULT mysql_free_result(result);

void V_GDS_Save(gds_queue_t *current, MYSQL* db)
{
	int i;
	int id; // character ID
	int numAbilities = CountAbilities(current->ent);
	int numWeapons = CountWeapons(current->ent);
	int numRunes = CountRunes(current->ent);
	char *format;
	MYSQL_ROW row;
	MYSQL_RES *result;

	if (!db)
	{
		if (gds_debug->value)
			gi.dprintf("DB: NULL database (V_GDS_Save())\n");
		return;
	}

	QUERY ("CALL CharacterExists(\"%s\", @exists);", current->myskills->player_name);
	
	QUERY ("SELECT @exists;", current->myskills->player_name);

	GET_RESULT;

	if (!strcmp(row[0], "0"))
		i = 0;
	else
		i = 1;
	
	FREE_RESULT;

	if (!i) // does not exist
	{
		// Create initial database.
		gi.dprintf("DB: Creating character \"%s\"!\n", current->myskills->player_name);
		QUERY ("CALL FillNewChar(\"%s\");", current->myskills->player_name);
	}

	QUERY ("CALL GetCharID(\"%s\", @PID);", current->myskills->player_name);
	
	QUERY ("SELECT @PID;", current->myskills->player_name); 

	GET_RESULT;

	id = atoi(row[0]);

	FREE_RESULT;

	{ // real saving
		mysql_query(db, "START TRANSACTION");
		// reset tables (remove records for reinsertion)
		QUERY("CALL ResetTables(\"%s\");", current->myskills->player_name);

		QUERY (MYSQL_UPDATEUDATA, 
		 current->myskills->title,
		 current->myskills->player_name,
		 current->myskills->password,
		 current->myskills->email,
		 current->myskills->owner,
 		 current->myskills->member_since,
		 current->myskills->last_played,
		 current->myskills->total_playtime,
 		 current->myskills->playingtime, id);

		// talents
		for (i = 0; i < current->myskills->talents.count; ++i)
		{
			QUERY (MYSQL_INSERTTALENT, id, current->myskills->talents.talent[i].id,
				current->myskills->talents.talent[i].upgradeLevel,
				current->myskills->talents.talent[i].maxLevel);
		}

		// abilities
	
		for (i = 0; i < numAbilities; ++i)
		{
			int index = FindAbilityIndex(i+1, current->ent);
			if (index != -1)
			{
				QUERY (MYSQL_INSERTABILITY, id, index, 
					current->myskills->abilities[index].level,
					current->myskills->abilities[index].max_level,
					current->myskills->abilities[index].hard_max,
					current->myskills->abilities[index].modifier,
					(int)current->myskills->abilities[index].disable,
					(int)current->myskills->abilities[index].general_skill);
			}
		}
		// gi.dprintf("saved abilities");
		
		//*****************************
		//in-game stats
		//*****************************

		QUERY (MYSQL_UPDATECDATA,
		 current->myskills->respawns,
		 current->myskills->current_health,
		 MAX_HEALTH(current->ent),
		 current->ent->client->pers.inventory[body_armor_index],
  		 MAX_ARMOR(current->ent),
 		 current->myskills->nerfme,
		 current->myskills->administrator, // flags
		 current->myskills->boss,  id); // last param WHERE char_idx = %d

		//*****************************
		//stats
		//*****************************

		QUERY (MYSQL_UPDATESTATS, 
		 current->myskills->shots,
		 current->myskills->shots_hit,
		 current->myskills->frags,
		 current->myskills->fragged,
  		 current->myskills->num_sprees,
 		 current->myskills->max_streak,
		 current->myskills->spree_wars,
		 current->myskills->break_sprees,
		 current->myskills->break_spree_wars,
		 current->myskills->suicides,
		 current->myskills->teleports,
		 current->myskills->num_2fers, id);
		
		//*****************************
		//standard stats
		//*****************************
		
		QUERY(MYSQL_UPDATEPDATA, 
		 current->myskills->experience,
		 current->myskills->next_level,
         current->myskills->level,
		 current->myskills->class_num,
		 current->myskills->speciality_points,
 		 current->myskills->credits,
		 current->myskills->weapon_points,
		 current->myskills->respawn_weapon,
		 current->myskills->talents.talentPoints, id);

		//begin weapons
		for (i = 0; i < numWeapons; ++i)
		{
			int index = FindWeaponIndex(i+1, current->ent);
			if (index != -1)
			{
				int j;
				QUERY (MYSQL_INSERTWMETA, id, index,
				 current->myskills->weapons[index].disable);			

				for (j = 0; j < MAX_WEAPONMODS; ++j)
				{
					QUERY (MYSQL_INSERTWMOD, id, index,	j,
					    current->myskills->weapons[index].mods[j].level,
					    current->myskills->weapons[index].mods[j].soft_max,
					    current->myskills->weapons[index].mods[j].hard_max);
				}
			}
		}
		//end weapons

		//begin runes
		for (i = 0; i < numRunes; ++i)
		{
			int index = FindRuneIndex(i+1, current->ent);
			if (index != -1)
			{
				int j;

				QUERY (MYSQL_INSERTRMETA, id, 
				 index,
				 current->myskills->items[index].itemtype,
				 current->myskills->items[index].itemLevel,
				 current->myskills->items[index].quantity,
				 current->myskills->items[index].untradeable,
				 current->myskills->items[index].id,
				 current->myskills->items[index].name,
				 current->myskills->items[index].numMods,
				 current->myskills->items[index].setCode,
				 current->myskills->items[index].classNum);

				for (j = 0; j < MAX_VRXITEMMODS; ++j)
				{
					QUERY(MYSQL_INSERTRMOD, id, index,
					    current->myskills->items[index].modifiers[j].type,
					    current->myskills->items[index].modifiers[j].index,
					    current->myskills->items[index].modifiers[j].value,
					    current->myskills->items[index].modifiers[j].set);
				}
			}
		}
		//end runes

		QUERY (MYSQL_UPDATECTFSTATS, 
			current->myskills->flag_pickups,
			current->myskills->flag_captures,
			current->myskills->flag_returns,
			current->myskills->flag_kills,
			current->myskills->offense_kills,
			current->myskills->defense_kills,
			current->myskills->assists, id);

	} // end saving

	mysql_query(db, "COMMIT;");

	if (current->ent->client->pers.inventory[power_cube_index] > current->ent->client->pers.max_powercubes)
		current->ent->client->pers.inventory[power_cube_index] = current->ent->client->pers.max_powercubes;

	pthread_mutex_lock(&StatusMutex);

	if (current->ent->PlayerID == current->id)
		current->ent->ThreadStatus = 3;

	pthread_mutex_unlock(&StatusMutex);
}

qboolean V_GDS_Load(gds_queue_t *current, MYSQL *db)
{
	char* format;
	int numAbilities, numWeapons, numRunes;
	int i;
	int id;
	edict_t *player = current->ent;
	MYSQL_ROW row;
	MYSQL_RES *result, *result_b;

	if (!db)
		return false;

	if (player->PlayerID != current->id)
		return false; // Heh.

	QUERY ("CALL CharacterExists(\"%s\", @Exists);", player->client->pers.netname);

	QUERY ("SELECT @Exists;", current->ent->myskills.player_name); 

	GET_RESULT;

	id = atoi(row[0]);

	if (id == 0)
	{
		pthread_mutex_lock(&StatusMutex);
		player->ThreadStatus = 2;
		pthread_mutex_unlock(&StatusMutex);
		return false;
	}

	FREE_RESULT;

	QUERY ("CALL GetCharID(\"%s\", @PID);", current->ent->client->pers.netname);
	
	QUERY ("SELECT @PID;", current->ent->myskills.player_name); 

	GET_RESULT;

	id = atoi(row[0]);

	FREE_RESULT;

	QUERY( "SELECT * FROM userdata WHERE char_idx=%d", id );

	GET_RESULT;

	if (row)
	{

		strcpy(player->myskills.title, row[1]);
		strcpy(player->myskills.player_name, row[2]);
		strcpy(player->myskills.password, row[3]);
		strcpy(player->myskills.email, row[4]);
		strcpy(player->myskills.owner, row[5]);
		strcpy(player->myskills.member_since, row[6]);
		strcpy(player->myskills.last_played, row[7]);
		player->myskills.total_playtime =  atoi(row[8]);

		player->myskills.playingtime = atoi(row[9]);
	}
	else return false;

	FREE_RESULT;

	if (player->PlayerID != current->id)
		return false; // .. Don't waste time...

	QUERY( "SELECT COUNT(*) FROM talents WHERE char_idx=%d", id );
	
	GET_RESULT;
    //begin talents
	player->myskills.talents.count = atoi(row[0]);

	FREE_RESULT;

	QUERY( "SELECT * FROM talents WHERE char_idx=%d", id );

	GET_RESULT;

	for (i = 0; i < player->myskills.talents.count; ++i)
	{
		//don't crash.
        if (i > MAX_TALENTS)
			return false;

		player->myskills.talents.talent[i].id = atoi(row[1]);
		player->myskills.talents.talent[i].upgradeLevel = atoi(row[2]);
		player->myskills.talents.talent[i].maxLevel = atoi(row[3]);

		row = mysql_fetch_row(result);
		if ( !row )
			break;
	}

	FREE_RESULT;
	//end talents

	if (player->PlayerID != current->id)
		return false; // Still here.

	QUERY( "SELECT COUNT(*) FROM abilities WHERE char_idx=%d", id );

	GET_RESULT;

	//begin abilities
	numAbilities = atoi(row[0]);
	
	FREE_RESULT;

	QUERY( "SELECT * FROM abilities WHERE char_idx=%d", id );

	GET_RESULT

	for (i = 0; i < numAbilities; ++i)
	{
		int index;
		index = atoi(row[1]);

		if ((index >= 0) && (index < MAX_ABILITIES))
		{
			player->myskills.abilities[index].level			= atoi(row[2]);
			player->myskills.abilities[index].max_level		= atoi(row[3]);
			player->myskills.abilities[index].hard_max		= atoi(row[4]);
			player->myskills.abilities[index].modifier		= atoi(row[5]);
			player->myskills.abilities[index].disable		= atoi(row[6]);
			player->myskills.abilities[index].general_skill = atoi(row[7]);
			
			row = mysql_fetch_row(result);
			if ( !row )
				break;
		}
	}
	//end abilities

	FREE_RESULT;

	if (player->PlayerID != current->id)
		return false; // Patient enough...

	QUERY( "SELECT COUNT(*) FROM weapon_meta WHERE char_idx=%d", id );

	GET_RESULT;

	//begin weapons
    numWeapons = atoi(row[0]);
	
	FREE_RESULT;

	QUERY( "SELECT * FROM weapon_meta WHERE char_idx=%d", id );

	result_b = mysql_store_result(db);
	row = mysql_fetch_row(result_b);

	for (i = 0; i < numWeapons; ++i)
	{
		int index;
		index = atoi(row[1]);

		if ((index >= 0 ) && (index < MAX_WEAPONS))
		{
			int j;
			player->myskills.weapons[index].disable = atoi(row[2]);

			QUERY ("SELECT * FROM weapon_mods WHERE weapon_index=%d AND char_idx=%d", index, id);

			GET_RESULT;

			if (row)
			{
				for (j = 0; j < MAX_WEAPONMODS; ++j)
				{
				
					player->myskills.weapons[index].mods[j].level = atoi(row[2]);
					player->myskills.weapons[index].mods[j].soft_max = atoi(row[3]);
					player->myskills.weapons[index].mods[j].hard_max = atoi(row[4]);
				
					row = mysql_fetch_row(result);
					if (!row)
						break;
				}
			}

			FREE_RESULT;

			
		}

		row = mysql_fetch_row(result_b);
		if (!row)
			break;

	}

	mysql_free_result(result_b);
	//end weapons

	if (player->PlayerID != current->id)
		return false; // Why quit now?

	//begin runes

	QUERY ("SELECT COUNT(*) FROM runes_meta WHERE char_idx=%d", id);

	GET_RESULT;

	numRunes = atoi(row[0]);

	FREE_RESULT;

	QUERY( "SELECT * FROM runes_meta WHERE char_idx=%d", id );

	GET_RESULT;

	for (i = 0; i < numRunes; ++i)
	{
		int index;
		index = atoi(row[1]);
		if ((index >= 0) && (index < MAX_VRXITEMS))
		{
			int j;
			player->myskills.items[index].itemtype = atoi(row[2]);
			player->myskills.items[index].itemLevel = atoi(row[3]);
			player->myskills.items[index].quantity = atoi(row[4]);
			player->myskills.items[index].untradeable = atoi(row[5]);
			strcpy(player->myskills.items[index].id, row[6]);
			strcpy(player->myskills.items[index].name, row[7]);
			player->myskills.items[index].numMods = atoi(row[8]);
			player->myskills.items[index].setCode = atoi(row[9]);
			player->myskills.items[index].classNum = atoi(row[10]);

			QUERY ("SELECT * FROM runes_mods WHERE rune_index=%d AND char_idx=%d", index, id);

			result_b = mysql_store_result(db);
			
			if (result_b)
				row = mysql_fetch_row(result_b);
			else
				row = NULL;

			if (row)
			{
				for (j = 0; j < MAX_VRXITEMMODS; ++j)
				{
					player->myskills.items[index].modifiers[j].type = atoi(row[2]);
					player->myskills.items[index].modifiers[j].index = atoi(row[3]);
					player->myskills.items[index].modifiers[j].value = atoi(row[4]);
					player->myskills.items[index].modifiers[j].set = atoi(row[5]);

					row = mysql_fetch_row(result_b);
					if (!row)
						break;
				}
			}

			mysql_free_result(result_b);
		}

		row = mysql_fetch_row(result);
		if (!row)
			break;
	}

	FREE_RESULT;
	//end runes

	if (player->PlayerID != current->id)
		return false; // Almost there.


	//*****************************
	//standard stats
	//*****************************

	QUERY("SELECT * FROM point_data WHERE char_idx=%d", id);

	GET_RESULT;

	//Exp
	player->myskills.experience =  atoi(row[1]);
	//next_level
	player->myskills.next_level =  atoi(row[2]);
	//Level
	player->myskills.level =  atoi(row[3]);
	//Class number
	player->myskills.class_num = atoi(row[4]);
	//skill points
	player->myskills.speciality_points = atoi(row[5]);
	//credits
	player->myskills.credits = atoi(row[6]);
	//weapon points
	player->myskills.weapon_points = atoi(row[7]);
	//respawn weapon
	player->myskills.respawn_weapon = atoi(row[8]);
	//talent points
	player->myskills.talents.talentPoints = atoi(row[9]);

	FREE_RESULT;

	QUERY("SELECT * FROM character_data WHERE char_idx=%d", id);

	GET_RESULT;


	//*****************************
	//in-game stats
	//*****************************
	//respawns
	player->myskills.respawns = atoi(row[1]);
	//health
	player->myskills.current_health = atoi(row[2]);
	//max health
	player->myskills.max_health = atoi(row[3]);
	//armour
	player->myskills.current_armor = atoi(row[4]);
	//max armour
	player->myskills.max_armor = atoi(row[5]);
	//nerfme			(cursing a player maybe?)
	player->myskills.nerfme = atoi(row[6]);

	//*****************************
	//flags
	//*****************************
	//admin flag
	player->myskills.administrator = atoi(row[7]);
	//boss flag
	player->myskills.boss = atoi(row[8]);


	FREE_RESULT;

	//*****************************
	//stats
	//*****************************

	QUERY( "SELECT * FROM game_stats WHERE char_idx=%d", id );

	GET_RESULT;

	//shots fired
	player->myskills.shots = atoi(row[1]);
	//shots hit
	player->myskills.shots_hit = atoi(row[2]);
	//frags
	player->myskills.frags = atoi(row[3]);
	//deaths
	player->myskills.fragged = atoi(row[4]);
	//number of sprees
	player->myskills.num_sprees = atoi(row[5]);
	//max spree
	player->myskills.max_streak = atoi(row[6]);
	//number of wars
	player->myskills.spree_wars = atoi(row[7]);
	//number of sprees broken
	player->myskills.break_sprees = atoi(row[8]);
	//number of wars broken
	player->myskills.break_spree_wars = atoi(row[9]);
	//suicides
	player->myskills.suicides = atoi(row[10]);
	//teleports			(link this to "use tballself" maybe?)
	player->myskills.teleports = atoi(row[11]);
	//number of 2fers
	player->myskills.num_2fers = atoi(row[12]);

	FREE_RESULT;

	QUERY( "SELECT * FROM ctf_stats WHERE char_idx=%d", id);

	GET_RESULT;

	//CTF statistics
	player->myskills.flag_pickups =  atoi(row[1]);
	player->myskills.flag_captures =  atoi(row[2]);
	player->myskills.flag_returns =  atoi(row[3]);
	player->myskills.flag_kills =  atoi(row[4]);
	player->myskills.offense_kills =  atoi(row[5]);
	player->myskills.defense_kills =  atoi(row[6]);
	player->myskills.assists =  atoi(row[7]);
	//End CTF

	FREE_RESULT;

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
	pthread_mutex_lock(&StatusMutex);
	
	if ( (i = canJoinGame(player)) == 0)
	{
		if (player->PlayerID == current->id)
			player->ThreadStatus = 1; // You can play! :)
	}else
		player->ThreadStatus = i;

	pthread_mutex_unlock(&StatusMutex);
	return true;
}

qboolean V_GDS_StartConn()
{
#ifndef GDS_NOMULTITHREAD
	int rc;
#endif
	gi.dprintf("DB: Initializing connection... ");

	if (!GDS_MySQL)
	{
		GDS_MySQL = mysql_init(NULL);
		if (mysql_real_connect(GDS_MySQL, DEFAULT_DATABASE, MYSQL_USER, MYSQL_PW, MYSQL_DBNAME, 0, NULL, 0) == NULL)
		{
			gi.dprintf("Failure: %s\n", mysql_error(GDS_MySQL));
			mysql_close(GDS_MySQL);
			GDS_MySQL = NULL;
			return false;
		}
	}else
	{
		if (GDS_MySQL)
		{
			gi.dprintf("DB: Already connected\n");
		}
		return false;
	}

	gds_debug = gi.cvar("gds_debug", "0", 0);

	gi.dprintf("Success!\n");
	
#ifndef GDS_NOMULTITHREAD
	
	gi.dprintf ("DB: Creating thread...");

	pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	rc = pthread_create(&QueueThread, &attr, ProcessQueue, NULL);
	
	if (rc)
	{
		gi.dprintf(" Failure creating thread! %d\n", rc);
		mysql_close(GDS_MySQL);
		GDS_MySQL = NULL;
		return false;
	}else
		gi.dprintf(" Done!\n");

	pthread_mutex_init(&QueueMutex, NULL);
	pthread_mutex_init(&StatusMutex, NULL);

#endif

	return true;
}

qboolean CanUseGDS()
{
	return GDS_MySQL != NULL;
}

#ifndef GDS_NOMULTITHREADING

void HandleStatus(edict_t *player)
{
	if (!player)
		return;

	if (!player->inuse)
		return;

	if (savemethod->value != 2)
		return;

	if (!G_IsSpectator(player)) // don't handle plyers that are already logged in!
		return;
		
	if (!GDS_MySQL)
		return;

	pthread_mutex_lock(&StatusMutex);

	if (player->ThreadStatus == 0)
	{
		pthread_mutex_unlock(&StatusMutex);
		return;
	}

	switch (player->ThreadStatus)
	{
	case 3:
		gi.cprintf(player, PRINT_LOW, "Character saved!\n");
		break;
	case 2: // does not exist?
		gi.centerprintf(player, "Creating a new Character!\n");
		newPlayer(player);
		OpenModeMenu(player);
		break;
	case 1:
		gi.centerprintf(player, "Your character was loaded succesfully.");
		OpenModeMenu(player);
		break;
	case 0:
		break;
	default:
		CanJoinGame(player, player->ThreadStatus); //Sends out correct message.
	}
	if (player->ThreadStatus != 1)
		player->ThreadStatus = 0;
	pthread_mutex_unlock(&StatusMutex);
}

void GDS_FinishThread()
{
	void *status;
	int rc;

	if (GDS_MySQL)
	{
		V_GDS_Queue_Add(NULL, GDS_EXITTHREAD);
		rc = pthread_join(QueueThread, &status);
		if (rc)
			gi.dprintf("pthread_join: %d\n", rc);
		rc = pthread_mutex_destroy(&QueueMutex);
		if (rc)
			gi.dprintf("pthread_mutex_destroy: %d\n", rc);
		rc = pthread_mutex_destroy(&StatusMutex);
		if (rc)
			gi.dprintf("pthread_mutex_destroy: %d\n", rc);
		V_GDS_FreeMemory_Queue();
	}
}
#endif

// az end



#endif NO_GDS