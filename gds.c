#include "g_local.h"

#if (0) /* disables code */
#define GDS_TESTFILE_NAME	"Player"

qboolean GDS_InFileQue (edict_t *ent)
{
	int i;

	for (i=0; i<MAX_CLIENTS; i++)
	{
		// return true if the file name is already in the list, and has a valid time
		if (!strcmp(V_FormatFileName(ent->client->pers.netname), level.gdsfiles[i].filename)
			&& (level.gdsfiles[i].time > level.time))
			return true;
	}
	return false;
}

gdsfiles_t *GDS_FindEmptyQue (void)
{
	int	i;

	for (i=0; i<MAX_CLIENTS; i++)
	{
		if (!level.gdsfiles[i].time)
			return &level.gdsfiles[i];
	}
	return NULL;
}

void GDS_CheckFiles (void)
{
	int			i;
	char		buf[100];
	gdsfiles_t *slot;

	for (i=0; i<MAX_CLIENTS; i++)
	{
		slot = &level.gdsfiles[i];
		if (slot->time && (level.time > slot->time))
		{
			// delete the file
			sprintf(buf, "%s/\"%s.vrx\"", save_path->string, slot->filename);
			system(va("del %s\n", buf));
			// remove it from the que
			slot->time = 0;
			strcpy(slot->filename, "");
		}
	}
}

void GDS_DeleteCharFile (edict_t *ent)
{
	gdsfiles_t *slot;

	if (GDS_InFileQue(ent))
		return; // already in the list

	// mark this player's character file for deletion
	if ((slot = GDS_FindEmptyQue()) != NULL)
	{
		strcpy(slot->filename, V_FormatFileName(ent->client->pers.netname));
		slot->time = level.time + GDS_CLEANUP_DELAY;
	}
}

void GDS_FileIO (const char *filename, int cmd)
{
	char				buf1[100], buf2[100];
	HINSTANCE			hRet;

	sprintf(buf1, "%s/gds.exe", gds_exe->string);

	if (cmd)
		sprintf(buf2, "\"%s\" %s %s", filename, save_path->string, "1");
	else
		sprintf(buf2, "\"%s\" %s %s", filename, save_path->string, "0");

	// launch process
	hRet = ShellExecute(HWND_DESKTOP, "open", buf1, buf2, NULL, SW_HIDE);
	// check for error codes
	if ((int)hRet <= 32)
	{
		printf("ERROR: Unable to initialize GDS subsystem.\n");
		switch ((int)hRet)
		{
		case ERROR_FILE_NOT_FOUND: printf("GDS: File not found\n"); break;
		case ERROR_PATH_NOT_FOUND: printf("GDS: Path not found\n"); break;
		case ERROR_BAD_FORMAT: printf("GDS: Bad format\n"); break;
		case SE_ERR_ACCESSDENIED: printf("GDS: Access denied\n"); break;
		}
	}
}

qboolean ValidPlayerName (char *name)
{
	// these characters are not compatible with the GDS
	return (!strchr(name, '%'));// && !strchr(name, '^') && !strchr(name, '&'));
}

void GDS_LoadPlayer (edict_t *ent)
{
	if (!ValidPlayerName(ent->client->pers.netname))
	{
		// certain characters have special meaning with Windows programs, and cannot be used
		// FIXME: update V_FormatFileName() with these invalid characters!
		gi.cprintf(ent, PRINT_HIGH, "Name contains invalid character(s). Please change your name.\n");
		return;
	}

	if (ent->client->resp.gds_downloading || ent->client->resp.gds_testing)
	{
		gi.cprintf(ent, PRINT_HIGH, "Please wait for GDS.\n");
		return;
	}

	gi.cprintf(ent, PRINT_HIGH, "Testing GDS connection...\n");
	GDS_GetTestFile();
	ent->client->resp.gds_testing = true;
	ent->client->resp.gds_timeout = level.time + GDS_TIMEOUT;
}

void GDS_GetPlayerFile (edict_t *ent)
{
	// delete any local save file, we only want to load the file the GDS gives us
	//gi.dprintf("INFO: Downloading %s\n", V_FormatFileName(ent->client->pers.netname));
	GDS_FileExists(V_FormatFileName(ent->client->pers.netname), true);

	gi.cprintf(ent, PRINT_HIGH, "Loading character file from GDS...\n");
	GDS_FileIO(V_FormatFileName(ent->client->pers.netname), 1);
	ent->client->resp.gds_downloading = true;
}

void GDS_SavePlayer (edict_t *ent)
{
	gi.dprintf("INFO: Initiating file transfer via GDS subsystem.\n");
	GDS_FileIO(V_FormatFileName(ent->client->pers.netname), 0);
}

void GDS_GetTestFile (void)
{
	// delete any existing test file
	GDS_FileExists(GDS_TESTFILE_NAME, true);

	// begin download of test file
	GDS_FileIO(GDS_TESTFILE_NAME, 1);
}

int GDS_FileExists (char *filename, qboolean remove_file)
{
	char		buf[100];
	struct stat	file;

	// did the file we want download successfully?
	sprintf(buf, "%s/%s.vrx", save_path->string, filename);
	if(!stat(buf, &file) && (file.st_size > 1))
	{
		gi.dprintf("INFO: GDS successfully downloaded %s\n", buf);

		if (remove_file)
		{
			gi.dprintf("INFO: Deleting temporary file %s\n", buf);

			sprintf(buf, "del %s/\"%s.vrx\"", save_path->string, filename);
			system(buf);
		}

		return GDS_FILE_EXISTS;
	}

	// did the GDS create a NULL file?
	sprintf(buf, "%s/%s.NULL", save_path->string, filename);
	if (!stat(buf, &file))
	{
		gi.dprintf("INFO: GDS created NULL file %s\n", buf);

		// always delete the NULL file
		sprintf(buf, "del %s/\"%s.NULL\"", save_path->string, filename);
		system(buf);

		return GDS_NULLFILE_EXISTS;
	}

	return 0; // didn't find anything yet
}

void GDS_CheckPlayer (edict_t *ent)
{
	int		value;

	if (ent->client->resp.gds_complete)
		return;

	if (ent->client->resp.gds_testing)
	{
		if (level.time > ent->client->resp.gds_timeout)
		{
			gi.cprintf(ent, PRINT_HIGH, "Unable to connect to GDS. Try again later.\n");
			gi.dprintf("WARNING: GDS download timed out\n");

			// let them try again later
			ent->client->resp.gds_downloading = false;
			ent->client->resp.gds_testing = false;
			//ent->client->resp.gds_timeout = level.time + GDS_TIMEOUT_RETRY;
			return;
		}

		// if the test file exists, then we know we successfully connected to the GDS FTP
		if (GDS_FileExists(GDS_TESTFILE_NAME, true) == GDS_FILE_EXISTS)
		{
			gi.dprintf("INFO: GDS connection established.\n");
			// we can begin downloading the player file
			ent->client->resp.gds_testing = false;
			GDS_GetPlayerFile(ent);
		}
		return;
	}

	if (!ent->client->resp.gds_downloading)
		return;

	if (value = GDS_FileExists(V_FormatFileName(ent->client->pers.netname), false))
	{
		if (value == GDS_NULLFILE_EXISTS)
			gi.dprintf("INFO: File doesn't exist on GDS.\n");
		else
			gi.dprintf("INFO: File successfully downloaded from GDS.\n");

		// we're done downloading
		ent->client->resp.gds_downloading = false;
		ent->client->resp.gds_complete = true;
		
		JoinTheGame(ent);
		return;
	}
}
#endif

int GDS_OpenConfigFile(edict_t *ent)
{
	if(ent->threadReturnVal != 0)
		newPlayer(ent);
	
	return canJoinGame(ent);
}

void GDS_CheckPlayer(edict_t *ent)
{
	if(!ent->hThreadFinishTime)
		return;

#if defined(_WIN32) || defined(WIN32)
	if(ent->isLoading && ent->client && ent->client->pers.connected)
	{
		ent->isLoading = false;
		JoinTheGame(ent);
	}
	else if(ent->isSaving)
	{
        ent->isSaving = false;

		//Reset the player if they disconnected.
		if(ent->client && !ent->client->pers.connected)
			memset(&ent->myskills, 0, sizeof(skills_t));
	}

	ent->hThread = 0;
	ent->threadReturnVal = 0;
	ent->hThreadFinishTime = 0.0f;
#endif
}

