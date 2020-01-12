#include "g_local.h"

void ShowVoteMapMenu(edict_t *ent, int pagenum, int mapmode);

//************************************************************************************************
//		**VOTING UTILS**
//************************************************************************************************

#ifdef OLD_VOTE_SYSTEM // Paril
static votes_t votes[MAX_VOTERS];
#else
votes_t currentVote = {false, 0, 0, "", ""};
int numVotes = 0;
float voteTimeLeft = 0;
edict_t *voter;
#endif

//************************************************************************************************

void V_ChangeMap(v_maplist_t *maplist, int mapindex, int gamemode)
{
	//char buf[8];

	//Change the mode
	switch(gamemode)
	{
		case MAPMODE_PVP:
		{
			// player versus player
			gi.cvar_set("ffa", "0");
			gi.cvar_set("domination", "0");
			gi.cvar_set("ctf", "0");
			gi.cvar_set("pvm", "0");
			gi.cvar_set("invasion", "0");
			gi.cvar_set("fraglimit", "100");
			gi.cvar_set("timelimit", "0");
			//gi.cvar_set("dm_monsters", "0");				
		}
		break;
		case MAPMODE_PVM:
		{
			// player versus monsters
			gi.cvar_set("ffa", "0");
			gi.cvar_set("domination", "0");
			gi.cvar_set("ctf", "0");
			gi.cvar_set("pvm", "1");
			gi.cvar_set("invasion", "0");
			gi.cvar_set("fraglimit", "0");
			gi.cvar_set("timelimit", "12");
			//if (dm_monsters->value < 8)
			//	gi.cvar_set("dm_monsters", "8");
			//else
			//	gi.cvar_set("dm_monsters", itoa(total_players(), buf, 10));
		}
		break;
		case MAPMODE_DOM:
		{
			// domination mode
			gi.cvar_set("ffa", "0");
			gi.cvar_set("domination", "1");
			gi.cvar_set("ctf", "0");
			gi.cvar_set("pvm", "0");
			gi.cvar_set("invasion", "0");
			gi.cvar_set("fraglimit", "0");
			gi.cvar_set("timelimit", "12");
			//gi.cvar_set("dm_monsters", "0");
		}
		break;
		case MAPMODE_CTF:
		{
			// ctf mode
			gi.cvar_set("ffa", "0");
			gi.cvar_set("domination", "0");
			gi.cvar_set("ctf", "1");
			gi.cvar_set("pvm", "0");
			gi.cvar_set("invasion", "0");
			gi.cvar_set("fraglimit", "0");
			gi.cvar_set("timelimit", "12");
			//gi.cvar_set("dm_monsters", "0");
		}
		break;
		case MAPMODE_FFA:
		{
			// free for all mode
			gi.cvar_set("ffa", "1");
			gi.cvar_set("domination", "0");
			gi.cvar_set("ctf", "0");
			gi.cvar_set("pvm", "0");
			gi.cvar_set("invasion", "0");
			gi.cvar_set("fraglimit", "100");
			gi.cvar_set("timelimit", "22");
			//if (dm_monsters->value < 4)
			//	gi.cvar_set("dm_monsters", "4");
			//else
			//	gi.cvar_set("dm_monsters", itoa(total_players(), buf, 10));
		}
		break;
		case MAPMODE_INV:
		{
			// invasion mode
			gi.cvar_set("ffa", "0");
			gi.cvar_set("domination", "0");
			gi.cvar_set("ctf", "0");
			gi.cvar_set("pvm", "1");
			gi.cvar_set("invasion", "1");
			gi.cvar_set("fraglimit", "0");
			gi.cvar_set("timelimit", "22");
			//gi.cvar_set("dm_monsters", "4");
		}
		break;
	}

	//Reset player votes
#ifdef OLD_VOTE_SYSTEM // Paril
	memset(votes, 0, sizeof(votes_t) * MAX_VOTERS);
#else
	numVotes = 0;
	voteTimeLeft = 0;
	memset (&currentVote, 0, sizeof(currentVote));
#endif

	gi.dprintf("Votes have been reset.\n");

	//Change the map
	strcpy(level.nextmap, maplist->maps[mapindex].name);
	level.r_monsters = maplist->maps[mapindex].monsters;//4.5
	gi.dprintf("Next map is: %s\n", level.nextmap);
}

//************************************************************************************************

v_maplist_t *GetMapList(int mode)
{
	//Change the map
	switch(mode)
	{
	case MAPMODE_PVP:	return &maplist_PVP;
	case MAPMODE_DOM:	return &maplist_DOM;
	case MAPMODE_PVM:	return &maplist_PVM;
	case MAPMODE_CTF:	return &maplist_CTF;
	case MAPMODE_FFA:	return &maplist_FFA;
	case MAPMODE_INV:	return &maplist_INV;
	default:
		gi.dprintf("ERROR in GetMapList(). Incorrect map mode. (%d)\n", mode);
		return 0;
	}
}

//************************************************************************************************

//Removes ip addresses not on the server at this time
#ifdef OLD_VOTE_SYSTEM // Paril
void CheckPlayerVotes (void)
{
	int i, j;
    edict_t *player;
	qboolean found;

	//Check all the votes
    for(i = 0; i < MAX_VOTERS; ++i)
	{
		if (!votes[i].used)
			continue;

		found = false;

		//Check all the players and look for a matching ip
		for (j = 1; j <= game.maxclients; j++)
		{
			player = &g_edicts[j];

			if (!player->inuse || !player->client)
				continue;
		
			//If ip was found on the server
    		if (Q_stricmp(votes[i].ip, player->client->pers.current_ip) == 0)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			//Delete the vote
			gi.dprintf("Removing vote for: %s [%s]\n", votes[i].name, votes[i].ip);
			memset(&votes[i], 0, sizeof(votes_t));
		}
	}
}
#endif

//************************************************************************************************

#ifdef OLD_VOTE_SYSTEM // Paril
votes_t *FindVote(edict_t *ent)
{
	int i;

	//Try to find the player's vote
	for (i = 0; i < MAX_VOTERS; ++i)
	{
		if (!votes[i].used)
			continue;
		
		if (Q_stricmp(votes[i].ip, ent->client->pers.current_ip) == 0)
		{
			gi.cprintf(ent, PRINT_HIGH, "Your vote has been changed.\n");
			return &votes[i];
		}
		
		if (Q_stricmp(votes[i].name, ent->client->pers.netname) == 0)
		{
			gi.cprintf(ent, PRINT_HIGH, "Your vote has been changed.\n");
			return &votes[i];
		}

	}

	//Can't find a previous vote, so find an empty slot
	for (i = 0; i < MAX_VOTERS; ++i)
	{
		if(votes[i].used)
			continue;
		return &votes[i];
	}

	return NULL;
}
#endif

//************************************************************************************************

#ifdef OLD_VOTE_SYSTEM // Paril
qboolean AddVote(edict_t *ent, int mode, int mapnum)
{
	v_maplist_t *maplist = GetMapList(mode);
	votes_t *myvote;

	if (!maplist) return false;

	//Find the player's vote
	myvote = FindVote(ent);
  
	//check for valid choice
	if (myvote && mode && (maplist->nummaps > mapnum))
	{
		//Add the vote
		strcpy(myvote->ip, ent->client->pers.current_ip);
		strcpy(myvote->name, ent->myskills.player_name);
		myvote->mapindex = mapnum;
		myvote->mode = mode;
		myvote->used = true;

		//Notify everyone
		gi.bprintf(PRINT_HIGH, "%s voted for %s.\n", ent->myskills.player_name, maplist->mapnames[mapnum]);
		switch(mode)
		{
			case MAPMODE_PVP:	gi.bprintf(PRINT_HIGH, "%s voted for Player Vs. Player (PvP).\n", ent->myskills.player_name);	break;
			case MAPMODE_PVM:	gi.bprintf(PRINT_HIGH, "%s voted for Player Vs. Monster (PvM).\n", ent->myskills.player_name);	break;
			case MAPMODE_DOM:	gi.bprintf(PRINT_HIGH, "%s voted for Domination.\n", ent->myskills.player_name);			break;
			case MAPMODE_CTF:	gi.bprintf(PRINT_HIGH, "%s voted for Capture The Flag (CTF).\n", ent->myskills.player_name);					break;
			case MAPMODE_FFA:	gi.bprintf(PRINT_HIGH, "%s voted for Free For All (FFA).\n", ent->myskills.player_name);	break;
			case MAPMODE_INV:	gi.bprintf(PRINT_HIGH, "%s voted for Invasion (INV).\n", ent->myskills.player_name);
		}
		return true;
	}
	else 
	{
		gi.dprintf("Error in AddVote(): map number = %d, mode number = %d, address of myvote = %d\n", mapnum, mode, myvote);
		return false;
	}
}
#else
void AddVote(edict_t *ent, int mode, int mapnum)
{
	v_maplist_t *maplist = GetMapList(mode);

	if (!maplist)
		return;
  
	//check for valid choice
	if (mode && (maplist->nummaps > mapnum))
	{
		char tempBuffer[1024];
		//Add the vote
		voter = ent;
		numVotes = 1;
		ent->client->resp.HasVoted = true;
		strcpy(currentVote.ip, ent->client->pers.current_ip);
		strcpy(currentVote.name, ent->myskills.player_name);
		currentVote.mapindex = mapnum;
		currentVote.mode = mode;
		currentVote.used = true;
		voteTimeLeft = level.time + 90;


		Com_sprintf (tempBuffer, 1024, "%s started a vote for ", ent->myskills.player_name);
		switch(mode)
		{
			case MAPMODE_PVP:	Com_sprintf (tempBuffer, 1024, "%sPlayer vs. Player (PvP) ", tempBuffer);	break;
			case MAPMODE_PVM:	Com_sprintf (tempBuffer, 1024, "%sPlayer vs. Monster (PvM) ", tempBuffer);	break;
			case MAPMODE_DOM:	Com_sprintf (tempBuffer, 1024, "%sDomination ", tempBuffer);			break;
			case MAPMODE_CTF:	Com_sprintf (tempBuffer, 1024, "%sCapture The Flag (CTF) ", tempBuffer);					break;
			case MAPMODE_FFA:	Com_sprintf (tempBuffer, 1024, "%sFree For All (FFA) ", tempBuffer);	break;
			case MAPMODE_INV:	Com_sprintf (tempBuffer, 1024, "%sInvasion ", tempBuffer);
		}
		Com_sprintf (tempBuffer, 1024, "%son %s\n", tempBuffer, maplist->maps[mapnum].name);

		gi.bprintf (PRINT_CHAT, "%s", tempBuffer);
		gi.bprintf (PRINT_HIGH, "Please place your vote by typing 'vote yes' or 'vote no' within the next %d seconds.\n", floattoint(voteTimeLeft-level.time));

	}
	else 
		gi.dprintf("Error in AddVote(): map number = %d, mode number = %d\n", mapnum, mode);
}
#endif

//************************************************************************************************

#ifdef OLD_VOTE_SYSTEM // Paril
int CountTotalModeVotes(int mode)
{
	int count = 0;
	int i;

	for (i = 0; i < MAX_VOTERS; ++i)
		if (votes[i].used && votes[i].mode == mode)
			++count;
	return count;
}
#endif

//************************************************************************************************

#ifdef OLD_VOTE_SYSTEM // Paril
int CountMapVotes(int mapIndex, int mode)
{
	int count = 0;
	int i;
		
	//count the number of votes for this map
	for (i = 0; i < MAX_VOTERS; ++i)
	{
		if ((votes[i].used) && (votes[i].mode == mode) && (votes[i].mapindex == mapIndex))
			++count;
	}
	return count;
}
#endif

#define CHANGE_NOW		1	// change the map/mode immediately
#define CHANGE_LATER	2	// change the map/mode at the completion of vote duration

int V_VoteDone ()
{
	int players = total_players();

	if (players < 1)
		return 0;

	if (numVotes >= 0.75*players)
		return CHANGE_NOW;

	if (numVotes > 0.5*players)
		return CHANGE_LATER;

	return 0;
}

//************************************************************************************************

//Finds the index of the map with the most votes.
//Returns: -1 if there is no winning map (a tie)
#ifdef OLD_VOTE_SYSTEM // Paril
int FindBestMap(int mode)
{
	v_maplist_t *maplist = GetMapList(mode);
	int i;
	int count;
	int bestIndex = -1;
	int bestCount = 0;

	//iterate through each map
	for (i = 0; i < maplist->nummaps; ++i)
	{
		//count the number of votes for this map
		count = CountMapVotes(i, mode);
		
		//Check to see if this map has the most votes
		if (count > bestCount)
		{
			bestCount = count;
			bestIndex = i;
		}
		//Ties don't count
		else if ((count > 0) && (count == bestCount))
		{
			bestIndex = -1;
		}
	}

	//gi.dprintf("bestcount=%d, index=%d\n", bestCount, bestIndex);
	//return map with the most votes
	return bestIndex;
}
#else
int FindBestMap(int mode)
{
	//if (numVotes > 0.5*total_players())
	//	return currentVote.mapindex;
	if (V_VoteDone())
		return currentVote.mapindex;
	return -1;
}
#endif

//************************************************************************************************

int V_AttemptModeChange(qboolean endlevel)
{
#ifdef OLD_VOTE_SYSTEM // Paril
	int pvp, pvm, dom, ctf, ffa, inv, players, total_votes;
	
	players = total_players();

	if (players < 1)
		return 0; // not enough players

	// count the votes for each mode
	pvp = CountTotalModeVotes(MAPMODE_PVP);
	pvm = CountTotalModeVotes(MAPMODE_PVM);
	dom = CountTotalModeVotes(MAPMODE_DOM);
	ctf = CountTotalModeVotes(MAPMODE_CTF);
	ffa = CountTotalModeVotes(MAPMODE_FFA);
	inv = CountTotalModeVotes(MAPMODE_INV);

	total_votes = pvp + pvm + dom + ctf + ffa + inv;

	//gi.dprintf("total_votes=%d\n", total_votes);
	//gi.dprintf("%d,%d,%d,%d,%d\n", pvp,pvm,dom,ctf,ffa);

	if (total_votes < 0.33*players)
		return 0; // need at least 1/3rd of server to change mode

	// change mode immediately if the majority
	// of players are voting
	if (!endlevel && (total_votes < 0.8*players))
		return 0;

	if (pvp && (pvp>pvm) && (pvp>dom) && (pvp>ctf) && (pvp>ffa) && (pvp>inv))
		return MAPMODE_PVP;
	else if (pvm && (pvm>pvp) && (pvm>dom) && (pvm>ctf) && (pvm>ffa) && (pvm>inv))
		return MAPMODE_PVM;
	else if (dom && (dom>pvp) && (dom>pvm) && (dom>ctf) && (dom>ffa) && (dom>inv))
		return MAPMODE_DOM;
	else if (ctf && (ctf>pvp) && (ctf>pvm) && (ctf>dom) && (ctf>ffa) && (ctf>inv))
		return MAPMODE_CTF;
	else if (ffa && (ffa>pvp) && (ffa>pvm) && (ffa>dom) && (ffa>ctf) && (ffa>inv))
		return MAPMODE_FFA;
	else if (inv && (inv>pvp) && (inv>pvm) && (inv>dom) && (inv>ctf) && (inv>ffa))
		return MAPMODE_INV;

	return 0;
#else
	int max_players, players = total_players();

	//4.4 forcibly switch to PvP if we are in Invasion/PvM and there are many players
	max_players = 0.25 * maxclients->value;
	if (max_players < 4)
		max_players = 4;
	if (players >= max_players && (pvm->value || invasion->value) 
		&& (currentVote.mode == MAPMODE_PVM || currentVote.mode == MAPMODE_INV))
	{
		gi.bprintf(PRINT_HIGH, "Forcing switch to FFA because there are too many players!\n");
		return MAPMODE_FFA;
	}

	// did the vote pass?
	if (V_VoteDone())
		return currentVote.mode;

	// vote failed
	return 0;
#endif
}

#ifndef OLD_VOTE_SYSTEM // Paril


void RunVotes(void)
{
	if (voteTimeLeft == -1)
		return;

	if (voteTimeLeft == level.time+60.0f)
		gi.bprintf (PRINT_CHAT, "One minute left to place your vote.\n");
	else if (voteTimeLeft == level.time+30.0f)
		gi.bprintf (PRINT_CHAT, "Thirty seconds left to place your vote.\n");
	else if (voteTimeLeft == level.time+10)
		gi.bprintf (PRINT_CHAT, "Ten seconds left to place your vote.\n");
	else if (voteTimeLeft == level.time)
	{
		// Finish vote
		// Did we reach a majority?
		if (V_VoteDone())
		{
			// Tell everyone
			gi.bprintf (PRINT_CHAT, "A majority was reached!\nVote passed!\n");
			voteTimeLeft = -1;
			//Change the map
			EndDMLevel();
		}
		else
		{
			edict_t *e;
			int i;

			gi.bprintf (PRINT_CHAT, "Vote failed.\n");
			numVotes = 0;
			voteTimeLeft = 0;
			memset (&currentVote, 0, sizeof(currentVote));
			
			for_each_player(e, i)
			{
				e->client->resp.HasVoted = false;
			}

			voter->client->resp.VoteTimeout = level.time + 20;
			voter = NULL;
		}
	}
	else
	{	
		if (V_VoteDone() == CHANGE_NOW)
		{
			// Tell everyone
			gi.bprintf (PRINT_CHAT, "A majority was reached!\nVote passed!\n");
			voteTimeLeft = -1;
			//Change the map
			EndDMLevel();
		}
	}

}
#endif


//************************************************************************************************
//		**VOTE MAP SELECT MENU**
//************************************************************************************************

void ShowVoteMapMenu_handler(edict_t *ent, int option)
{
	if (option == 66666)
	{
		closemenu(ent);
		return;
	}
	//Multi-page navigation
	else if (option > 10000)
	{
		int mode = (option / 1000) - 10;
		int nextpage = option % 1000;		//page number we will end up in (page 0 = main menu)
		if (nextpage != 0)
			ShowVoteMapMenu(ent, nextpage, mode);
		else
			ShowVoteModeMenu(ent);
	}
	else
	{
		int mode = option / 1000;
		int mapnum = option % 1000;
		v_maplist_t *maplist = GetMapList(mode);

		if (!maplist) return;

		//Admins directly control the map change
		if (ent->myskills.administrator)
		{
			VortexEndLevel();
            V_ChangeMap(maplist, mapnum-1, mode);
			ExitLevel();
			return;
		}
		else if (!Q_stricmp(maplist->maps[mapnum-1].name, level.mapname))
		{
			gi.cprintf(ent, PRINT_HIGH, "Can't vote for current map!\n");
			return;
		}
		else
		{
			//Add the player's vote
			AddVote(ent, mode, mapnum-1);
		}
		
		//Are we changing the map?
		//if (V_AttemptModeChange(false))
		//{
			//Change the map
		//	EndDMLevel();
		//}
	}
}

//************************************************************************************************

void ShowVoteMapMenu(edict_t *ent, int pagenum, int mapmode)
{
	int i;
	v_maplist_t *maplist = GetMapList(mapmode);
	qboolean EndOfList = false;
	
	if (!maplist) return;

	//Usual menu stuff
	 if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//Print header
	addlinetomenu(ent, "Vote for map:", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);

	for (i = 10 * (pagenum-1); i < 10 * pagenum; ++i)
	{
		char buf[20];
		if (i >= maplist->nummaps)
		{
			EndOfList = true;
			break;
		}
		strcpy(buf, maplist->maps[i].name);

		padRight(buf, 15);
#ifdef OLD_VOTE_SYSTEM // Paril
		addlinetomenu(ent, va(" %s (%d)", buf, CountMapVotes(i, mapmode)), i+1+(mapmode * 1000));
#else
		addlinetomenu(ent, va(" %s", buf), i+1+(mapmode * 1000));
#endif
	}

	//Menu footer
	addlinetomenu(ent, " ", 0);
#ifdef OLD_VOTE_SYSTEM // Paril
	addlinetomenu(ent, va("    %d votes total.", CountTotalModeVotes(mapmode)), 0);
#endif
	addlinetomenu(ent, " ", 0);

	//Set current line
	if (i % 10 != 0) ent->client->menustorage.currentline = 6 + (i % 10);
	else ent->client->menustorage.currentline = 16;

	if ((!EndOfList) && (i != maplist->nummaps))
	{
		addlinetomenu(ent, " Next", (10000 + pagenum + 1) + (mapmode * 1000)); //ex: pvp, page 3 = 11004
	}
	else
	{
		addlinetomenu(ent, " ", 0);
		++ent->client->menustorage.currentline;
	}
	
	addlinetomenu(ent, " Previous", (10000 + pagenum - 1) + (mapmode * 1000));	//ex: pvm, page 3 = 12002
	addlinetomenu(ent, " Exit", 66666);

	//Set handler
	setmenuhandler(ent, ShowVoteMapMenu_handler);

	//Display the menu
	showmenu(ent);
}

//************************************************************************************************
//		**VOTE MODE SELECT MENU**
//************************************************************************************************

void ShowVoteModeMenu_handler(edict_t *ent, int option)
{
	if (option == 66666)
	{
		closemenu(ent);
		return;
	}

	//Select a mode, and pick a map
	ShowVoteMapMenu(ent, 1, option);	//page = 1, gamemode = option
}

//************************************************************************************************

void KillMyVote (edict_t *ent)
{
	if (ent->client->resp.HasVoted)
	{
		ent->client->resp.HasVoted = false;
		numVotes--;
	}
}

void ShowVoteModeMenu(edict_t *ent)
{
#ifdef OLD_VOTE_SYSTEM // Paril
	int pvp, pvmon, dom, ctf, ffa, total, inv;
	int	players, lastline=8, min_players;

	//Voting enabled?
	if (!voting->value)
		return;
	// don't allow non-admin voting during pre-game to allow players time to connect
	if (!ent->myskills.administrator && (level.time < 30.0)) // allow 30 seconds for players to connect
		return;

	//Usual menu stuff
	 if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	pvp = CountTotalModeVotes(MAPMODE_PVP);
	pvmon = CountTotalModeVotes(MAPMODE_PVM);
	dom = CountTotalModeVotes(MAPMODE_DOM);
	ctf = CountTotalModeVotes(MAPMODE_CTF);
	ffa = CountTotalModeVotes(MAPMODE_FFA);
	inv = CountTotalModeVotes(MAPMODE_INV);

	total = pvp + pvmon + dom + ctf + ffa + inv;
	
	//Print header
	addlinetomenu(ent, "Vote for game mode:", MENU_GREEN_LEFT);
	addlinetomenu(ent, " ", 0);

	addlinetomenu(ent, va(" Deathmatch         (%d)", pvp), MAPMODE_PVP);
	addlinetomenu(ent, va(" Free For All       (%d)", ffa), MAPMODE_FFA);
//GHz START
	players = total_players();
	// pvm and invasion are only available when there are few players on the server
	if (0.33 * maxclients->value < 4)
		min_players = 4;
	else
		min_players = 0.25 * maxclients->value;
	if (/*pvm->value ||*/ (players < min_players))
	{
		addlinetomenu(ent, va(" Player Vs. Monster (%d)", pvmon), MAPMODE_PVM);
		lastline++;
	}
	// invasion mode
	if (/*invasion->value ||*/ (players < min_players))
	{
		addlinetomenu(ent, va(" Invasion           (%d)", inv), MAPMODE_INV);
		lastline++;
	}
	
	// domination available when there are at least 4 players
	if (players >= 4)
	{
		addlinetomenu(ent, va(" Domination         (%d)", dom), MAPMODE_DOM);
		lastline++;
	}
	// CTF available when there are at least 4 players
	if (players >= 4)
	{
		addlinetomenu(ent, va(" CTF                (%d)", ctf), MAPMODE_CTF);
		lastline++;
	}
//GHz END
	//Menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, va("    %d votes total.", total), 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " Exit", 66666);

	//Set handler
	setmenuhandler(ent, ShowVoteModeMenu_handler);

	//Set current line
	ent->client->menustorage.currentline = lastline;

	//Display the menu
	showmenu(ent);
#else
	int	players, lastline=6, min_players;

	char *cmd2 = gi.argv(1);

	if (voteTimeLeft == -1)
		return;

	//Voting enabled?
	if (!voting->value)
		return;
	// don't allow non-admin voting during pre-game to allow players time to connect
	if (!ent->myskills.administrator && (level.time < 30.0)) // allow 30 seconds for players to connect
	{
		gi.cprintf(ent, PRINT_HIGH, "Please allow time for other players to connect.\n");
		return;
	}
	if (ent->client->resp.VoteTimeout > level.time)
		return;

	if (cmd2[0] == 'y' || cmd2[0] == 'Y')
	{
		if (!currentVote.mode)
			gi.cprintf(ent, PRINT_HIGH, "There is no vote taking place at this time.\n");
		else
		{
			// Did we already vote?
			if (ent->client->resp.HasVoted)
			{
				gi.cprintf(ent, PRINT_HIGH, "You already placed your vote.\n");
				return; // GTFO PLZ
			}
			ent->client->resp.HasVoted = true;
			numVotes++;

			gi.bprintf (PRINT_CHAT, "%s voted Yes.\n", ent->client->pers.netname);
		}
		return;
	}
	else if (cmd2[0] == 'n' || cmd2[0] == 'N')
	{
		if (!currentVote.mode)
			gi.cprintf(ent, PRINT_HIGH, "There is no vote taking place at this time.\n");
		else
		{
			// Did we already vote?
			if (ent->client->resp.HasVoted)
			{
				gi.cprintf(ent, PRINT_HIGH, "You already placed your vote.\n");
				return; // GTFO PLZ
			}
			ent->client->resp.HasVoted = true;
			gi.bprintf (PRINT_CHAT, "%s voted No.\n", ent->client->pers.netname);
		}
		return;
	}
	// Bring open the menu
	else if (currentVote.mode)
	{
		// TO DO
		return;
	}

	//Usual menu stuff
	 if (!ShowMenu(ent))
        return;
	clearmenu(ent);
	
	//Print header
	addlinetomenu(ent, "Vote for game mode:", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);

	addlinetomenu(ent, " Player vs. Player", MAPMODE_PVP);
	addlinetomenu(ent, " Free For All", MAPMODE_FFA);
//GHz START
	players = total_players();
	// pvm and invasion are only available when there are few players on the server

	if (0.33 * maxclients->value < 4)
		min_players = 4;
	else
		min_players = 0.25 * maxclients->value;
	/*4.5
	if (players < min_players)
	{
		addlinetomenu(ent, " Player vs. Monster", MAPMODE_PVM);
		lastline++;
	}*/
	// invasion mode
	if (players < min_players)
	{
		addlinetomenu(ent, " Invasion", MAPMODE_INV);
		lastline++;
	}
	
	// domination available when there are at least 4 players
	if (players >= 4)
	{
		addlinetomenu(ent, " Domination", MAPMODE_DOM);
		lastline++;
	}
	// CTF available when there are at least 4 players
	if (players >= 4)
	{
		addlinetomenu(ent, " CTF", MAPMODE_CTF);
		lastline++;
	}
//GHz END
	//Menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " Exit", 66666);

	//Set handler
	setmenuhandler(ent, ShowVoteModeMenu_handler);

	//Set current line
	ent->client->menustorage.currentline = lastline;

	//Display the menu
	showmenu(ent);
#endif
}
