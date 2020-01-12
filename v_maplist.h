#ifndef V_MAPLIST_H
#define V_MAPLIST_H

#define MAX_VOTERS			(MAX_CLIENTS * 2)
#define MAX_MAPS			64
#define MAX_MAPNAME_LEN		16

#define MAPMODE_PVP			1
#define MAPMODE_PVM			2
#define MAPMODE_DOM			3
#define MAPMODE_CTF			4
#define MAPMODE_FFA			5
#define MAPMODE_INV			6

#define ML_ROTATE_SEQ          0
#define ML_ROTATE_RANDOM       1
#define ML_ROTATE_NUM_CHOICES  2

//***************************************************************************************
//***************************************************************************************
//QW// FIXME: This originally had name[MAX_MAPNAME_LEN].
// Should this not be MAX_QPATH * MAX_MAPS?  See v_maplist_s below.
// Needs testing. 
// Note to self: the game was historically
// restricted to MAX_QPATH = 64. Windows and Linux both
// support much larger paths now and if the game doesn't rely
// on the engine to open files then non-standard _MAX_PATH might
// be more appropriate. Test GCC/clang for support of this.
typedef struct
{
	char	name[MAX_QPATH];
	int		monsters;
} mapdata_t;

mapdata_t	mapdata;

typedef struct v_maplist_s
{
	int  nummaps;          // number of maps in list
	mapdata_t maps[MAX_MAPS * MAX_MAPNAME_LEN];//4.5
	//char mapnames[MAX_MAPS][MAX_MAPNAME_LEN];
} v_maplist_t;

//***************************************************************************************
//***************************************************************************************

typedef struct votes_s
{
	qboolean	used; // Paril: in "new" system, this means in progress.
	int			mode;
	int			mapindex;
	char		name[24];
	char		ip[16];
} votes_t;

//***************************************************************************************
//***************************************************************************************

#endif