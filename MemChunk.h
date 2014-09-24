// --- Author	: Moreau Cyril - Spl3en
// --- File		: MemChunk.h
// --- Date		: 2013-03-04-22.10.17
// --- Version	: 1.0

#ifndef MemChunk_H_INCLUDED
#define MemChunk_H_INCLUDED


// ---------- Includes ------------
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include "../BbQueue/BbQueue.h"
#include "../Win32Tools/Win32Tools.h"

// ---------- Defines -------------

typedef enum {

	MEM_TYPE_IMAGE = 0,
	MEM_TYPE_MAPPED = 1,
	MEM_TYPE_PRIVATE = 2,

} MemType;

// ------ Struct declaration -------
typedef
struct _MemChunk
{
	HANDLE proc;
	DWORD  addr;

	unsigned char *buffer;
	unsigned int size;
	MemType type;

	BbQueue *matches;

}	MemChunk;



// --------- Constructors ---------

MemChunk *
memchunk_new (HANDLE hProc, MEMORY_BASIC_INFORMATION *meminfo);

void
memchunk_init (MemChunk *mem, HANDLE hProc, MEMORY_BASIC_INFORMATION *meminfo);


// ----------- Functions ------------

void
memchunk_read_from_memory (MemChunk *mem);

void
memchunk_set_update (MemChunk *mem, char to_update);


void
memchunk_debug (MemChunk *mc);

void
memchunk_full_debug (MemChunk *mc);



// --------- Destructors ----------

void
memchunk_free (MemChunk *memchunk);






#endif // MemChunk_INCLUDED
