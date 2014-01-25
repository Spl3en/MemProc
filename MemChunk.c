#include "MemChunk.h"

MemChunk *
memchunk_new (HANDLE hProc, MEMORY_BASIC_INFORMATION *meminfo)
{
	MemChunk *mem;

	if ((mem = malloc(sizeof(MemChunk))) == NULL)
		return NULL;

	mem->proc = hProc;

	mem->addr = (DWORD) meminfo->BaseAddress;
	mem->size = meminfo->RegionSize;

	mem->buffer =  malloc (meminfo->RegionSize);
	memset(mem->buffer, 0, meminfo->RegionSize);

	mem->matches = bb_queue_new();

	memchunk_read_from_memory(mem);

	return mem;
}

inline void
memchunk_read_from_memory (MemChunk *mem)
{
	read_from_memory(mem->proc, mem->buffer, mem->addr, mem->size);
}

void
memchunk_debug (MemChunk *mc)
{
	printf("Addr : 0x%.8x - 0x%.8x (%d bytes) \n", (int) mc->addr, (int) mc->addr + mc->size, mc->size);
}

void
memchunk_full_debug (MemChunk *mc)
{
	memchunk_debug(mc);

	for (unsigned int i = 0; i < mc->size; i++)
		printf("0x%.2x ", mc->buffer[i]);

	printf("\n");
}


void
memchunk_free (MemChunk *memchunk)
{
	if (memchunk != NULL)
	{
		free (memchunk);
	}
}
