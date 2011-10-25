#include "g_local.h"
#include "bot.h"

//K03 Begin
#define CTF_TEAM1SKIN ctf_team1_skin->string
#define CTF_TEAM2SKIN ctf_team2_skin->string

#define	SKIN_ID_TEAM1	"female/lotus"
#define	SKIN_ID_TEAM2	"male/claymore"

#define	CTF_TEAM1_SKIN		(zoidctf->value == 1 ? CTF_TEAM1SKIN : SKIN_ID_TEAM1 )
#define	CTF_TEAM2_SKIN		(zoidctf->value == 1 ? CTF_TEAM2SKIN : SKIN_ID_TEAM2 )
/*
#define CTF_CAPTURE_BASE		ctf_capture_base->value	// base value for calculating award for capturing team (def: 100)
#define CTF_CAPTURE_BONUS		ctf_capture_bonus->value	// multiplier for EXTRA bonus for the capturer (def: 2)
#define CTF_TEAM_BONUS			ctf_team_bonus->value	// multiplier for EXTRA bonus for the cap'ing team members (def: 1)
#define CTF_RECOVERY_BONUS		ctf_recovery_bonus->value	// extra bonus for the recoverer (def: 0.5)
#define CTF_FLAG_BONUS			ctf_flag_bonus->value	// what you get for picking up enemy flag
#define CTF_FRAG_CARRIER_BONUS	ctf_frag_carrier_bonus->value	// extra bonus for fragging enemy flag carrier (def: 0.5)
#define CTF_FRAG_MODIFIER		ctf_frag_modifier->value	// extra bonus for fragging enemy flag carrier (def: 0.5)
#define CTF_MIN_CAPTEAM			ctf_min_capteam->value		// min players on both teams to award experience at cap
#define CTF_FORCE_EVENTEAMS		ctf_force_eventeams->value	// set to 1 to force teams to be even by level
#define CTF_CARRIER_DANGER_PROTECT_BONUS	ctf_carrier_danger_protect_bonus->value	// bonus for fraggin someone who has recently hurt your flag carrier
#define CTF_CARRIER_PROTECT_BONUS			ctf_carrier_protect_bonus->value	// bonus for fraggin someone while either you or your target are near your flag carrier
#define CTF_FLAG_DEFENSE_BONUS				ctf_flag_defense_bonus->value	// bonus for fraggin someone while either you or your target are near your flag
#define CTF_RETURN_FLAG_ASSIST_BONUS		ctf_return_flag_assist_bonus->value	// awarded for returning a flag that causes a capture to happen almost immediately
#define CTF_FRAG_CARRIER_ASSIST_BONUS		ctf_frag_carrier_assist_bonus->value	// award for fragging a flag carrier if a capture happens almost immediately

#define CTF_TARGET_PROTECT_RADIUS			ctf_target_protect_radius->value	// the radius around an object being defended where a target will be worth extra frags
#define CTF_ATTACKER_PROTECT_RADIUS			ctf_attacker_protect_radius->value	// the radius around an object being defended where an attacker will get extra frags when making kills

#define CTF_CARRIER_DANGER_PROTECT_TIMEOUT	ctf_carrier_danger_protect_timeout->value
#define CTF_FRAG_CARRIER_ASSIST_TIMEOUT		ctf_frag_carrier_assist_timeout->value
#define CTF_RETURN_FLAG_ASSIST_TIMEOUT		ctf_return_flag_assist_timeout->value

#define CTF_AUTO_FLAG_RETURN_TIMEOUT		ctf_auto_flag_return_timeout->value	// number of seconds before dropped flag auto-returns

#define CTF_TECH_TIMEOUT					tech_timeout->value  // seconds before techs spawn again
*/
//K03 End

typedef struct ctfgame_s
{
	int team1, team2;
	int total1, total2; // these are only set when going into intermission!
	float last_flag_capture;
	int last_capture_team;
} ctfgame_t;
//PON
qboolean bots_moveok ( edict_t *ent,float ryaw,vec3_t pos,float dist,float *bottom);
//PON
ctfgame_t ctfgame;
qboolean techspawn = false;

cvar_t *ctf;
cvar_t *ctf_forcejoin;

char *ctf_statusbar =
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
"	xv	90 "//K03 was 100
"	anum "
"	xv	140 "//K03 was 150
"	pic 2 "
"endif "

// armor
"if 4 "
"	xv	180 "//K03 was 200
"	rnum "
"	xv	230 "//K03 was 250
"	pic 4 "
"endif "

// selected item
"if 6 "
//K03 Begin
"   xv  246 "
"   num 3 27 "
//K03 End
"	xv	296 "
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer
"if 9 "
//K03 Begin
//"	xv	246 "
" xv 221 "
"	num	3 10 "//was " num 2 10 "
//K03 End
"	xv	296 "
"	pic	9 "
"endif "

//  help / weapon icon 
"if 11 "
"	xv	148 "
"	pic	11 "
"endif "
//K03 Begin
//  frags
"xr -34 "
"yt 2 "
"string \"Game\" "

"xr	-50 "
"yt 11 "
"num 3 14 "
//K03 End
// spectator
"if 29 "
  "xv 0 "
  "yb -58 "
  "string2 \"SPECTATOR MODE\" "
"endif "

// chase camera
"if 16 "
  "xv 0 "
  "yb -68 "
  "string \"Chasing\" "
  "xv 64 "
  "stat_string 16 "
"endif "

//K03 Begin rank order system
//  rank
"xr -34 "
"yt 42 "
"string \"Rank\" "

"xr	-50 "
"yt 53 "
"num 3 24 "

//Rank stuff done
//Show the Streak
//GHz: Begin Power Cube stuff
"xr -42 "
"yt 112 "
"string \"Cubes\" "

"xr -50 "
"yt 120 "
"num 3 28 "
//GHz: End Power Cube stuff
//start time left.
/*"xr -35 "
"yt 112 "
"string \"Time\" "

"xr -50 "
"yt 120 "
"num 3 28 "*/
//print out our current level
/*"xr -25 "
"yt 158 "//was 141
"string \"Lvl\" "

"xr -50 "
"yt 167 "//150
"num 3 26 "*/
//K03 End

// red team
"yb -102 "
//K03 Begin
"xr -60 "
"num 2 22 "
//K03 End
"if 17 "
  "xr -26 "
  "pic 17 "
"endif "
//joined overlay
"if 20 "
  "yb -104 "
  "xr -28 "
  "pic 20 "
"endif "

// blue team
"yb -75 "
//K03 Begin
"xr -60 "
"num 2 23 "
//K03 End
"if 18 "
  "xr -26 "
  "pic 18 "
"endif "
"if 21 "
  "yb -77 "
  "xr -28 "
  "pic 21 "
"endif "

// have flag graph
"if 19 "
  "yt 26 "
  "xr -24 "
  "pic 19 "
"endif "
//GHz START
// health-id
"if 30 "
"   xv 0 "
"   yb  -157 "
"   string2 \"HLT-ID\" "

"   xv 0 "
"   yb -148  "
"   num 4 30 "
"endif "

// armor-id
"if 26 "
"   xv 0 "
"   yb  -124 "
"   string2 \"AMR-ID\" "
"endif "

"if 26 "
"   xv 0 "
"   yb -115 "
"   num 4 26 "
"endif "

// ammo-id
"if 31 "
"   xv 0 "
"   yb  -91 "
"   string2 \"AMO-ID\" "
"endif "

"if 31 "
"   xv 0 "
"   yb -82 "
"   num 4 31 "
"endif "
//GHz END
;
/*
char *ctf_statusbar =
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

// armor
"if 4 "
"	xv	200 "
"	rnum "
"	xv	250 "
"	pic 4 "
"endif "

// selected item
"if 6 "
"	xv	296 "
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer
"if 9 "
  "xv 246 "
  "num 2 10 "
  "xv 296 "
  "pic 9 "
"endif "

//  help / weapon icon 
"if 11 "
  "xv 148 "
  "pic 11 "
"endif "

//  frags
"xr	-50 "
"yt 2 "
"num 3 14 "

// red team
"yb -102 "
"if 17 "
  "xr -26 "
  "pic 17 "
"endif "
//joined overlay
"if 20 "
  "yb -104 "
  "xr -28 "
  "pic 20 "
"endif "

// blue team
"yb -75 "
"if 18 "
  "xr -26 "
  "pic 18 "
"endif "
"if 21 "
  "yb -77 "
  "xr -28 "
  "pic 21 "
"endif "

// have flag graph
"if 19 "
  "yt 26 "
  "xr -24 "
  "pic 19 "
"endif "
;*/


static char *tnames[] = {
	"item_tech1", "item_tech2", "item_tech3", "item_tech4",
	NULL
};

void stuffcmd(edict_t *ent, char *s) 	
{
   	gi.WriteByte (11);	        
	gi.WriteString (s);
    gi.unicast (ent, true);	
}

/*--------------------------------------------------------------------------*/

/*
=================
findradius

Returns entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
static edict_t *loc_findradius (edict_t *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		j;

	if (!from)
		from = g_edicts;
	else
		from++;
	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;
#if 0
		if (from->solid == SOLID_NOT)
			continue;
#endif
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);
		if (VectorLength(eorg) > rad)
			continue;
		return from;
	}

	return NULL;
}

static void loc_buildboxpoints(vec3_t p[8], vec3_t org, vec3_t mins, vec3_t maxs)
{
	VectorAdd(org, mins, p[0]);
	VectorCopy(p[0], p[1]);
	p[1][0] -= mins[0];
	VectorCopy(p[0], p[2]);
	p[2][1] -= mins[1];
	VectorCopy(p[0], p[3]);
	p[3][0] -= mins[0];
	p[3][1] -= mins[1];
	VectorAdd(org, maxs, p[4]);
	VectorCopy(p[4], p[5]);
	p[5][0] -= maxs[0];
	VectorCopy(p[0], p[6]);
	p[6][1] -= maxs[1];
	VectorCopy(p[0], p[7]);
	p[7][0] -= maxs[0];
	p[7][1] -= maxs[1];
}

qboolean loc_CanSee (edict_t *targ, edict_t *inflictor)
{
	trace_t	trace;
	vec3_t	targpoints[8];
	int i;
	vec3_t viewpoint;

// bmodels need special checking because their origin is 0,0,0
	if (targ->movetype == MOVETYPE_PUSH)
		return false; // bmodels not supported

	loc_buildboxpoints(targpoints, targ->s.origin, targ->mins, targ->maxs);
	
	VectorCopy(inflictor->s.origin, viewpoint);
	viewpoint[2] += inflictor->viewheight;

	for (i = 0; i < 8; i++) {
		trace = gi.trace (viewpoint, vec3_origin, vec3_origin, targpoints[i], inflictor, MASK_SOLID);
		if (trace.fraction == 1.0)
			return true;
	}

	return false;
}

/*--------------------------------------------------------------------------*/

static gitem_t *flag1_item;
static gitem_t *flag2_item;

void CTFInit(void)
{
	ctf = gi.cvar("ctf", "0", CVAR_SERVERINFO | CVAR_LATCH);
	ctf_forcejoin = gi.cvar("ctf_forcejoin", "", 0);

	if (!flag1_item)
		flag1_item = FindItemByClassname("item_flag_team1");
	if (!flag2_item)
		flag2_item = FindItemByClassname("item_flag_team2");

//	if (zoidctf->value)
//	{
		flag1_item->world_model = "players/male/flag1.md2";
		flag1_item->icon = "i_ctf1";
		flag1_item->pickup_sound = "ctf/flagtk.wav";

		flag2_item->world_model = "players/male/flag2.md2";
		flag2_item->icon = "i_ctf2";
		flag2_item->pickup_sound = "ctf/flagtk.wav";
//	}

	memset(&ctfgame, 0, sizeof(ctfgame));
	techspawn = false;
}

//K03 Begin
// points the lasers where they have to go
void flag_update_lasers(edict_t *l) {
	vec3_t	dest;
	trace_t	tr;

	if (!l->owner) {
		G_FreeEdict (l);
		return;
	}

	if (l->owner->solid != SOLID_NOT) {
		// flag at base
		VectorCopy(l->owner->s.origin, l->s.old_origin);
	} else {
		// flag elsewhere
		VectorCopy(l->s.origin, dest);
		dest[2] += 2048;
		tr = gi.trace(l->s.origin, NULL, NULL, dest, l, MASK_SOLID);
		VectorCopy(tr.endpos, l->s.old_origin);
	}
	gi.linkentity(l);

	l->nextthink = level.time + FRAMETIME;
}

// links the lasers into the world
void flag_lasers_link(edict_t *l) {
	vec3_t	dest;
	trace_t	tr;

	VectorCopy(l->s.origin, dest);
	dest[2] -= 128;
	tr = gi.trace(l->s.origin, NULL, NULL, dest, l, MASK_SOLID);
	VectorCopy(tr.endpos, l->s.origin);

	gi.linkentity(l);
	
	l->think = flag_update_lasers;
	l->nextthink = level.time + FRAMETIME;
}


// spawns the 4 lasers for a flag
void flag_lasers_spawn(edict_t *flag) {
	static	vec3_t	offset[4] = {{16,16,0}, {-16,16,0}, {-16,-16,0}, {16,-16,0}};
	edict_t	*l[4];		// could eliminate need for this to be an array
	int		i;

	for (i=0; i<4; i++) {
		l[i] = G_Spawn();
//		l[i]->flags &= FL_TEAMSLAVE;
		l[i]->owner = flag;
		
		// set up individual positions
		VectorAdd(flag->s.origin, offset[i], l[i]->s.origin);

//		l[i]->noblock = true;
		
		l[i]->movetype = MOVETYPE_NONE;
		l[i]->solid = SOLID_NOT;
		l[i]->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
		l[i]->s.modelindex = 1;			// must be non-zero

		// set the width
		l[i]->s.frame = 4;

		// set the color
		if (flag->item == flag1_item)
			l[i]->s.skinnum = 0xf2f2f0f0;	// red
		else
			l[i]->s.skinnum = 0xf3f3f1f1;	// blue

		VectorSet (l[i]->mins, -8, -8, -8);
		VectorSet (l[i]->maxs, 8, 8, 8);
	
		l[i]->think = flag_lasers_link;
		l[i]->nextthink = level.time + 2 * FRAMETIME;
	}
}
//K03 End

/*--------------------------------------------------------------------------*/

char *CTFTeamName(int team)
{
	switch (team) {
	case CTF_TEAM1:
		return "RED";
	case CTF_TEAM2:
		return "BLUE";
	}
	return "UKNOWN";
}

char *CTFOtherTeamName(int team)
{
	switch (team) {
	case CTF_TEAM1:
		return "BLUE";
	case CTF_TEAM2:
		return "RED";
	}
	return "UKNOWN";
}

int CTFOtherTeam(int team)
{
	switch (team) {
	case CTF_TEAM1:
		return CTF_TEAM2;
	case CTF_TEAM2:
		return CTF_TEAM1;
	}
	return -1; // invalid value
}

/*--------------------------------------------------------------------------*/

edict_t *SelectRandomDeathmatchSpawnPoint (void);
edict_t *SelectFarthestDeathmatchSpawnPoint (void);
float	PlayersRangeFromSpot (edict_t *spot);

void CTFAssignSkin(edict_t *ent, char *s)
{
	int playernum = ent-g_edicts-1;
	char *p;
	char t[64];

	Com_sprintf(t, sizeof(t), "%s", s);

	if ((p = strrchr(t, '/')) != NULL)
		p[1] = 0;
	else
		strcpy(t, "male/");

	switch (ent->client->resp.ctf_team) {
	case CTF_TEAM1:
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", 
			ent->client->pers.netname, CTF_TEAM1_SKIN) );
		break;
	case CTF_TEAM2:
		gi.configstring (CS_PLAYERSKINS+playernum,
			va("%s\\%s", ent->client->pers.netname, CTF_TEAM2_SKIN) );
		break;
	default:
		gi.configstring (CS_PLAYERSKINS+playernum, 
			va("%s\\%s", ent->client->pers.netname, s) );
		break;
	}
//	gi.cprintf(ent, PRINT_HIGH, "You have been assigned to %s team.\n", ent->client->pers.netname);
}

void CTFAssignTeam(gclient_t *who)
{
	edict_t		*player;
	int i;
	int team1count = 0, team2count = 0;

	who->resp.ctf_state = CTF_STATE_START;

	if (!((int)dmflags->value & DF_CTF_FORCEJOIN)) {
		who->resp.ctf_team = CTF_NOTEAM;
		return;
	}

	for (i = 1; i <= maxclients->value; i++) {
		player = &g_edicts[i];

		if (!player->inuse || player->client == who)
			continue;

		switch (player->client->resp.ctf_team) {
		case CTF_TEAM1:
			team1count++;
			break;
		case CTF_TEAM2:
			team2count++;
		}
	}
	if (team1count < team1count)
		who->resp.ctf_team = CTF_TEAM1;
	else if (team2count < team1count)
		who->resp.ctf_team = CTF_TEAM2;
	else if (rand() & 1)
		who->resp.ctf_team = CTF_TEAM1;
	else
		who->resp.ctf_team = CTF_TEAM2;
}

/*
================
SelectCTFSpawnPoint

go to a ctf point, but NOT the two points closest
to other players
================
*/
edict_t *SelectCTFSpawnPoint (edict_t *ent)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;
	char	*cname;

//	gi.dprintf("SelectCTFSpawnPoint()\n");

	if (ent->client->resp.ctf_state != CTF_STATE_START)
		if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
			return SelectFarthestDeathmatchSpawnPoint ();
		else
			return SelectRandomDeathmatchSpawnPoint ();

	ent->client->resp.ctf_state = CTF_STATE_PLAYING;

	switch (ent->client->resp.ctf_team) {
	case CTF_TEAM1:
		cname = "info_player_team1";
		break;
	case CTF_TEAM2:
		cname = "info_player_team2";
		break;
	default:
		return SelectRandomDeathmatchSpawnPoint();
	}

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), cname)) != NULL)
	{
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
		return SelectRandomDeathmatchSpawnPoint();

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
		spot = G_Find (spot, FOFS(classname), cname);
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

/*------------------------------------------------------------------------*/
/*
Chamooze:
CTFFragExpModify

This is here to calculate the total modification of a frag in ctf, also
calculates the bonuses for flag defense, flag carrier defense, etc.
Note that bonuses are not cumaltive.  You get one, they are in importance
order.
Observe that it returns the CHANGE in exp gained/lost, not the actual value.
*/
int CTFFragExpModify(edict_t *targ, edict_t *attacker, int beginexp)
{
	float resultexp, value;
	int i;
	edict_t *ent;
	gitem_t *flag_item, *enemy_flag_item;
	int otherteam;
	edict_t *flag, *carrier;
	char *c;
	vec3_t v1, v2;

	// no bonus for fragging yourself
	if (!targ->client || !attacker->client || targ == attacker)
		return 0;

	if (CTF_FRAG_MODIFIER == 0)
		return (-beginexp);

	if (!found_flag)
		value = 2.0 * CTF_FRAG_MODIFIER;
	else
		value = CTF_FRAG_MODIFIER;

	resultexp = ((float) beginexp) * value;

	otherteam = CTFOtherTeam(targ->client->resp.ctf_team);
	if (otherteam < 0)
		return (resultexp - beginexp); // whoever died isn't on a team

	// same team, if the flag at base, check to he has the enemy flag
	if (targ->client->resp.ctf_team == CTF_TEAM1) {
		flag_item = flag1_item;
		enemy_flag_item = flag2_item;
	} else {
		flag_item = flag2_item;
		enemy_flag_item = flag1_item;
	}

	// did the attacker frag the flag carrier?
	if (targ->client->pers.inventory[ITEM_INDEX(enemy_flag_item)]) {
		attacker->client->resp.ctf_lastfraggedcarrier = level.time;
		resultexp *= CTF_FRAG_CARRIER_BONUS;

		// the target had the flag, clear the hurt carrier
		// field on the other team
		for (i = 1; i <= maxclients->value; i++) {
			ent = g_edicts + i;
			if (ent->inuse && ent->client->resp.ctf_team == otherteam)
				ent->client->resp.ctf_lasthurtcarrier = 0;
		}
		return (resultexp - beginexp);
	}

	if (targ->client->resp.ctf_lasthurtcarrier &&
		level.time - targ->client->resp.ctf_lasthurtcarrier < CTF_CARRIER_DANGER_PROTECT_TIMEOUT &&
		!attacker->client->pers.inventory[ITEM_INDEX(flag_item)]) {
		// attacker is on the same team as the flag carrier and
		// fragged a guy who hurt our flag carrier
		resultexp *= CTF_CARRIER_DANGER_PROTECT_BONUS;

/*		attacker->client->resp.score += CTF_CARRIER_DANGER_PROTECT_BONUS;
		attacker->myskills.experience += CTF_CARRIER_DANGER_PROTECT_BONUS;

		if (attacker->myskills.experience > attacker->myskills.max_experience)
			attacker->myskills.max_experience = attacker->myskills.experience;
		gi.bprintf(PRINT_MEDIUM, "%s defends %s's flag carrier against an agressive enemy\n",
			attacker->client->pers.netname, 
			CTFTeamName(attacker->client->resp.ctf_team));*/
		return (resultexp - beginexp);
	}

	// flag and flag carrier area defense bonuses

	// we have to find the flag and carrier entities

	// find the flag
	switch (attacker->client->resp.ctf_team) {
	case CTF_TEAM1:
		c = "item_flag_team1";
		break;
	case CTF_TEAM2:
		c = "item_flag_team2";
		break;
	default:
		return (resultexp - beginexp);
	}

	flag = NULL;
	while ((flag = G_Find (flag, FOFS(classname), c)) != NULL) {
		if (!(flag->spawnflags & DROPPED_ITEM))
			break;
	}

	if (!flag)
		return (resultexp - beginexp); // can't find attacker's flag

	// find attacker's team's flag carrier
	for (i = 1; i <= maxclients->value; i++) {
		carrier = g_edicts + i;
		if (carrier->inuse && 
			carrier->client->pers.inventory[ITEM_INDEX(flag_item)])
			break;
		carrier = NULL;
	}

	// ok we have the attackers flag and a pointer to the carrier

	// check to see if we are defending the base's flag
	VectorSubtract(targ->s.origin, flag->s.origin, v1);
	VectorSubtract(attacker->s.origin, flag->s.origin, v2);

	if (VectorLength(v1) < CTF_TARGET_PROTECT_RADIUS ||
		VectorLength(v2) < CTF_TARGET_PROTECT_RADIUS ||
		loc_CanSee(flag, targ) || loc_CanSee(flag, attacker)) {
		// we defended the base flag
		resultexp *= CTF_FLAG_DEFENSE_BONUS;
/*		attacker->client->resp.score += CTF_FLAG_DEFENSE_BONUS;
		attacker->myskills.experience += CTF_FLAG_DEFENSE_BONUS;
		if (attacker->myskills.experience > attacker->myskills.max_experience)
			attacker->myskills.max_experience = attacker->myskills.experience;*/
		if (flag->solid == SOLID_NOT)
			gi.bprintf(PRINT_MEDIUM, "%s defends the %s base.\n",
				attacker->client->pers.netname, 
				CTFTeamName(attacker->client->resp.ctf_team));
		else
			gi.bprintf(PRINT_MEDIUM, "%s defends the %s flag.\n",
				attacker->client->pers.netname, 
				CTFTeamName(attacker->client->resp.ctf_team));
		return (resultexp - beginexp);
	}

	if (carrier && carrier != attacker) {
		VectorSubtract(targ->s.origin, carrier->s.origin, v1);
		VectorSubtract(attacker->s.origin, carrier->s.origin, v1);

		if (VectorLength(v1) < CTF_ATTACKER_PROTECT_RADIUS ||
			VectorLength(v2) < CTF_ATTACKER_PROTECT_RADIUS ||
			loc_CanSee(carrier, targ) || loc_CanSee(carrier, attacker)) {

			resultexp *= CTF_CARRIER_PROTECT_BONUS;
/*			attacker->client->resp.score += CTF_CARRIER_PROTECT_BONUS;
			attacker->myskills.experience += CTF_CARRIER_PROTECT_BONUS;
			if (attacker->myskills.experience > attacker->myskills.max_experience)
				attacker->myskills.max_experience = attacker->myskills.experience;*/
			gi.bprintf(PRINT_MEDIUM, "%s defends the %s's flag carrier.\n",
				attacker->client->pers.netname, 
				CTFTeamName(attacker->client->resp.ctf_team));
			return (resultexp - beginexp);
		}
	}
	return (resultexp - beginexp);
}

void check_for_levelup(edict_t *);

/*
	Add_ctfteam_exp loops through all players on the same team as ent and adds
	experience to them according to points and ctf_teammate_fragaward
*/
void Add_ctfteam_exp(edict_t *ent, int points)
{
	int		i, addexp = (int)(CTF_TEAMMATE_FRAGAWARD * points);
	float	level_diff;
	edict_t *player;

	for (i = 1; i <= maxclients->value; i++) {
		player = &g_edicts[i];
		if (!player->inuse)
			continue;
		if (player == ent)
			continue;
		if (player->solid == SOLID_NOT)
			continue;
		if (player->health <= 0)
			continue;
		// players must help the team in order to get shared points!
		if ((!pvm->value && (player->lastkill + 30 < level.time)) 
			|| (player->lastkill+60 < level.time))
			continue;

		if (OnSameTeam(ent, player)) 
		{
			level_diff = (float) (1 + player->myskills.level) / (float) (1 + ent->myskills.level);
			if (level_diff > 1.5 || level_diff < 0.66)
				continue;
			player->client->resp.score += addexp;
			player->myskills.experience += addexp;
			
			check_for_levelup(player);
		}
	}
}

void CTFCheckHurtCarrier(edict_t *targ, edict_t *attacker)
{
	gitem_t *flag_item;

	if (!targ->client || !attacker->client)
		return;

	if (targ->client->resp.ctf_team == CTF_TEAM1)
		flag_item = flag2_item;
	else
		flag_item = flag1_item;

	if (targ->client->pers.inventory[ITEM_INDEX(flag_item)] &&
		targ->client->resp.ctf_team != attacker->client->resp.ctf_team)
		attacker->client->resp.ctf_lasthurtcarrier = level.time;
}


/*------------------------------------------------------------------------*/

void CTFResetFlag(int ctf_team)
{
	char *c;
	edict_t *ent;

	switch (ctf_team) {
	case CTF_TEAM1:
		c = "item_flag_team1";
		break;
	case CTF_TEAM2:
		c = "item_flag_team2";
		break;
	default:
		return;
	}

	ent = NULL;
	while ((ent = G_Find (ent, FOFS(classname), c)) != NULL) {
		if (ent->spawnflags & DROPPED_ITEM)
			G_FreeEdict(ent);
		else {
			ent->svflags &= ~SVF_NOCLIENT;
			ent->solid = SOLID_TRIGGER;
			gi.linkentity(ent);
			ent->s.event = EV_ITEM_RESPAWN;
		}
	}
}

void CTFResetFlags(void)
{
	CTFResetFlag(CTF_TEAM1);
	CTFResetFlag(CTF_TEAM2);
}

// GHz START

float CTFPlayerValue (edict_t *player) 
{
	float	value;

	value = player->myskills.level*pow(1.02, player->myskills.level);
	if (value < 1)
		value = 1;
	return value;
}

/*
=============
CTFTeamValue

Returns a weighted value for teamnum team, based on player levels
=============
*/
float CTFTeamValue (int teamnum)
{
	int		i;
	float	weight=0;
	edict_t	*player;
	
	for (i=1; i<=maxclients->value;i++) 
	{
		player = &g_edicts[i];
		if (!player->inuse)
			continue;
		if (player->solid == SOLID_NOT)
			continue;
		if (player->client->resp.ctf_team == teamnum) 
			weight += CTFPlayerValue(player);
	}
	if (weight<1)
		weight=1;
	return weight;
}

edict_t *GetHighestLevelPlayer (void)
{
	int		i;
	edict_t *player, *found=NULL;

	for (i=1 ;i<=maxclients->value; i++) 
	{
		player = &g_edicts[i];
		if (!G_EntExists(player))
			continue;
		// dont pick players that are already assigned to a team
		if (player->client->resp.ctf_team != CTF_NOTEAM)
			continue;
		if (!found || (player->myskills.level>found->myskills.level))
			found = player;
	}
	return found;
}

void ResetCTFState (void)
{
	int		i;
	edict_t	*player;

	for (i=1 ;i<=maxclients->value; i++) 
	{
		player = &g_edicts[i];
		if (!G_EntExists(player))
			continue;
		if ((player->health<0) || (player->deadflag==DEAD_DEAD))
			respawn(player); // force respawn
		player->client->resp.ctf_team = CTF_NOTEAM; // reset their team
	}
}

void FindCTFSpawnPoint (edict_t *player)
{
	int		i;
	edict_t *scan;

	scan = SelectCTFSpawnPoint(player); // find spawnpoint
	// teleport effects
	player->s.event = EV_PLAYER_TELEPORT;
	scan->s.event = EV_PLAYER_TELEPORT;
	gi.unlinkentity (player); // dont interfere with killbox
	// move to spawnpoint origin
	VectorCopy (scan->s.origin, player->s.origin);
	VectorCopy (scan->s.origin, player->s.old_origin);
	player->s.origin[2] += 10;
	// clear the velocity and hold them in place briefly
	VectorClear (player->velocity);
	player->client->ps.pmove.pm_time = 160>>3; // hold time
	player->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;
	// set angles
	for (i=0 ; i<3 ; i++)
		player->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(scan->s.angles[i] - player->client->resp.cmd_angles[i]);
	VectorClear(player->s.angles);
	VectorClear(player->client->ps.viewangles);
	VectorClear(player->client->v_angle);
	KillBox (player); // kill anything at the destination
	gi.linkentity (player);
}


qboolean TeleportNearPoint (edict_t *self, vec3_t point);
void CTFSetTeam (edict_t *player, int team)
{
	char	*s;
	edict_t	*scan=NULL;

	// reset player state
	VortexRemovePlayerSummonables(player);
	player->client->respawn_time = level.time + 2.0;
	player->client->resp.ctf_team = team;
	player->client->resp.ctf_state = CTF_STATE_START;
	s = Info_ValueForKey (player->client->pers.userinfo, "skin");
	CTFAssignSkin(player, s);
	CTFResetFlags();

	if (team == CTF_TEAM1)
	{
		// teleport near the red flag
		while((scan = G_Find(scan, FOFS(classname), "item_flag_team1")) != NULL) {
			if (scan && scan->inuse)
			{
				if (!TeleportNearPoint(player, scan->s.origin))
					FindCTFSpawnPoint(player);
				break;
			}
		}
	}
	else
	{
		// teleport near the blue flag
		while((scan = G_Find(scan, FOFS(classname), "item_flag_team2")) != NULL) {
			if (scan && scan->inuse)
			{
				if (!TeleportNearPoint(player, scan->s.origin))
					FindCTFSpawnPoint(player);
				break;
			}
		}
	}
}

qboolean SomeoneHasFlag (void)
{
	int		i;
	edict_t *player;

	for (i=1 ; i<=maxclients->value; i++) 
	{
		player = &g_edicts[i];
		if (!G_EntExists(player))
			continue;
		if (player->client->pers.inventory[ITEM_INDEX(FindItem("Blue Flag"))] > 0 
			|| player->client->pers.inventory[ITEM_INDEX(FindItem("Red Flag"))] > 0)
		return true;
	}
	return false;
}

void CTFSortTeams (void)
{
	edict_t *player=NULL;

	if (SomeoneHasFlag())
		return;

	gi.bprintf(PRINT_HIGH, "Teams are being automatically sorted.\n");
	ResetCTFState();
	while ((player = GetHighestLevelPlayer()) != NULL)
	{
		if (CTFTeamValue(CTF_TEAM1)>CTFTeamValue(CTF_TEAM2))
			CTFSetTeam(player, CTF_TEAM2);
		else
			CTFSetTeam(player, CTF_TEAM1);
	}
}			

// Chamooze
void check_for_levelup(edict_t *ent);



void CTFTeamExpForCap(edict_t *capturer)
{
	edict_t *player;
	int team, otherteam, ownlevelsum = 0, onownteam = 0, otherlevelsum = 0, onotherteam = 0;
	int i;
	float value, teamaward;
	switch (capturer->client->resp.ctf_team) {
	case CTF_TEAM1:
		team = CTF_TEAM1;
		otherteam = CTF_TEAM2;
		break;
	case CTF_TEAM2:
		team = CTF_TEAM2;
		otherteam = CTF_TEAM1;
		break;
	}

	for (i = 1; i <= maxclients->value; i++) {
		player = &g_edicts[i];
		if (!player->inuse)
			continue;
		if (player->solid == SOLID_NOT)
			continue;
		
		if (player->client->resp.ctf_team == team) {
			ownlevelsum += (int)CTFPlayerValue(player);//(player->myskills.level + 1);
			onownteam++;
		}
		else if (player->client->resp.ctf_team == otherteam) {
			otherlevelsum += (int)CTFPlayerValue(player);//(player->myskills.level + 1);
			onotherteam++;
		}
	}

//	gi.bprintf(PRINT_HIGH, "owncount: %d ownlevel: %d, othercount: %d otherlevel %d\n", onownteam, ownlevelsum, onotherteam, otherlevelsum);

	if ((onownteam < CTF_MIN_CAPTEAM) || (onotherteam < CTF_MIN_CAPTEAM)) {
		gi.bprintf(PRINT_HIGH, "No capture awarded, not enough players!\n");
		return;
	}

	if (level.time < 120){
		gi.bprintf(PRINT_HIGH, "No capture awarded during pre-game!\n");
		return;
	}

	value = (float)otherlevelsum/ownlevelsum;
	if ((value > 1.2) || (value < 0.8))
	{
		teamaward = 0;
		gi.bprintf(PRINT_HIGH, "No capture awarded, teams are not even!\n");
	}
	else
		teamaward = value*CTF_CAPTURE_BASE;
	//teamaward = ((float) otherlevelsum / ownlevelsum) * CTF_CAPTURE_BASE;

	for (i = 1; i <= maxclients->value; i++) {
		player = &g_edicts[i];
		if (!player->inuse)
			continue;

		if (player->client->resp.ctf_team == capturer->client->resp.ctf_team) {
			player->client->resp.score += CTF_TEAM_BONUS * teamaward;
			player->myskills.experience += CTF_TEAM_BONUS * teamaward;

			if (player->client->resp.ctf_lastreturnedflag + CTF_RETURN_FLAG_ASSIST_TIMEOUT > level.time) {
				gi.bprintf(PRINT_HIGH, "%s gets an assist for returning the flag!\n", player->client->pers.netname);
				player->client->resp.score += CTF_RETURN_FLAG_ASSIST_BONUS * teamaward;
				player->myskills.experience += CTF_RETURN_FLAG_ASSIST_BONUS * teamaward;
			}

			if (player->client->resp.ctf_lastfraggedcarrier + CTF_FRAG_CARRIER_ASSIST_TIMEOUT > level.time) {
				gi.bprintf(PRINT_HIGH, "%s gets an assist for fragging the flag carrier!\n", player->client->pers.netname);
				player->client->resp.score += CTF_FRAG_CARRIER_ASSIST_BONUS * teamaward;
				player->myskills.experience += CTF_FRAG_CARRIER_ASSIST_BONUS * teamaward;
			}
			check_for_levelup(player);
		}
	}

	capturer->client->resp.score += CTF_CAPTURE_BONUS * teamaward;
	capturer->myskills.experience += CTF_CAPTURE_BONUS * teamaward;
	check_for_levelup(capturer);
}

qboolean CTFPickup_Flag(edict_t *ent, edict_t *other)
{
	int ctf_team;
//	int i;
//	edict_t *player;
	gitem_t *flag_item, *enemy_flag_item;

	if (level.time < 130)
		return false;
	if (other->client->invincible_framenum > level.framenum)
		return false;
	
	if(chedit->value) {SetRespawn (ent, 30); return true;};


	// figure out what team this flag is
	if (strcmp(ent->classname, "item_flag_team1") == 0)
		ctf_team = CTF_TEAM1;
	else if (strcmp(ent->classname, "item_flag_team2") == 0)
		ctf_team = CTF_TEAM2;
	else {
		gi.cprintf(ent, PRINT_HIGH, "Don't know what team the flag is on.\n");
		return false;
	}

	// same team, if the flag at base, check to he has the enemy flag
	if (ctf_team == CTF_TEAM1) {
		flag_item = flag1_item;
		enemy_flag_item = flag2_item;
	} else {
		flag_item = flag2_item;
		enemy_flag_item = flag1_item;
	}

	if (ctf_team == other->client->resp.ctf_team) {

		if (!(ent->spawnflags & DROPPED_ITEM)) {
			// the flag is at home base.  if the player has the enemy
			// flag, he's just won!
		
			if (other->client->pers.inventory[ITEM_INDEX(enemy_flag_item)]) {
				gi.bprintf(PRINT_HIGH, "%s captured the %s flag!\n",
						other->client->pers.netname, CTFOtherTeamName(ctf_team));
				other->client->pers.inventory[ITEM_INDEX(enemy_flag_item)] = 0;

				ctfgame.last_flag_capture = level.time;
				ctfgame.last_capture_team = ctf_team;
				if (ctf_team == CTF_TEAM1)
					ctfgame.team1++;
				else
					ctfgame.team2++;

				gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/flagcap.wav"), 1, ATTN_NONE, 0);

				// Chamooze put all awards in a separate function:
				CTFTeamExpForCap(other);


				CTFResetFlags();
				return false;
			}
			return false; // its at home base already
		}	
		// hey, its not home.  return it by teleporting it back
		gi.bprintf(PRINT_HIGH, "%s returned the %s flag!\n", 
			other->client->pers.netname, CTFTeamName(ctf_team));
		other->client->resp.score += CTF_RECOVERY_BONUS;
		other->myskills.experience += CTF_RECOVERY_BONUS;
		other->client->resp.ctf_lastreturnedflag = level.time;
		gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/flagret.wav"), 1, ATTN_NONE, 0);
		//CTFResetFlag will remove this entity!  We must return false
		CTFResetFlag(ctf_team);
		return false;
	}

	// hey, its not our flag, pick it up
	gi.bprintf(PRINT_HIGH, "%s got the %s flag!\n",
		other->client->pers.netname, CTFTeamName(ctf_team));

	// disable speed abilities for flag carrier
	hook_reset(other->client->hook);
	other->superspeed = false;
	other->client->thrusting = 0;
	VectorSet(other->velocity, 0, 0, 0);

	// disable morphing on flag pickup
	if (other->s.modelindex != 255)
	{
		other->mtype = 0;
		other->s.modelindex = 255;
		other->s.skinnum = other-g_edicts-1;
		ShowGun(other);
	}

	//other->client->disconnect_time = level.time + CTF_FLAG_HOLDTIME; //GHz: Max time someone can hold flag
	other->client->resp.score += CTF_FLAG_BONUS;
	other->myskills.experience += CTF_FLAG_BONUS;

	//Getting flag makes you uncloak
	other->svflags &= ~SVF_NOCLIENT;
	other->client->cloaking = false;
	other->client->cloakable = 0;

	other->client->pers.inventory[ITEM_INDEX(flag_item)] = 1;
	other->client->resp.ctf_flagsince = level.time;

	// pick up the flag
	// if it's not a dropped flag, we just make is disappear
	// if it's dropped, it will be removed by the pickup caller
	if (!(ent->spawnflags & DROPPED_ITEM)) {
		ent->flags |= FL_RESPAWN;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
	}
	return true;
}

static void CTFDropFlagTouch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//owner (who dropped us) can't touch for two secs
	if (other == ent->owner && 
		ent->nextthink - level.time > CTF_AUTO_FLAG_RETURN_TIMEOUT-2)
		return;

	Touch_Item (ent, other, plane, surf);
}

static void CTFDropFlagThink(edict_t *ent)
{
	// auto return the flag
	// reset flag will remove ourselves
	if (strcmp(ent->classname, "item_flag_team1") == 0) {
		CTFResetFlag(CTF_TEAM1);
		gi.bprintf(PRINT_HIGH, "The %s flag has returned!\n",
			CTFTeamName(CTF_TEAM1));
	} else if (strcmp(ent->classname, "item_flag_team2") == 0) {
		CTFResetFlag(CTF_TEAM2);
		gi.bprintf(PRINT_HIGH, "The %s flag has returned!\n",
			CTFTeamName(CTF_TEAM2));
	}
}

// Called from PlayerDie, to drop the flag from a dying player
void CTFDeadDropFlag(edict_t *self)
{
	edict_t *dropped = NULL;

	if (!ctf->value)
		return;
	if (!flag1_item || !flag2_item)
		CTFInit();

	if (self->client->pers.inventory[ITEM_INDEX(flag1_item)]) {
		dropped = Drop_Item(self, flag1_item);
		self->client->pers.inventory[ITEM_INDEX(flag1_item)] = 0;
		gi.bprintf(PRINT_HIGH, "%s lost the %s flag!\n",
			self->client->pers.netname, CTFTeamName(CTF_TEAM1));
	} else if (self->client->pers.inventory[ITEM_INDEX(flag2_item)]) {
		dropped = Drop_Item(self, flag2_item);
		self->client->pers.inventory[ITEM_INDEX(flag2_item)] = 0;
		gi.bprintf(PRINT_HIGH, "%s lost the %s flag!\n",
			self->client->pers.netname, CTFTeamName(CTF_TEAM2));
	}

	if (dropped) {
		dropped->think = CTFDropFlagThink;
		dropped->nextthink = level.time + CTF_AUTO_FLAG_RETURN_TIMEOUT;
		dropped->touch = CTFDropFlagTouch;
		//K03 Begin
		dropped->s.effects |= EF_COLOR_SHELL;
		if (dropped->item == flag1_item)
			dropped->s.renderfx |= RF_SHELL_RED;
		else
			dropped->s.renderfx |= RF_SHELL_BLUE;
	
	//K03 End
	}
}

qboolean CTFDrop_Flag(edict_t *ent, gitem_t *item)
{
	if (rand() & 1) 
	{
		gi.cprintf(ent, PRINT_HIGH, "Only lusers drop flags.\n");
	}
	else
	{
		gi.cprintf(ent, PRINT_HIGH, "Winners don't drop flags.\n");
	}
	return false;
}

static void CTFFlagThink(edict_t *ent)
{
	if (ent->solid != SOLID_NOT && zoidctf->value)
		ent->s.frame = 173 + (((ent->s.frame - 173) + 1) % 16);
	else
		ent->s.frame = 0;
	ent->nextthink = level.time + FRAMETIME;
}

void droptofloor (edict_t *ent);
void SpawnItem3 (edict_t *ent, gitem_t *item);
//edict_t *GetBotFlag1();	//チーム1の旗
//edict_t *GetBotFlag2();  //チーム2の旗 
void ChainPodThink (edict_t *ent);
qboolean ChkTFlg( void );//旗セットアップ済み？
void SetBotFlag1(edict_t *ent);	//チーム1の旗
void SetBotFlag2(edict_t *ent);  //チーム2の旗

void CTFFlagSetup (edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;
	float		*v;

	if (ctf->value < 1)
		return;

	v = tv(-15,-15,-15);
	VectorCopy (v, ent->mins);
	v = tv(15,15,15);
	VectorCopy (v, ent->maxs);

	if (ent->model)
		gi.setmodel (ent, ent->model);
	else if(ent->item->world_model)		//PONKO
		gi.setmodel (ent, ent->item->world_model);
	else ent->s.modelindex = 0;			//PONKO
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;

	v = tv(0,0,-128);
	VectorAdd (ent->s.origin, v, dest);

	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
	if (tr.startsolid)
	{
		gi.dprintf ("CTFFlagSetup: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEdict (ent);
		return;
	}

	VectorCopy (tr.endpos, ent->s.origin);

	gi.linkentity (ent);

	//K03 Begin
	flag_lasers_spawn(ent);
	ent->s.effects |= EF_COLOR_SHELL;
	if (ent->item == flag1_item)
		ent->s.renderfx |= RF_SHELL_RED;
	else
		ent->s.renderfx |= RF_SHELL_BLUE;
	
	//K03 End

	ent->nextthink = level.time + FRAMETIME;
	ent->think = CTFFlagThink;

////PON
//	if (Q_stricmp(ent->classname, "item_flag_team1") == 0) SetBotFlag1(ent);
//	else if (Q_stricmp(ent->classname, "item_flag_team2") == 0) SetBotFlag2(ent);
////PON
	return;
}

void CTFEffects(edict_t *player)
{
	player->s.effects &= ~(EF_FLAG1 | EF_FLAG2);
	if (player->health > 0) {
		if (player->client->pers.inventory[ITEM_INDEX(flag1_item)]) {
			player->s.effects |= EF_FLAG1;
		}
		if (player->client->pers.inventory[ITEM_INDEX(flag2_item)]) {
			player->s.effects |= EF_FLAG2;
		}
	}

	//K03 Begin
	if (player->client->pers.inventory[ITEM_INDEX(flag1_item)])
		player->s.modelindex3 = gi.modelindex(flag1_item->world_model);
	else if (player->client->pers.inventory[ITEM_INDEX(flag2_item)])
		player->s.modelindex3 = gi.modelindex(flag2_item->world_model);
	else
		player->s.modelindex3 = 0;
	//K03 End
}

// called when we enter the intermission
void CTFCalcScores(void)
{
	int i;

	ctfgame.total1 = ctfgame.total2 = 0;
	for (i = 0; i < maxclients->value; i++) {
		if (!g_edicts[i+1].inuse)
			continue;
		if (game.clients[i].resp.ctf_team == CTF_TEAM1)
			ctfgame.total1 += game.clients[i].resp.score;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
			ctfgame.total2 += game.clients[i].resp.score;
	}
}

int CTFGetCaps(int team)
{
	if (team == CTF_TEAM1)
		return ctfgame.team1;
	else
		return ctfgame.team2;
}

int CTFGetSumLevels(int team)
{
	int i, sum = 0;

	for (i = 0; i < maxclients->value; i++) {
		if (!g_edicts[i+1].inuse)
			continue;
		if (game.clients[i].resp.ctf_team == team)
			sum += CTFPlayerValue(&g_edicts[i+1]);//.myskills.level;
	}

	return sum;
}

int CTFGetSumScores(int team)
{
	int i, sum = 0;

	for (i = 0; i < maxclients->value; i++) {
		if (!g_edicts[i+1].inuse)
			continue;
		if (game.clients[i].resp.ctf_team == team)
			sum += game.clients[i].resp.score;
	}

	return sum;
}

void CTFID_f (edict_t *ent)
{
	if (ent->client->resp.id_state) {
		gi.cprintf(ent, PRINT_HIGH, "Disabling player identication display.\n");
		ent->client->resp.id_state = false;
	} else {
		gi.cprintf(ent, PRINT_HIGH, "Activating player identication display.\n");
		ent->client->resp.id_state = true;
	}
}

static void CTFSetIDView(edict_t *ent)
{
	vec3_t	forward, dir;
	trace_t	tr;
	edict_t	*who, *best;
	float	bd = 0, d;
	int i;

	//ent->client->ps.stats[STAT_CTF_ID_VIEW] = 0;

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, 1024, forward);
	VectorAdd(ent->s.origin, forward, forward);
	tr = gi.trace(ent->s.origin, NULL, NULL, forward, ent, MASK_SOLID);
	if (tr.fraction < 1 && tr.ent && tr.ent->client) {
	//	ent->client->ps.stats[STAT_CTF_ID_VIEW] = 
	//		CS_PLAYERSKINS + (ent - g_edicts - 1);
		return;
	}

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	best = NULL;
	for (i = 1; i <= maxclients->value; i++) {
		who = g_edicts + i;
		if (!who->inuse)
			continue;
		VectorSubtract(who->s.origin, ent->s.origin, dir);
		VectorNormalize(dir);
		d = DotProduct(forward, dir);
		if (d > bd && loc_CanSee(ent, who)) {
			bd = d;
			best = who;
		}
	}
	//if (bd > 0.90)
//		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 
//			CS_PLAYERSKINS + (best - g_edicts - 1);
}

void SetCTFStats(edict_t *ent)
{
	gitem_t *tech;
	int i;
	int p1, p2;
	edict_t *e;
	
	//K03 Begin
	// logo headers for the frag display
	//K03 ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = gi.imageindex ("ctfsb1");
	//K03 ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = gi.imageindex ("ctfsb2");
	
	ent->client->ps.stats[STAT_CTF_TEAM1_CAPTURES] = ctfgame.team1;
	ent->client->ps.stats[STAT_CTF_TEAM2_CAPTURES] = ctfgame.team2;
	//K03 End
/*
	// if during intermission, we must blink the team header of the winning team
	if (level.intermissiontime && (level.framenum & 8)) { // blink 1/8th second
		// note that ctfgame.total[12] is set when we go to intermission
		if (ctfgame.team1 > ctfgame.team2)
			ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
		else if (ctfgame.team2 > ctfgame.team1)
			ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
		else if (ctfgame.total1 > ctfgame.total2) // frag tie breaker
			ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
		else if (ctfgame.total2 > ctfgame.total1) 
			ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
		else { // tie game!
			ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
			ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
		}
	}*/

	// tech icon
	i = 0;
//	ent->client->ps.stats[STAT_CTF_TECH] = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			ent->client->pers.inventory[ITEM_INDEX(tech)]) {
		//	ent->client->ps.stats[STAT_CTF_TECH] = gi.imageindex(tech->icon);
			break;
		}
		i++;
	}

	// figure out what icon to display for team logos
	// three states:
	//   flag at base
	//   flag taken
	//   flag dropped
	p1 = (zoidctf->value) ? gi.imageindex ("i_ctf1") : gi.imageindex ("k_redkey");//K03
	e = G_Find(NULL, FOFS(classname), "item_flag_team1");
	if (e != NULL) {
		if (e->solid == SOLID_NOT) {
			int i;

			// not at base
			// check if on player
			p1 = (zoidctf->value) ? gi.imageindex ("i_ctf1d") : gi.imageindex ("k_redkey"); // K03 default to dropped
			for (i = 1; i <= maxclients->value; i++)
				if (g_edicts[i].inuse &&
					g_edicts[i].client->pers.inventory[ITEM_INDEX(flag1_item)]) {
					// enemy has it
					p1 = (zoidctf->value) ? gi.imageindex ("i_ctf1t") : gi.imageindex ("k_redkey");
					break;
				}
		} else if (e->spawnflags & DROPPED_ITEM)
			p1 = (zoidctf->value) ? gi.imageindex ("i_ctf1d") : gi.imageindex ("k_redkey"); // K03 must be dropped
	}
	p2 = (zoidctf->value) ? gi.imageindex ("i_ctf2") : gi.imageindex ("k_bluekey");//K03
	e = G_Find(NULL, FOFS(classname), "item_flag_team2");
	if (e != NULL) {
		if (e->solid == SOLID_NOT) {
			int i;

			// not at base
			// check if on player
			p2 = (zoidctf->value) ? gi.imageindex ("i_ctf2d") : gi.imageindex ("k_bluekey"); // K03 default to dropped
			for (i = 1; i <= maxclients->value; i++)
				if (g_edicts[i].inuse &&
					g_edicts[i].client->pers.inventory[ITEM_INDEX(flag2_item)]) {
					// enemy has it
					p2 = (zoidctf->value) ? gi.imageindex ("i_ctf2t") : gi.imageindex ("k_bluekey");
					break;
				}
		} else if (e->spawnflags & DROPPED_ITEM)
			p2 = (zoidctf->value) ? gi.imageindex ("i_ctf2d") : gi.imageindex ("k_bluekey"); // K03 must be dropped
	}


	ent->client->ps.stats[STAT_CTF_TEAM1_PIC] = p1;
	ent->client->ps.stats[STAT_CTF_TEAM2_PIC] = p2;

	if (ctfgame.last_flag_capture && level.time - ctfgame.last_flag_capture < 5) {
		if (ctfgame.last_capture_team == CTF_TEAM1)
			if (level.framenum & 8)
				ent->client->ps.stats[STAT_CTF_TEAM1_PIC] = p1;
			else
				ent->client->ps.stats[STAT_CTF_TEAM1_PIC] = 0;
		else
			if (level.framenum & 8)
				ent->client->ps.stats[STAT_CTF_TEAM2_PIC] = p2;
			else
				ent->client->ps.stats[STAT_CTF_TEAM2_PIC] = 0;
	}

//	ent->client->ps.stats[STAT_CTF_TEAM1_CAPS] = ctfgame.team1;
//	ent->client->ps.stats[STAT_CTF_TEAM2_CAPS] = ctfgame.team2;

	ent->client->ps.stats[STAT_CTF_FLAG_PIC] = 0;
	if (ent->client->resp.ctf_team == CTF_TEAM1 &&
		ent->client->pers.inventory[ITEM_INDEX(flag2_item)] &&
		(level.framenum & 8))
		ent->client->ps.stats[STAT_CTF_FLAG_PIC] = (zoidctf->value) ? gi.imageindex ("i_ctf2") : gi.imageindex ("k_bluekey");//K03

	else if (ent->client->resp.ctf_team == CTF_TEAM2 &&
		ent->client->pers.inventory[ITEM_INDEX(flag1_item)] &&
		(level.framenum & 8))
		ent->client->ps.stats[STAT_CTF_FLAG_PIC] = (zoidctf->value) ? gi.imageindex ("i_ctf1") : gi.imageindex ("k_redkey");//K03

	ent->client->ps.stats[STAT_CTF_JOINED_TEAM1_PIC] = 0;
	ent->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = 0;
	if (ent->client->resp.ctf_team == CTF_TEAM1)
		ent->client->ps.stats[STAT_CTF_JOINED_TEAM1_PIC] = (zoidctf->value) ? gi.imageindex ("i_ctfj") : 0;//K03
	else if (ent->client->resp.ctf_team == CTF_TEAM2)
		ent->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = (zoidctf->value) ? gi.imageindex ("i_ctfj") : 0;//K03

//	ent->client->ps.stats[STAT_CTF_ID_VIEW] = 0;
//	if ((ent->client->resp.id_state) && (ent->myskills.id_upgrade > 0))
//		CTFSetIDView(ent);
}

/*------------------------------------------------------------------------*/

/*QUAKED info_player_team1 (1 0 0) (-16 -16 -24) (16 16 32)
potential team1 spawning position for ctf games
*/
void SP_info_player_team1(edict_t *self)
{
}

/*QUAKED info_player_team2 (0 0 1) (-16 -16 -24) (16 16 32)
potential team2 spawning position for ctf games
*/
void SP_info_player_team2(edict_t *self)
{
}


/*------------------------------------------------------------------------*/
/* GRAPPLE																  */
/*------------------------------------------------------------------------*/

// ent is player
void CTFPlayerResetGrapple(edict_t *ent)
{
	if (ent->client && ent->client->ctf_grapple)
		CTFResetGrapple(ent->client->ctf_grapple);
	ent->s.sound = 0;
}

// self is grapple, not player
void CTFResetGrapple(edict_t *self)
{
	self->s.sound = 0;

	if(self->owner == NULL)
	{
		G_FreeEdict(self);
		return;		
	}
	
	if (self->owner->client->ctf_grapple) {
		float volume = 1.0;
		gclient_t *cl;

		if (self->owner->client->silencer_shots)
			volume = 0.2;

		gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grreset.wav"), volume, ATTN_NORM, 0);
		cl = self->owner->client;
		cl->ctf_grapple = NULL;
		cl->ctf_grapplereleasetime = level.time;
		cl->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
		cl->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
		G_FreeEdict(self);
	}
}

void CTFGrappleTouch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	float volume = 1.0;

	if (other == self->owner)
		return;

	if (self->owner->client->ctf_grapplestate != CTF_GRAPPLE_STATE_FLY)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		CTFResetGrapple(self);
		return;
	}

	VectorCopy(vec3_origin, self->velocity);

	PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage) {
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, 0, MOD_GRAPPLE);
		CTFResetGrapple(self);
		return;
	}

	self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_PULL; // we're on hook
	self->enemy = other;
	self->solid = SOLID_NOT;

	if (self->owner->client->silencer_shots)
		volume = 0.2;

	gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grpull.wav"), volume, ATTN_NORM, 0);
	gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhit.wav"), volume, ATTN_NORM, 0);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPARKS);
	gi.WritePosition (self->s.origin);
	if (!plane)
		gi.WriteDir (vec3_origin);
	else
		gi.WriteDir (plane->normal);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

// draw beam between grapple and self
void CTFGrappleDrawCable(edict_t *self)
{
	vec3_t	offset, start, end, f, r;
	vec3_t	dir;
	float	distance;
	float	x;

	if(1/*!(self->owner->svflags & SVF_MONSTER)*/)
	{
		AngleVectors (self->owner->client->v_angle, f, r, NULL);
		VectorSet(offset, 16, 16, self->owner->viewheight-8);
		P_ProjectSource (self->owner->client, self->owner->s.origin, offset, f, r, start);
	}
	else
	{
		x = self->owner->s.angles[YAW] ;
		x = x * M_PI * 2 / 360;
		start[0] = self->owner->s.origin[0] + cos(x) * 16;
		start[1] = self->owner->s.origin[1] + sin(x) * 16;
		if(self->owner->maxs[2] >=32) start[2] = self->owner->s.origin[2]+16;
		else start[2] = self->owner->s.origin[2]-12;	
	}

	VectorSubtract(start, self->owner->s.origin, offset);

	VectorSubtract (start, self->s.origin, dir);
	distance = VectorLength(dir);
	// don't draw cable if close
	if (distance < 64)
		return;

#if 0
	if (distance > 256)
		return;

	// check for min/max pitch
	vectoangles (dir, angles);
	if (angles[0] < -180)
		angles[0] += 360;
	if (fabs(angles[0]) > 45)
		return;

	trace_t	tr; //!!

	tr = gi.trace (start, NULL, NULL, self->s.origin, self, MASK_SHOT);
	if (tr.ent != self) {
		CTFResetGrapple(self);
		return;
	}
#endif

	// adjust start for beam origin being in middle of a segment
//	VectorMA (start, 8, f, start);

	VectorCopy (self->s.origin, end);
	// adjust end z for end spot since the monster is currently dead
//	end[2] = self->absmin[2] + self->size[2] / 2;

	gi.WriteByte (svc_temp_entity);
#if 1 //def USE_GRAPPLE_CABLE
	gi.WriteByte (TE_GRAPPLE_CABLE);
	gi.WriteShort (self->owner - g_edicts);
	gi.WritePosition (self->owner->s.origin);
	gi.WritePosition (end);
	gi.WritePosition (offset);
#else
	gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (end);
	gi.WritePosition (start);
#endif
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

void SV_AddGravity (edict_t *ent);

// pull the player toward the grapple
void CTFGrapplePull(edict_t *self)
{
	vec3_t hookdir, v;
	float vlen;

	if(self->owner == NULL)
	{
		CTFResetGrapple(self);
		return;		
	}

	if (self->enemy) {
		if (self->enemy->solid == SOLID_NOT) {
			CTFResetGrapple(self);
			return;
		}
		if (self->enemy->solid == SOLID_BBOX) {
			VectorScale(self->enemy->size, 0.5, v);
			VectorAdd(v, self->enemy->s.origin, v);
			VectorAdd(v, self->enemy->mins, self->s.origin);
			gi.linkentity (self);
		} else
			VectorCopy(self->enemy->velocity, self->velocity);
		if (self->enemy->takedamage &&
			!CheckTeamDamage (self->enemy, self->owner)) {
			float volume = 1.0;

			if (self->owner->client->silencer_shots)
				volume = 0.2;

			T_Damage (self->enemy, self, self->owner, self->velocity, self->s.origin, vec3_origin, 1, 1, 0, MOD_GRAPPLE);
			gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhurt.wav"), volume, ATTN_NORM, 0);
		}
		if (self->enemy->deadflag) { // he died
			CTFResetGrapple(self);
			return;
		}
	}

	CTFGrappleDrawCable(self);

	if (self->owner->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY) {
		// pull player toward grapple
		// this causes icky stuff with prediction, we need to extend
		// the prediction layer to include two new fields in the player
		// move stuff: a point and a velocity.  The client should add
		// that velociy in the direction of the point
		vec3_t forward, up;

		if(1/*!(self->owner->svflags & SVF_MONSTER)*/)
		{
			AngleVectors (self->owner->client->v_angle, forward, NULL, up);
			VectorCopy(self->owner->s.origin, v);
			v[2] += self->owner->viewheight;
			VectorSubtract (self->s.origin, v, hookdir);
		}
		else
		{
			VectorCopy(self->owner->s.origin, v);
			if(self->owner->maxs[2] >=32) v[2] += 16;
			else v[2] -= 12;
			VectorSubtract (self->s.origin, v, hookdir);
		}

		vlen = VectorLength(hookdir);

		if (self->owner->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL &&
			vlen < 64) {
			float volume = 1.0;

			if (self->owner->client->silencer_shots)
				volume = 0.2;

			self->owner->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
			gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grhang.wav"), volume, ATTN_NORM, 0);
			self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_HANG;
		}

		VectorNormalize (hookdir);
		VectorScale(hookdir, CTF_GRAPPLE_PULL_SPEED, hookdir);
		VectorCopy(hookdir, self->owner->velocity);
		SV_AddGravity(self->owner);
	}
}

void CTFFireGrapple (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t	*grapple;
	trace_t	tr;

	VectorNormalize (dir);

	grapple = G_Spawn();
	VectorCopy (start, grapple->s.origin);
	VectorCopy (start, grapple->s.old_origin);
	vectoangles (dir, grapple->s.angles);
	VectorScale (dir, speed, grapple->velocity);
	grapple->movetype = MOVETYPE_FLYMISSILE;
	grapple->clipmask = MASK_SHOT;
	grapple->solid = SOLID_BBOX;
	grapple->s.effects |= effect;
	VectorClear (grapple->mins);
	VectorClear (grapple->maxs);
	grapple->s.modelindex = gi.modelindex ("models/weapons/grapple/hook/tris.md2");
//	grapple->s.sound = gi.soundindex ("misc/lasfly.wav");
	grapple->owner = self;
	grapple->touch = CTFGrappleTouch;
//	grapple->nextthink = level.time + FRAMETIME;
//	grapple->think = CTFGrappleThink;
	grapple->dmg = damage;
	self->client->ctf_grapple = grapple;
	self->client->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
	gi.linkentity (grapple);

	tr = gi.trace (self->s.origin, NULL, NULL, grapple->s.origin, grapple, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (grapple->s.origin, -10, dir, grapple->s.origin);
		grapple->touch (grapple, tr.ent, NULL, NULL);
	}
}	

void CTFGrappleFire (edict_t *ent, vec3_t g_offset, int damage, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
	float volume = 1.0;

	if (ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY)
		return; // it's already out

	AngleVectors (ent->client->v_angle, forward, right, NULL);
//	VectorSet(offset, 24, 16, ent->viewheight-8+2);
	VectorSet(offset, 24, 8, ent->viewheight-8+2);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	if (ent->client->silencer_shots)
		volume = 0.2;

	gi.sound (ent, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grfire.wav"), volume, ATTN_NORM, 0);
	CTFFireGrapple (ent, start, forward, damage, CTF_GRAPPLE_SPEED, effect);

#if 0
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
#endif

	PlayerNoise(ent, start, PNOISE_WEAPON);
}


void CTFWeapon_Grapple_Fire (edict_t *ent)
{
	int		damage;

	damage = 10;
	CTFGrappleFire (ent, vec3_origin, damage, 0);
	ent->client->ps.gunframe++;
}

void CTFWeapon_Grapple (edict_t *ent)
{
	static int	pause_frames[]	= {10, 18, 27, 0};
	static int	fire_frames[]	= {6, 0};
	int prevstate,i;
	vec3_t	vv;
	edict_t *e;

	// if the the attack button is still down, stay in the firing frame
	if ((ent->client->buttons & BUTTON_ATTACK) && 
		ent->client->weaponstate == WEAPON_FIRING &&
		ent->client->ctf_grapple)
		ent->client->ps.gunframe = 9;

	if (!(ent->client->buttons & BUTTON_ATTACK) && 
		ent->client->ctf_grapple) 
	{
		i = ent->client->ctf_grapplestate;
		e = (edict_t*)ent->client->ctf_grapple;
		VectorCopy(e->s.origin,vv);
		CTFResetGrapple(ent->client->ctf_grapple);
		if (ent->client->weaponstate == WEAPON_FIRING)
			ent->client->weaponstate = WEAPON_READY;
	}

	if (ent->client->newweapon && 
		ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY &&
		ent->client->weaponstate == WEAPON_FIRING) {
		// he wants to change weapons while grappled
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = 32;
	}

	prevstate = ent->client->weaponstate;
	Weapon_Generic (ent, 5, 9, 31, 36, pause_frames, fire_frames, 
		CTFWeapon_Grapple_Fire);

	// if we just switched back to grapple, immediately go to fire frame
	if (prevstate == WEAPON_ACTIVATING &&
		ent->client->weaponstate == WEAPON_READY &&
		ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY) {
		if (!(ent->client->buttons & BUTTON_ATTACK))
			ent->client->ps.gunframe = 9;
		else
			ent->client->ps.gunframe = 5;
		ent->client->weaponstate = WEAPON_FIRING;
	}
}

int CTFLowTeam(void);
void cmd_RemoveLaserDefense(edict_t *ent);
void turret_remove(edict_t *ent);
void CTFTeam_f (edict_t *ent)
{
	char *t, *s;
	int desired_team;
	edict_t *scan = NULL;

	t = gi.args();

	if (!*t) {
		gi.cprintf(ent, PRINT_HIGH, "You are on the %s team.\n",
			CTFTeamName(ent->client->resp.ctf_team));
		return;
	}

//	if (ent->client->delay > level.time)
//		return;

	if (Q_stricmp(t, "red") == 0)
		desired_team = CTF_TEAM1;
	else if (Q_stricmp(t, "blue") == 0)
		desired_team = CTF_TEAM2;
	else {
		gi.cprintf(ent, PRINT_HIGH, "Unknown team %s.\n", t);
		return;
	}

	if (ent->client->resp.ctf_team == desired_team) {
		gi.cprintf(ent, PRINT_HIGH, "You are already on the %s team.\n",
			CTFTeamName(ent->client->resp.ctf_team));
		return;
	}

	if (((desired_team == CTF_TEAM2) && (CTFLowTeam() - ent->myskills.level < 0))
		|| ((desired_team == CTF_TEAM1) && (CTFLowTeam() + ent->myskills.level > 0))) {
		gi.cprintf(ent, PRINT_HIGH, "That team is stronger.\n");
		return;
	}
	

////
	ent->svflags = 0;
	ent->flags &= ~FL_GODMODE;
	ent->client->resp.ctf_team = desired_team;
	ent->client->resp.ctf_state = CTF_STATE_START;
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	CTFAssignSkin(ent, s);

	if (ent->solid == SOLID_NOT) { // spectator
		PutClientInServer (ent);
		// add a teleportation effect
		ent->s.event = EV_PLAYER_TELEPORT;
		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
		gi.bprintf(PRINT_HIGH, "%s joined the %s team.\n",
			ent->client->pers.netname, CTFTeamName(desired_team));
//		ent->client->delay = level.time + 10;
		return;
	}

	ent->health = 0;
	player_die (ent, ent, ent, 100000, vec3_origin);
	// don't even bother waiting for death frames
	ent->deadflag = DEAD_DEAD;

	gi.bprintf(PRINT_HIGH, "%s changed to the %s team.\n",
		ent->client->pers.netname, CTFTeamName(desired_team));
//	ent->client->delay = level.time + 30;
}

void CTFForceTeam (edict_t *ent)
{
	char *t, *s;
	int desired_team;
	edict_t *scan = NULL;

	t = gi.args();

	if (!*t) {
		gi.cprintf(ent, PRINT_HIGH, "You are on the %s team.\n",
			CTFTeamName(ent->client->resp.ctf_team));
		return;
	}

//	if (ent->client->delay > level.time)
//		return;

	if (Q_stricmp(t, "red") == 0)
		desired_team = CTF_TEAM1;
	else if (Q_stricmp(t, "blue") == 0)
		desired_team = CTF_TEAM2;
	else {
			gi.cprintf(ent, PRINT_HIGH, "Unknown team %s.\n", t);
		return;
	}

	if (ent->client->resp.ctf_team == desired_team) {
		gi.cprintf(ent, PRINT_HIGH, "You are already on the %s team.\n",
			CTFTeamName(ent->client->resp.ctf_team));
		return;
	}	

////
	ent->svflags = 0;
	ent->flags &= ~FL_GODMODE;
	ent->client->resp.ctf_team = desired_team;
	ent->client->resp.ctf_state = CTF_STATE_START;
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	CTFAssignSkin(ent, s);

	if (ent->solid == SOLID_NOT) { // spectator
		PutClientInServer (ent);
		// add a teleportation effect
		ent->s.event = EV_PLAYER_TELEPORT;
		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
		gi.bprintf(PRINT_HIGH, "%s joined the %s team.\n",
			ent->client->pers.netname, CTFTeamName(desired_team));
//		ent->client->delay = level.time + 10;
		return;
	}

	ent->health = 0;
	player_die (ent, ent, ent, 100000, vec3_origin);
	// don't even bother waiting for death frames
	ent->deadflag = DEAD_DEAD;

	gi.bprintf(PRINT_HIGH, "%s changed to the %s team.\n",
		ent->client->pers.netname, CTFTeamName(desired_team));
//	ent->client->delay = level.time + 30;
}

/*------------------------------------------------------------------------*/
/* TECH																	  */
/*------------------------------------------------------------------------*/

void CTFHasTech(edict_t *who)
{
	if (level.time - who->client->ctf_lasttechmsg > 2) {
		gi.centerprintf(who, "You already have a TECH powerup.");
		who->client->ctf_lasttechmsg = level.time;
	}
}

gitem_t *CTFWhat_Tech(edict_t *ent)
{
	gitem_t *tech;
	int i;

	i = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			ent->client->pers.inventory[ITEM_INDEX(tech)]) {
			return tech;
		}
		i++;
	}
	return NULL;
}

qboolean CTFPickup_Tech (edict_t *ent, edict_t *other)
{
	gitem_t *tech;
	int i;

	//K03 Begin
	if (other->client && other->myskills.level > 9)
		return false;

	if (SPREE_WAR == true && other == SPREE_DUDE)
		return false;
	//K03 End

	i = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			other->client->pers.inventory[ITEM_INDEX(tech)]) {
			CTFHasTech(other);
			return false; // has this one
		}
		i++;
	}
	
	// client only gets one tech
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	other->client->ctf_regentime = level.time;
	return true;
}

static void SpawnTech(gitem_t *item, edict_t *spot);

//K03 Begin
char *rune_modelfornum[] = {"models/ctf/resistance/tris.md2", "models/ctf/strength/tris.md2", "models/ctf/haste/tris.md2", "models/ctf/regeneration/tris.md2"};
char *rune_iconfornum[] = {"tech1", "tech2", "tech3", "tech4"};
int	rune_renderfx[] = {RF_SHELL_BLUE, RF_SHELL_RED, RF_SHELL_YELLOW, RF_SHELL_GREEN};
//K03 End

static edict_t *FindTechSpawn(void)
{
	edict_t *spot = NULL;
	int i = rand() % 16;

	while (i--)
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
	if (!spot)
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
	return spot;
}

static void TechThink(edict_t *tech)
{
	edict_t *spot;

	if ((spot = FindTechSpawn()) != NULL) {
		SpawnTech(tech->item, spot);
		G_FreeEdict(tech);
	} else {
		tech->nextthink = level.time + CTF_TECH_TIMEOUT;
		tech->think = TechThink;
	}
}

void CTFDrop_Tech(edict_t *ent, gitem_t *item)
{
	edict_t *tech;

	tech = Drop_Item(ent, item);
	tech->nextthink = level.time + CTF_TECH_TIMEOUT;
	tech->think = TechThink;
	//K03 Begin
	tech->s.effects |= EF_COLOR_SHELL;
	if (item == FindItemByClassname(tnames[0]))
		tech->s.renderfx |= rune_renderfx[0];
	else if (item == FindItemByClassname(tnames[1]))
		tech->s.renderfx |= rune_renderfx[1];
	else if (item == FindItemByClassname(tnames[2]))
		tech->s.renderfx |= rune_renderfx[2];
	else
		tech->s.renderfx |= rune_renderfx[3];
	//K03 End

	ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
}

void CTFDeadDropTech(edict_t *ent)
{
	gitem_t *tech;
	edict_t *dropped;
	int i;

	i = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			ent->client->pers.inventory[ITEM_INDEX(tech)]) {
			if(ent->client->pers.inventory[ITEM_INDEX(tech)])
			{
				dropped = Drop_Item(ent, tech);
				// hack the velocity to make it bounce random
				dropped->velocity[0] = (rand() % 600) - 300;
				dropped->velocity[1] = (rand() % 600) - 300;
				dropped->nextthink = level.time + CTF_TECH_TIMEOUT;
				dropped->think = TechThink;
				dropped->owner = NULL;
				//K03 Begin
				dropped->s.effects |= EF_COLOR_SHELL;
				if (tech == FindItemByClassname(tnames[0]))
					dropped->s.renderfx |= rune_renderfx[0];
				else if (tech == FindItemByClassname(tnames[1]))
					dropped->s.renderfx |= rune_renderfx[1];
				else if (tech == FindItemByClassname(tnames[2]))
					dropped->s.renderfx |= rune_renderfx[2];
				else
					dropped->s.renderfx |= rune_renderfx[3];
				//K03 End
			}
			ent->client->pers.inventory[ITEM_INDEX(tech)] = 0;
		}
		i++;
	}
}

static void SpawnTech(gitem_t *item, edict_t *spot)
{
	edict_t	*ent;
	vec3_t	forward, right;
	vec3_t  angles;

	ent = G_Spawn();

	ent->classname = item->classname;
	ent->item = item;
	ent->spawnflags = DROPPED_ITEM;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
	VectorSet (ent->mins, -15, -15, -15);
	VectorSet (ent->maxs, 15, 15, 15);
	if(ent->item->world_model)	//PONKO
	gi.setmodel (ent, ent->item->world_model);
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;
	ent->owner = ent;

	angles[0] = 0;
	angles[1] = rand() % 360;
	angles[2] = 0;

	AngleVectors (angles, forward, right, NULL);
	VectorCopy (spot->s.origin, ent->s.origin);
	ent->s.origin[2] += 16;
	VectorScale (forward, 100, ent->velocity);
	ent->velocity[2] = 300;

	ent->nextthink = level.time + CTF_TECH_TIMEOUT;
	ent->think = TechThink;

	//K03 Begin
	ent->s.effects |= EF_COLOR_SHELL;
	if (item == FindItemByClassname(tnames[0]))
		ent->s.renderfx |= rune_renderfx[0];
	else if (item == FindItemByClassname(tnames[1]))
		ent->s.renderfx |= rune_renderfx[1];
	else if (item == FindItemByClassname(tnames[2]))
		ent->s.renderfx |= rune_renderfx[2];
	else
		ent->s.renderfx |= rune_renderfx[3];
	//K03 End

	gi.linkentity (ent);
}

static void SpawnTechs(edict_t *ent)
{
	gitem_t *tech;
	edict_t *spot;
	int i;

	i = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			(spot = FindTechSpawn()) != NULL)
			//K03 Begin
			if (zoidctf->value)
			{
				tech->world_model = rune_modelfornum[i];
				tech->icon = rune_iconfornum[i];
			}
			//K03 End
			SpawnTech(tech, spot);
		i++;
	}
}

// frees the passed edict!
void CTFRespawnTech(edict_t *ent)
{
	edict_t *spot;

	if ((spot = FindTechSpawn()) != NULL)
		SpawnTech(ent->item, spot);
	G_FreeEdict(ent);
}

void CTFSetupTechSpawn(void)
{
	edict_t *ent;

	if (techspawn || ((int)dmflags->value & DF_CTF_NO_TECH))
		return;

	ent = G_Spawn();
	ent->nextthink = level.time + 2;
	ent->think = SpawnTechs;
	techspawn = true;
}

int CTFApplyResistance(edict_t *ent, int dmg)
{
	static gitem_t *tech = NULL;
	float volume = 1.0;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech1");
	if (dmg && tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)]) {
		// make noise
	   	gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech1.wav"), volume, ATTN_NORM, 0);
		if (ent->myskills.level <= 3)
			return dmg / 2;
		else if (ent->myskills.level <= 6)
			return dmg / 1.5;
		else
			return dmg / 1.25;
	}
	return dmg;
}

int CTFApplyStrength(edict_t *ent, int dmg)
{
	static gitem_t *tech = NULL;

	if (!tech)
		tech = FindItemByClassname("item_tech2");
	if (dmg && tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)]) {
		if (ent->myskills.level <= 3)
			return 2 * dmg;
		else if (ent->myskills.level <= 6)
			return 1.5 * dmg;
		else
			return 1.25 * dmg;
	}
	return dmg;
}

qboolean CTFApplyStrengthSound(edict_t *ent)
{
	static gitem_t *tech = NULL;
	float volume = 1.0;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech2");
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)]) {
		if (ent->client->ctf_techsndtime < level.time) {
			ent->client->ctf_techsndtime = level.time + 1;
			if (ent->client->quad_framenum > level.framenum)
				gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech2x.wav"), volume, ATTN_NORM, 0);
			else
				gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech2.wav"), volume, ATTN_NORM, 0);
		}
		return true;
	}
	return false;
}


qboolean CTFApplyHaste(edict_t *ent)
{
	static gitem_t *tech = NULL;

	if (!tech)
		tech = FindItemByClassname("item_tech3");
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)])
		return true;
	return false;
}

void CTFApplyRegeneration(edict_t *ent)
{
	static gitem_t *tech = NULL;
	qboolean noise = false;
	gclient_t *client;
	int index;
	float volume = 1.0;

	client = ent->client;
	if (!client)
		return;

	if (ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech4");
	if (tech && client->pers.inventory[ITEM_INDEX(tech)]) {
		if (client->ctf_regentime < level.time) {
			client->ctf_regentime = level.time;
			if (ent->health < ent->max_health) {
				if (ent->myskills.level <= 3)
					ent->health += 10;
				else if (ent->myskills.level <= 6)
					ent->health += 5;
				else
					ent->health += 2;
				if (ent->health > ent->max_health)
					ent->health = ent->max_health;
				client->ctf_regentime += 1.0;
				noise = true;
			}
			index = ITEM_INDEX(FindItem("Body Armor"));//K03 ArmorIndex (ent);
			if (index && client->pers.inventory[index] < (MAX_ARMOR(ent)) ) {
				if (ent->myskills.level <= 3)
					client->pers.inventory[index] += 10;
				else if (ent->myskills.level <= 6)
					client->pers.inventory[index] += 5;
				else
					client->pers.inventory[index] += 2;
				if (client->pers.inventory[index] > (MAX_ARMOR(ent)) )
					client->pers.inventory[index] = (MAX_ARMOR(ent));
				client->ctf_regentime += 1.0;
				noise = true;
			}
		}
		if (noise && ent->client->ctf_techsndtime < level.time) {
			ent->client->ctf_techsndtime = level.time + 1;
			gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech4.wav"), volume, ATTN_NORM, 0);
		}
	}
	//gi.dprintf("DEBUG: ctf_regentime is %f\n", (client->ctf_regentime - level.time));
}

qboolean CTFHasRegeneration(edict_t *ent)
{
	static gitem_t *tech = NULL;

	if (!tech)
		tech = FindItemByClassname("item_tech4");
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)])
		return true;
	return false;
}

/*
======================================================================

SAY_TEAM

======================================================================
*/

// This array is in 'importance order', it indicates what items are
// more important when reporting their names.
struct {
	char *classname;
	int priority;
} loc_names[] = 
{
	{	"item_flag_team1",			1 },
	{	"item_flag_team2",			1 },
	{	"item_quad",				2 }, 
	{	"item_invulnerability",		2 },
	{	"weapon_bfg",				3 },
	{	"weapon_railgun",			4 },
	{	"weapon_rocketlauncher",	4 },
	{	"weapon_hyperblaster",		4 },
	{	"weapon_chaingun",			4 },
	{	"weapon_grenadelauncher",	4 },
	{	"weapon_machinegun",		4 },
	{	"weapon_supershotgun",		4 },
	{	"weapon_shotgun",			4 },
	{	"item_power_screen",		5 },
	{	"item_power_shield",		5 },
	{	"item_armor_body",			6 },
	{	"item_armor_combat",		6 },
	{	"item_armor_jacket",		6 },
	{	"item_silencer",			7 },
	{	"item_breather",			7 },
	{	"item_enviro",				7 },
	{	"item_adrenaline",			7 },
	{	"item_bandolier",			8 },
	{	"item_pack",				8 },
	{ NULL, 0 }
};


static void CTFSay_Team_Location(edict_t *who, char *buf)
{
	edict_t *what = NULL;
	edict_t *hot = NULL;
	float hotdist = 999999, newdist;
	vec3_t v;
	int hotindex = 999;
	int i;
	gitem_t *item;
	int nearteam = -1;
	edict_t *flag1, *flag2;
	qboolean hotsee = false;
	qboolean cansee;

	while ((what = loc_findradius(what, who->s.origin, 1024)) != NULL) {
		// find what in loc_classnames
		for (i = 0; loc_names[i].classname; i++)
			if (strcmp(what->classname, loc_names[i].classname) == 0)
				break;
		if (!loc_names[i].classname)
			continue;
		// something we can see get priority over something we can't
		cansee = loc_CanSee(what, who);
		if (cansee && !hotsee) {
			hotsee = true;
			hotindex = loc_names[i].priority;
			hot = what;
			VectorSubtract(what->s.origin, who->s.origin, v);
			hotdist = VectorLength(v);
			continue;
		}
		// if we can't see this, but we have something we can see, skip it
		if (hotsee && !cansee)
			continue;
		if (hotsee && hotindex < loc_names[i].priority)
			continue;
		VectorSubtract(what->s.origin, who->s.origin, v);
		newdist = VectorLength(v);
		if (newdist < hotdist || 
			(cansee && loc_names[i].priority < hotindex)) {
			hot = what;
			hotdist = newdist;
			hotindex = i;
			hotsee = loc_CanSee(hot, who);
		}
	}

	if (!hot) {
		strcpy(buf, "nowhere");
		return;
	}

	// we now have the closest item
	// see if there's more than one in the map, if so
	// we need to determine what team is closest
	what = NULL;
	while ((what = G_Find(what, FOFS(classname), hot->classname)) != NULL) {
		if (what == hot)
			continue;
		// if we are here, there is more than one, find out if hot
		// is closer to red flag or blue flag
		if ((flag1 = G_Find(NULL, FOFS(classname), "item_flag_team1")) != NULL &&
			(flag2 = G_Find(NULL, FOFS(classname), "item_flag_team2")) != NULL) {
			VectorSubtract(hot->s.origin, flag1->s.origin, v);
			hotdist = VectorLength(v);
			VectorSubtract(hot->s.origin, flag2->s.origin, v);
			newdist = VectorLength(v);
			if (hotdist < newdist)
				nearteam = CTF_TEAM1;
			else if (hotdist > newdist)
				nearteam = CTF_TEAM2;
		}
		break;
	}

	if ((item = FindItemByClassname(hot->classname)) == NULL) {
		strcpy(buf, "nowhere");
		return;
	}

	// in water?
	if (who->waterlevel)
		strcpy(buf, "in the water ");
	else
		*buf = 0;

	// near or above
	VectorSubtract(who->s.origin, hot->s.origin, v);
	if (fabs(v[2]) > fabs(v[0]) && fabs(v[2]) > fabs(v[1]))
		if (v[2] > 0)
			strcat(buf, "above ");
		else
			strcat(buf, "below ");
	else
		strcat(buf, "near ");

	if (nearteam == CTF_TEAM1)
		strcat(buf, "the red ");
	else if (nearteam == CTF_TEAM2)
		strcat(buf, "the blue ");
	else
		strcat(buf, "the ");

	strcat(buf, item->pickup_name);
}

static void CTFSay_Team_Armor(edict_t *who, char *buf)
{
	gitem_t		*item;
	int			index, cells;
	int			power_armor_type;

	*buf = 0;

	power_armor_type = PowerArmorType (who);
	if (power_armor_type)
	{
		cells = who->client->pers.inventory[ITEM_INDEX(Fdi_CELLS/*FindItem ("cells")*/)];
		if (cells)
			sprintf(buf+strlen(buf), "%s with %i cells ",
				(power_armor_type == POWER_ARMOR_SCREEN) ?
				"Power Screen" : "Power Shield", cells);
	}

	index = ArmorIndex (who);
	if (index)
	{
		item = GetItemByIndex (index);
		if (item) {
			if (*buf)
				strcat(buf, "and ");
			sprintf(buf+strlen(buf), "%i units of %s",
				who->client->pers.inventory[index], item->pickup_name);
		}
	}

	if (!*buf)
		strcpy(buf, "no armor");
}

static void CTFSay_Team_Health(edict_t *who, char *buf)
{
	if (who->health <= 0)
		strcpy(buf, "dead");
	else
		sprintf(buf, "%i health", who->health);
}

static void CTFSay_Team_Tech(edict_t *who, char *buf)
{
	gitem_t *tech;
	int i;

	// see if the player has a tech powerup
	i = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			who->client->pers.inventory[ITEM_INDEX(tech)]) {
			sprintf(buf, "the %s", tech->pickup_name);
			return;
		}
		i++;
	}
	strcpy(buf, "no powerup");
}

static void CTFSay_Team_Weapon(edict_t *who, char *buf)
{
	if (who->client->pers.weapon)
		strcpy(buf, who->client->pers.weapon->pickup_name);
	else
		strcpy(buf, "none");
}

static void CTFSay_Team_Sight(edict_t *who, char *buf)
{
	int i;
	edict_t *targ;
	int n = 0;
	char s[1024];
	char s2[1024];

	*s = *s2 = 0;
	for (i = 1; i <= maxclients->value; i++) {
		targ = g_edicts + i;
		if (!targ->inuse || 
			targ == who ||
			!loc_CanSee(targ, who))
			continue;
		if (*s2) {
			if (strlen(s) + strlen(s2) + 3 < sizeof(s)) {
				if (n)
					strcat(s, ", ");
				strcat(s, s2);
				*s2 = 0;
			}
			n++;
		}
		strcpy(s2, targ->client->pers.netname);
	}
	if (*s2) {
		if (strlen(s) + strlen(s2) + 6 < sizeof(s)) {
			if (n)
				strcat(s, " and ");
			strcat(s, s2);
		}
		strcpy(buf, s);
	} else
		strcpy(buf, "no one");
}

void CTFSay_Team(edict_t *who, char *msg)
{
	char outmsg[1024];
	char buf[1024];
	int i;
	char *p;
	edict_t *cl_ent;

	outmsg[0] = 0;

	if (*msg == '\"') {
		msg[strlen(msg) - 1] = 0;
		msg++;
	}

	for (p = outmsg; *msg && (p - outmsg) < sizeof(outmsg) - 1; msg++) {
		if (*msg == '%') {
			switch (*++msg) {
				case 'l' :
				case 'L' :
					CTFSay_Team_Location(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;
				case 'a' :
				case 'A' :
					CTFSay_Team_Armor(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;
				case 'h' :
				case 'H' :
					CTFSay_Team_Health(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;
				case 't' :
				case 'T' :
					CTFSay_Team_Tech(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;
				case 'w' :
				case 'W' :
					CTFSay_Team_Weapon(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;

				case 'n' :
				case 'N' :
					CTFSay_Team_Sight(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;

				default :
					*p++ = *msg;
			}
		} else
			*p++ = *msg;
	}
	*p = 0;

	for (i = 0; i < maxclients->value; i++) {
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (cl_ent->client->resp.ctf_team == who->client->resp.ctf_team)
		{
			if (strcmp(cl_ent->myskills.title, "") != 0 && cl_ent->solid != SOLID_NOT)
				gi.cprintf(cl_ent, PRINT_HIGH, "%s ", cl_ent->myskills.title);
			gi.cprintf(cl_ent, PRINT_CHAT, "%s", outmsg);
		}
	}
}

/*-----------------------------------------------------------------------*/
/*QUAKED misc_ctf_banner (1 .5 0) (-4 -64 0) (4 64 248) TEAM2
The origin is the bottom of the banner.
The banner is 248 tall.
*/
static void misc_ctf_banner_think (edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 16;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_ctf_banner (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/ctf/banner/tris.md2");
	if (ent->spawnflags & 1) // team2
		ent->s.skinnum = 1;

	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_ctf_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

/*QUAKED misc_ctf_small_banner (1 .5 0) (-4 -32 0) (4 32 124) TEAM2
The origin is the bottom of the banner.
The banner is 124 tall.
*/
void SP_misc_ctf_small_banner (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/ctf/banner/small.md2");
	if (ent->spawnflags & 1) // team2
		ent->s.skinnum = 1;

	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_ctf_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}


/*-----------------------------------------------------------------------*/
//Chamooze
//Returns (levelsum_of_redteam - levelsum_of_blueteam)
//If negative, blue team has higher levelsum than red and vice versa..

int CTFLowTeam(void)
{
	edict_t *player;
	int levelsum[] = {0,0};
	int i;

	for (i = 1; i <= maxclients->value; i++) {
		player = &g_edicts[i];
		if (!player->inuse)
			continue;
		if (player->solid == SOLID_NOT)
			continue;
		
		if (player->client->resp.ctf_team == CTF_TEAM1) {
			levelsum[0] += (player->myskills.level + 1);
		}
		else if (player->client->resp.ctf_team == CTF_TEAM2) {
			levelsum[1] += (player->myskills.level + 1);
		}
	}

	return (levelsum[0] - levelsum[1]);
}

//GHz: Returns number of clients with same class as ent
int ClassNum(edict_t *ent, int team)
{
	edict_t *player;
	int i, sameclass = 0;

	for (i = 1; i <= maxclients->value; i++){
		player = &g_edicts[i];

		if (!player->inuse)
			continue;
		if (player == ent)
			continue;
		if (player->solid == SOLID_NOT)
			continue;
		if (ctf->value && player->client->resp.ctf_team != team)
			continue;

		//GHz: This is to prevent Controller wannabees from bypassing this check
		if ((ent->myskills.monster_abilities > 0
			|| ent->myskills.build_sentry > 0
			|| ent->myskills.build_lasers > 0) && player->myskills.class_num == 4)
		{
			sameclass++;
			continue;
		}

		if (ent->myskills.class_num == player->myskills.class_num)
			sameclass++;		
	}

	return sameclass;
}

//GHz: Kills enemies within radius of flag
void CTFKillCampers(edict_t *flag)
{
	edict_t *enemy = NULL;
	int teamnum;
	vec3_t zvec = {0,0,0};

	if (strcmp(flag->classname, "item_flag_team1") == 0) //GHz: Which flag is this?
		teamnum = 1;
	else
		teamnum = 2;

	while ((enemy = findradius(enemy, flag->s.origin, 1024)) != NULL)
	{
		if (!enemy->inuse)
			continue;
		if (!enemy->takedamage)
			continue;
		if (enemy == flag)
			continue;
		if (enemy->solid == SOLID_NOT)
			continue;
		if (enemy->health <= 0)
			continue;
		if (enemy->client->resp.ctf_team == teamnum) //GHz: Don't target our teammates :)
			continue;

		//GHz: Kill the bastards!!!
		T_Damage(enemy, enemy, enemy, zvec, enemy->s.origin, NULL, 5000, 1, 0, MOD_SUICIDE);
	}
}

/*
//Chamooze
//Returns (levelsum_of_redteam - levelsum_of_blueteam) for the bots reserved for the teams
//If negative, blue team has higher levelsum than red and vice versa..

  --- NOT FUNCTIONAL ---

int CTFLowReservedBots(void)
{
	int levelsum[] = {0,0};
	int i;

	{
		if(Bot[i].spflg == BOT_SPAWNNOT)
		{
			Bot[i].spflg = BOT_SPRESERVED;
			SpawnWaitingBots++;
			return;
		}
	}
	for(i = 0;i < MAXBOTS; i++)
		if(Bot[i].spflg == BOT_SPAWNNOT) {
			if (Bot[i].team == CTF_TEAM1) {
				levelsum[0] += (player->myskills.level + 1);
			}
			else if (Bot[i].team == CTF_TEAM2) {
				levelsum[1] += (player->myskills.level + 1);
			}
		}
	}

	return (levelsum[0] - levelsum[1]);
}
*/

void CTFJoinTeam(edict_t *ent, int desired_team)
{
	char *s;

	PMenu_Close(ent);

	//K03 must be added through start reign also ent->svflags &= ~SVF_NOCLIENT;
	ent->client->resp.ctf_team = desired_team;
	ent->client->resp.ctf_state = CTF_STATE_START;
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	CTFAssignSkin(ent, s);

/*	if(!hokuto->value)
	{*/ 
		PutClientInServer (ent);
		// add a teleportation effect
		//K03 must be added through start reign also ent->s.event = EV_PLAYER_TELEPORT;
		// hold in place briefly
		/* K03 must be added through start reign also
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;*/
		gi.bprintf(PRINT_HIGH, "%s joined the %s team.\n",
			ent->client->pers.netname, CTFTeamName(ent->client->resp.ctf_team/*desired_team*/));
				JoinTheGame(ent, NULL); //GHz: force them to start reign
/*	}
	else ZigockJoinMenu(ent);*/
}
void CTFJoinTeam1(edict_t *ent, pmenu_t *p)
{
	if (CTF_FORCE_EVENTEAMS == 3)
	{
		if (random() > 0.5)
			CTFJoinTeam(ent, CTF_TEAM1);
		else
			CTFJoinTeam(ent, CTF_TEAM2);
		return;
	}
	if ((CTF_FORCE_EVENTEAMS) && CTFTeamValue(CTF_TEAM1) > CTFTeamValue(CTF_TEAM2)/*(CTFLowTeam() > 1)*/) {
		gi.cprintf(ent, PRINT_HIGH, "Can't join that team, other team is weaker.\n");
		return;
	}

	CTFJoinTeam(ent, CTF_TEAM1);
}

void CTFJoinTeam2(edict_t *ent, pmenu_t *p)
{
	if (CTF_FORCE_EVENTEAMS == 3)
	{
		if (random() > 0.5)
			CTFJoinTeam(ent, CTF_TEAM1);
		else
			CTFJoinTeam(ent, CTF_TEAM2);
		return;
	}
	if ((CTF_FORCE_EVENTEAMS) && CTFTeamValue(CTF_TEAM2) > CTFTeamValue(CTF_TEAM1)/*(CTFLowTeam() < -1)*/) {
		gi.cprintf(ent, PRINT_HIGH, "Can't join that team, other team is weaker.\n");
		return;
	}
	
	CTFJoinTeam(ent, CTF_TEAM2);
}

void CTFChaseCam(edict_t *ent, pmenu_t *p)
{
	int i;
	edict_t *e;

	if (ent->client->chase_target) {
		ent->client->chase_target = NULL;
		PMenu_Close(ent);
		return;
	}

	for (i = 1; i <= maxclients->value; i++) {
		e = g_edicts + i;
		if (e->inuse && e->solid != SOLID_NOT) {
			ent->client->chase_target = e;
			PMenu_Close(ent);
			ent->client->update_chase = true;
			break;
		}
	}
}

void CTFReturnToMain(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);
	CTFOpenJoinMenu(ent);
}

void CTFCredits(edict_t *ent, pmenu_t *p);

void DeathmatchScoreboard (edict_t *ent);

void CTFShowScores(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);

	ent->client->showscores = true;
	ent->client->showinventory = false;
	DeathmatchScoreboard (ent);
}

pmenu_t creditsmenu[] = {
	{ "*Quake II",						PMENU_ALIGN_CENTER, 0, NULL },
	{ "*ThreeWave Capture the Flag",	PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, 0, NULL },
	{ "*Programming",					PMENU_ALIGN_CENTER, 0, NULL }, 
	{ "Dave 'Zoid' Kirsch",				PMENU_ALIGN_CENTER, 0, NULL },
	{ "*Level Design", 					PMENU_ALIGN_CENTER, 0, NULL },
	{ "Christian Antkow",				PMENU_ALIGN_CENTER, 0, NULL },
	{ "Tim Willits",					PMENU_ALIGN_CENTER, 0, NULL },
	{ "Dave 'Zoid' Kirsch",				PMENU_ALIGN_CENTER, 0, NULL },
	{ "*Art",							PMENU_ALIGN_CENTER, 0, NULL },
	{ "Adrian Carmack Paul Steed",		PMENU_ALIGN_CENTER, 0, NULL },
	{ "Kevin Cloud",					PMENU_ALIGN_CENTER, 0, NULL },
	{ "*Sound",							PMENU_ALIGN_CENTER, 0, NULL },
	{ "Tom 'Bjorn' Klok",				PMENU_ALIGN_CENTER, 0, NULL },
	{ "*Original CTF Art Design",		PMENU_ALIGN_CENTER, 0, NULL },
	{ "Brian 'Whaleboy' Cozzens",		PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, 0, NULL },
	{ "Return to Main Menu",			PMENU_ALIGN_LEFT, 0, CTFReturnToMain }
};


pmenu_t joinmenu[] = {
	{ "*The Vortex",						PMENU_ALIGN_CENTER, 0, NULL },
	{ "*ThreeWave Capture the Flag",	PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, 0, NULL },
	{ "1. Join Red Team",				PMENU_ALIGN_LEFT, 0, CTFJoinTeam1 },
	{ NULL,								PMENU_ALIGN_LEFT, 0, NULL },
	{ "2. Join Blue Team",				PMENU_ALIGN_LEFT, 0, CTFJoinTeam2 },
	{ NULL,								PMENU_ALIGN_LEFT, 0, NULL },
	{ "3. Chase Camera",				PMENU_ALIGN_LEFT, 0, CTFChaseCam },
	{ "4. Credits",						PMENU_ALIGN_LEFT, 0, CTFCredits },
	{ NULL,								PMENU_ALIGN_LEFT, 0, NULL },
	{ "Use [ and ] to move cursor",		PMENU_ALIGN_LEFT, 0, NULL },
	{ "ENTER to select",				PMENU_ALIGN_LEFT, 0, NULL },
	{ "ESC to Exit Menu",				PMENU_ALIGN_LEFT, 0, NULL },
	{ "(TAB to Return)",				PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, 0, NULL },
};

int CTFUpdateJoinMenu(edict_t *ent)
{
	static char levelname[32];
	static char team1players[32];
	static char team2players[32];
	int num1, num2, i;

	joinmenu[4].text = "1. Join Red Team";
	joinmenu[4].SelectFunc = CTFJoinTeam1;
	joinmenu[6].text = "2. Join Blue Team";
	joinmenu[6].SelectFunc = CTFJoinTeam2;

	if (ctf_forcejoin->string && *ctf_forcejoin->string) {
		if (stricmp(ctf_forcejoin->string, "red") == 0) {
			joinmenu[6].text = NULL;
			joinmenu[6].SelectFunc = NULL;
		} else if (stricmp(ctf_forcejoin->string, "blue") == 0) {
			joinmenu[4].text = NULL;
			joinmenu[4].SelectFunc = NULL;
		}
	}

	if (ent->client->chase_target)
		joinmenu[8].text = "3. Leave Chase Camera";
	else
		joinmenu[8].text = "3. Chase Camera";

	levelname[0] = '*';
	if (g_edicts[0].message)
		strncpy(levelname+1, g_edicts[0].message, sizeof(levelname) - 2);
	else
		strncpy(levelname+1, level.mapname, sizeof(levelname) - 2);
	levelname[sizeof(levelname) - 1] = 0;

	num1 = num2 = 0;
	for (i = 0; i < maxclients->value; i++) {
		if (!g_edicts[i+1].inuse)
			continue;
		if (game.clients[i].resp.ctf_team == CTF_TEAM1)
			num1++;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
			num2++;
	}

	sprintf(team1players, "  (%d players)", num1);
	sprintf(team2players, "  (%d players)", num2);

	joinmenu[2].text = levelname;
	if (joinmenu[4].text)
		joinmenu[5].text = team1players;
	else
		joinmenu[5].text = NULL;
	if (joinmenu[6].text)
		joinmenu[7].text = team2players;
	else
		joinmenu[7].text = NULL;
	
	if (num1 > num2)
		return CTF_TEAM1;
	else if (num2 > num1)
		return CTF_TEAM1;
	return (rand() & 1) ? CTF_TEAM1 : CTF_TEAM2;
}

void CTFOpenJoinMenu(edict_t *ent)
{
	int team;

	team = CTFUpdateJoinMenu(ent);
	if (ent->client->chase_target)
		team = 8;
	else if (team == CTF_TEAM1)
		team = 4;
	else
		team = 6;
	PMenu_Open(ent, joinmenu, team, sizeof(joinmenu) / sizeof(pmenu_t), true, true);
}

void CTFCredits(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, creditsmenu, -1, sizeof(creditsmenu) / sizeof(pmenu_t), false, true);
}

qboolean CTFStartClient(edict_t *ent)
{
	if (ent->client->resp.ctf_team != CTF_NOTEAM)
		return false;

	if (!((int)dmflags->value & DF_CTF_FORCEJOIN)) {
		// start as 'observer'
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->resp.ctf_team = CTF_NOTEAM;
		ent->client->ps.gunindex = 0;
		gi.linkentity (ent);

		CTFOpenJoinMenu(ent);
		return true;
	}
	return false;
}

qboolean CTFCheckRules(void)
{
	if (capturelimit->value && 
		(ctfgame.team1 >= capturelimit->value ||
		ctfgame.team2 >= capturelimit->value)) {
		gi.bprintf (PRINT_HIGH, "Capturelimit hit.\n");
		return true;
	}
	return false;
}

/*--------------------------------------------------------------------------
 * just here to help old map conversions
 *--------------------------------------------------------------------------*/

static void old_teleporter_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t		*dest;
	int			i;
	vec3_t		forward;

	if (!other->client)
		return;
	dest = G_Find (NULL, FOFS(targetname), self->target);
	if (!dest)
	{
		gi.dprintf ("Couldn't find destination\n");
		return;
	}

//ZOID
	CTFPlayerResetGrapple(other);
//ZOID

	// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity (other);

	VectorCopy (dest->s.origin, other->s.origin);
	VectorCopy (dest->s.origin, other->s.old_origin);
//	other->s.origin[2] += 10;

	// clear the velocity and hold them in place briefly
	VectorClear (other->velocity);
	other->client->ps.pmove.pm_time = 160>>3;		// hold time
	other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	// draw the teleport splash at source and on the player
	self->enemy->s.event = EV_PLAYER_TELEPORT;
	other->s.event = EV_PLAYER_TELEPORT;

	// set angles
	for (i=0 ; i<3 ; i++)
		other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);

	other->s.angles[PITCH] = 0;
	other->s.angles[YAW] = dest->s.angles[YAW];
	other->s.angles[ROLL] = 0;
	VectorCopy (dest->s.angles, other->client->ps.viewangles);
	VectorCopy (dest->s.angles, other->client->v_angle);

	// give a little forward velocity
	AngleVectors (other->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, 200, other->velocity);

	// kill anything at the destination
	if (!KillBox (other))
	{
	}

	gi.linkentity (other);
}

/*QUAKED trigger_teleport (0.5 0.5 0.5) ?
Players touching this will be teleported
*/
void SP_trigger_teleport (edict_t *ent)
{
	edict_t *s;
	int i;

	if (!ent->target)
	{
		gi.dprintf ("teleporter without a target.\n");
		G_FreeEdict (ent);
		return;
	}

	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->touch = old_teleporter_touch;
	if(ent->model)
	gi.setmodel (ent, ent->model);
	gi.linkentity (ent);

	// noise maker and splash effect dude
	s = G_Spawn();
	ent->enemy = s;
	for (i = 0; i < 3; i++)
		s->s.origin[i] = ent->mins[i] + (ent->maxs[i] - ent->mins[i])/2;
	s->s.sound = gi.soundindex ("world/hum1.wav");
	gi.linkentity(s);
	
}

/*QUAKED info_teleport_destination (0.5 0.5 0.5) (-16 -16 -24) (16 16 32)
Point trigger_teleports at these.
*/
void SP_info_teleport_destination (edict_t *ent)
{
	ent->s.origin[2] += 16;
}

//PON
void SpawnExtra(vec3_t position,char *classname);
void ED_CallSpawn (edict_t *ent);

void SpawnExtra(vec3_t position,char *classname)
{
	edict_t		*it_ent;

	it_ent = G_Spawn();
	
	it_ent->classname = classname;
	VectorCopy(position,it_ent->s.origin);
	ED_CallSpawn(it_ent);

	if(ctf->value && chedit->value)
	{
		it_ent->moveinfo.speed = -1;
		it_ent->s.effects |= EF_QUAD;
	}
}
