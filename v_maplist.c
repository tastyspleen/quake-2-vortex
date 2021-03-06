#include "g_local.h"

int v_LoadMapList(int mode)
{
	FILE* fptr;
	v_maplist_t* maplist;
	char filename[256];
	int iterator = 0;

	//determine path
	Com_sprintf(filename, sizeof filename, "%s/Settings/", game_path->string);

	switch (mode)
	{
	case MAPMODE_PVP:
		strcat(filename, "maplist_PVP.txt");
		maplist = &maplist_PVP;
		break;
	case MAPMODE_DOM:
		strcat(filename, "maplist_DOM.txt");
		maplist = &maplist_DOM;
		break;
	case MAPMODE_PVM:
		strcat(filename, "maplist_PVM.txt");
		maplist = &maplist_PVM;
		break;
	case MAPMODE_CTF:
		strcat(filename, "maplist_CTF.txt");
		maplist = &maplist_CTF;
		break;
	case MAPMODE_FFA:
		strcat(filename, "maplist_FFA.txt");
		maplist = &maplist_FFA;
		break;
	case MAPMODE_INV:
		strcat(filename, "maplist_INV.txt");
		maplist = &maplist_INV;
		break;
	default:
		gi.dprintf("ERROR in v_LoadMapList(). Incorrect map mode. (%d)\n", mode);
		return 0;
	}

	//gi.dprintf("mode = %d\n", mode);

	if ((fptr = fopen(filename, "r")) != NULL)
	{
		char buf[MAX_INFO_STRING], * s;

		while (fgets(buf, MAX_INFO_STRING, fptr) != NULL)
		{
			// tokenize string using space, comma or tabs as separator
			if ((s = strtok(buf, " ,\t")) != NULL)	//s is zero-terminated by strtok
			{
				// copy map name to list
				strcpy(maplist->maps[iterator].name, s);
			}
			else
			{
				// couldn't find first token, fail
				gi.dprintf("Error loading map file: %s\n", filename);
				maplist->nummaps = 0;
				fclose(fptr);
				return 0;
			}

			// find next token
			if ((s = strtok(NULL, ",")) != NULL)	//s is zero-terminated by strtok
			{
				// copy monster value to list
				maplist->maps[iterator].monsters = atoi(s);
			}

			++iterator;
		}
		maplist->nummaps = iterator;
		if (iterator != 0)
			gi.dprintf("INFO: Success loading %s maps: %i\n", filename, maplist->nummaps);
		else
			gi.dprintf("Error loading map file: %s maps: %i\n", filename, maplist->nummaps);
		fclose(fptr);
	}
	return maplist->nummaps;
}
