/*
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

*/

#ifndef MYSQL_GDS
#define MYSQL_GDS

/* 
So back then there was a GDS 
and it had issues.

But now, we're doing it Right!<tm>

So, with a few pointers from KOTS2007
(ideas borrowed from kots. heheh)
we're having a MYSQL GDS.

						-az

(this will also work with qfusion)
*/
#ifndef NO_GDS

#define GDS_LOAD 1
#define GDS_SAVE 2
#define GDS_SAVECLOSE 3
#define GDS_SAVERUNES 4
#define GDS_EXITTHREAD 5

// For Everyone
// void V_GDS_Load(edict_t *ent);
qboolean V_GDS_StartConn();
void V_GDS_Queue_Add(edict_t *ent, int operation);
qboolean CanUseGDS();
#ifndef GDS_NOMULTITHREADING
void GDS_FinishThread();
void HandleStatus(edict_t *player);
#endif

void Mem_PrepareMutexes();

#endif //NO_GDS

// Wrapped, thread save mem allocation.
void *V_Malloc(size_t Size, int Tag);
void V_Free (void* mem);

#endif // MYSQL_GDS