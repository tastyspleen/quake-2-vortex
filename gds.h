#include <sys/stat.h>

#define GDS_FILE_EXISTS			1		// the file we want exists
#define GDS_NULLFILE_EXISTS		2		// a NULL file exists

#define GDS_TIMEOUT				30		// seconds before GDS transaction times out
#define GDS_TIMEOUT_RETRY		60		// seconds before we allow another GDS attempt
#define GDS_CLEANUP_DELAY		0.5		// seconds before temp file cleanup

void GDS_FileIO (const char *filename, int cmd);
void GDS_LoadPlayer (edict_t *ent);
void GDS_SavePlayer (edict_t *ent);
void GDS_CheckPlayer (edict_t *ent);
void GDS_DeleteCharFile (edict_t *ent);
void GDS_CheckFiles (void);
void GDS_GetTestFile (void);
int GDS_FileExists (char *filename, qboolean remove_file);

typedef struct gdsfiles_s
{
	char		filename[100];	// file name to delete
	float		time;			// time to delete the file
}gdsfiles_t;