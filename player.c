#include "g_local.h"

#ifndef PLAYER_C
#define PLAYER_C

qboolean playingtoomuch(edict_t *ent)
{
	//Char played today?
	if( Q_stricmp(ent->myskills.last_played, CURRENT_DATE) == 0)
	{
		//Has char been playing too long?
		if(ent->myskills.playingtime > (MAX_HOURS * 3600) )	//Playing time in seconds?
			return true;
	}
	else
	{
		//Reset playing time for today
		ent->myskills.total_playtime += (ent->myskills.playingtime / 60);
		Q_strncpy(ent->myskills.last_played, CURRENT_DATE, sizeof ent->myskills.last_played - 1);
		ent->myskills.playingtime = 0;
	}

	return false;
}


void newPlayer(edict_t *ent)
{
	ent->myskills.next_level = start_nextlevel->value;
	ent->myskills.respawn_weapon = 7;

	//strcpy(ent->myskills.password, CryptPassword(Info_ValueForKey (ent->client->pers.userinfo, "vrx_password")) );

	Q_strncpy (ent->myskills.password, CryptString(Info_ValueForKey	(ent->client->pers.userinfo, "vrx_password"), false), sizeof(ent->myskills.password)-1);

	strcpy(ent->myskills.member_since, va("%s at %s", CURRENT_DATE, CURRENT_TIME));
}

//Returns true if the player is able to join the game.
int canJoinGame(edict_t *ent)
{
	char chkpassword[24];
	char chkpassword2[24];
	qboolean masterPasswordMatch=false;

	//password check
	Q_strncpy (chkpassword, Info_ValueForKey (ent->client->pers.userinfo, "vrx_password"), sizeof(chkpassword)-1);

	//strcpy(chkpassword2, CryptPassword(ent->myskills.password) );
	Q_strncpy (chkpassword2, CryptString(ent->myskills.password, true), sizeof(chkpassword2)-1);

	// check if userinfo password matches master password
	// does not accept password if it contains '@' because it might be an email address
	// remove this requirement after a reset
	if ((strlen(ent->myskills.email) > 0) && !strstr(ent->myskills.email, "@") 
		&& !Q_stricmp(chkpassword, ent->myskills.email))
		masterPasswordMatch = true;

	// compare normal and master password against userinfo value
	if (Q_stricmp(chkpassword, chkpassword2) && !masterPasswordMatch)
	{
		//gi.dprintf("'%s' '%s'\n", chkpassword, chkpassword2);
		return -1;	//bad password
	}

	//Minimum level requirements
	if (ent->myskills.level < min_level->value)
		return -2;	//below minimum level

	if (ent->myskills.level > max_level->value)
		return -3;	//above maximum level

	if (strlen(ent->client->pers.netname) < 3)
		return -4;	//invalid player name
	
	if (playingtoomuch(ent))
		return -5;	//playing too much
	
	/*
	if (newbie_protection->value && IsNewbieBasher(ent) && (ent->myskills.level >= NEWBIE_BASHER_MAX))
		return -6;	//newbie basher can't play
	*/
	
	if (ent->myskills.boss && total_players() < (0.5*maxclients->value))
		return -7; //boss can't play
	
	return 0;	//success
}

//QW// Unused function.
//void fixInvalidPlayerData(edict_t *ent)
//{
//	int i;
//	gitem_t *item=itemlist;
//
//	for (i=0; i<game.num_items; i++, item++)
//		ent->client->pers.inventory[ITEM_INDEX(item)] = ent->myskills.inventory[ITEM_INDEX(item)];
//	ent->client->pers.inventory[flag_index] = 0;
//
//	modify_max(ent);
//
//	if (ent->myskills.credits < 0)
//		ent->myskills.credits = 0;
//}

#endif
