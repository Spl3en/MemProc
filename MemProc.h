// --- Author	: Moreau Cyril - Spl3en - Credits to gimmeamilk  (http://www.youtube.com/watch?v=YRPMdb1YMS8)
// --- File		: MemProc.h
// --- Date		: 2013-03-04-22.14.43
// --- Version	: 1.0

#ifndef MemProc_H_INCLUDED
#define MemProc_H_INCLUDED


// ---------- Includes ------------
#include <stdlib.h>
#include "MemChunk.h"
#include "../BbQueue/BbQueue.h"
#include "../Win32Tools/Win32Tools.h"

#ifdef MEMPROC_EXPORT_FUNCTION
	#ifdef WIN32
		#ifdef EXPORT_FUNCTION
			#undef EXPORT_FUNCTION
		#endif
		#define EXPORT_FUNCTION __declspec(dllexport)
	#endif
#endif

#ifndef EXPORT_FUNCTION
	#define EXPORT_FUNCTION
#endif

// ---------- Defines -------------


// ------ Struct declaration -------
typedef enum {

	SEARCH_TYPE_BYTES = -1,
	SEARCH_TYPE_INTEGER,
	SEARCH_TYPE_FLOAT,
	SEARCH_TYPE_STRING,

} SearchType;


typedef
struct _MemProc
{
	BbQueue *memchunks;
	HANDLE proc;
	HWND hwnd;
	SearchType stype;
	int mask_len;

	int pid;
	char *window_name;
	char *process_name;

	DWORD base_addr;
	DWORD default_baseaddr;

}	MemProc;


typedef struct _MemBlock
{
	void *data;
	DWORD addr;
	int size;
	MemType type;

	char to_update;

}	MemBlock;

typedef struct ImageSectionInfo
{
	  char SectionName[IMAGE_SIZEOF_SHORT_NAME];
	  char *SectionAddress;
	  int SectionSize;

}	ImageSectionInfo;

// --------- Constructors ---------

MemProc *
memproc_new (char *process_name, char *window_name);


MemBlock *
memblock_new (void *data, DWORD addr, int size, MemType type);

// ----------- Functions ------------

void
memproc_debug (MemProc *mp);

void
memproc_full_debug (MemProc *mp);

void
memproc_search (MemProc *mp, unsigned char *pattern, char *mask, void (*pre_search)(MemChunk *, float prct), SearchType stype);

void
memproc_update (MemProc *mp, BbQueue *memblocks);

void
memproc_dump (MemProc *mp, int start, int end);

void
memproc_dump_details (MemProc *mp, int start, int end, int nbSections, int (*boolean_function) (MEMORY_BASIC_INFORMATION *, void *), void *arg);

int
memproc_is_dumped (MemProc *mp);

bool
memproc_refresh_handle (MemProc *mp);

BbQueue *
memproc_get_res (MemProc *mp);

void
memproc_search_float (MemProc *mp, float value, void (*pre_search)(MemChunk *, float prct));

void
memproc_search_integer (MemProc *mp, int value, void (*pre_search)(MemChunk *, float prct));

void
memproc_search_text (MemProc *mp, char *text, char *mask, void (*pre_search)(MemChunk *, float prct));

void
memproc_search_changed (MemProc *mp, void (*pre_search)(MemChunk *, float prct));

void
memblock_read_from_memory (MemProc *mp, MemBlock *mem);

void
memproc_set_absolute_addr (MemProc *mp, DWORD *addr);

void
memproc_set_default_baseaddr (MemProc *mp, int default_baseaddr);

void
memblock_debug (MemBlock *mb);

bool
memproc_detected (MemProc *mp);

// --------- Destructors ----------

void
memproc_clear (MemProc *memproc);

void
memproc_free (MemProc *memproc);


void
memblock_free (MemBlock *m);




#endif // MemProc_INCLUDED
