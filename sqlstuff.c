#include "g_local.h"
#include "bot.h"
#include "mysql/mysql.h"

#define		DEFALT_SQL_STMT	"select * from deathmatch"

MYSQL		myData;
qboolean	connected_to_gds=true;
int mysql_processes=0;

void Print_ConnectionGDS(edict_t *ent)
{
	//if (!(ent->svflags & SVF_MONSTER) && gds_uselocal->value != 1)
		//gi.cprintf(ent, PRINT_HIGH, "GDS CONNECTION: %s\n", (connected_to_gds == true) ? "CONNECTED" : "DISCONNECTED");
	//else if (!(ent->svflags & SVF_MONSTER))
		gi.cprintf(ent, PRINT_HIGH, "GDS CONNECTION: LOCAL\n");
}

