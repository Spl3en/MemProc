#include "MemProc.h"

MemProc *
memproc_new (char *process_name, char *window_name)
{
	MemProc *mp;

	if ((mp = calloc (sizeof (MemProc), 1)) == NULL)
		return NULL;

	mp->memchunks = NULL;
	mp->stype = SEARCH_TYPE_BYTES;
	mp->pid = 0;
	mp->process_name = process_name;
	mp->window_name = window_name;
	mp->base_addr = 0;
	mp->default_baseaddr = 0;

	return mp;
}

int
memproc_is_dumped (MemProc *mp)
{
	return mp->memchunks != NULL;
}

void
memproc_dump_details (MemProc *mp, int start, int end, int (*boolean_function) (MEMORY_BASIC_INFORMATION *, void *), void *arg)
{
	MEMORY_BASIC_INFORMATION meminfo;
	int addr = start;

	if (!mp->proc)
	{
		warning ("Process pid=%d not found", mp->pid);
		return;
	}

	mp->memchunks = bb_queue_new ();

	while (1)
	{
		if (addr >= end && end != -1)
			break;

		if (VirtualQueryEx (mp->proc, (void *) addr, &meminfo, sizeof (meminfo)) == 0)
			break;

		if (boolean_function (&meminfo, arg))
		{
			MemChunk *mc = memchunk_new (mp->proc, &meminfo);

			switch (meminfo.Type)
			{
				case MEM_IMAGE:		mc->type = MEM_TYPE_IMAGE;   break;
				case MEM_MAPPED:	mc->type = MEM_TYPE_MAPPED;  break;
				case MEM_PRIVATE:	mc->type = MEM_TYPE_PRIVATE; break;
			}

			console_set_col (0x07);
			bb_queue_add (mp->memchunks, mc);
		}

		addr = ((unsigned int) meminfo.BaseAddress + (unsigned int) meminfo.RegionSize);

	}
}

static int
memproc_dump_helper (MEMORY_BASIC_INFORMATION *meminfo, void *arg)
{
    (void) meminfo;
    (void) arg;

	return 1;
}

void
memproc_dump (MemProc *mp, int start, int end)
{
	memproc_dump_details (mp, start, end, memproc_dump_helper, NULL);
}

void
memproc_set_default_baseaddr (MemProc *mp, int default_baseaddr)
{
	mp->default_baseaddr = default_baseaddr;
}

bool
memproc_refresh_handle (MemProc *mp)
{
	if (mp == NULL)
		return false;

	// Get the PID
	if ((mp->pid = get_pid_by_name (mp->process_name)) == 0)
	{
		// Process not active
		mp->proc = NULL;
		return false;
	}

	// Get the process handle
	if ((mp->proc = OpenProcess (PROCESS_ALL_ACCESS, FALSE, mp->pid)) == 0)
	{
		warning ("Process is unable to be opened with all access.");
		return false;
	}

	// Get the base address
	if ((mp->base_addr = get_baseaddr (mp->process_name)) == 0)
	{
		info ("Base address of the process %s not found. Using the default value 0x%x.", mp->process_name, mp->default_baseaddr);
		mp->base_addr  = mp->default_baseaddr;
	}

	// Get the window handle
	if (mp->window_name != NULL)
	{
		mp->hwnd = get_hwnd_from_title (mp->window_name);
	}

	return true;
}

void
memproc_debug (MemProc *mp)
{
	bb_queue_debug_custom_data (mp->memchunks, memchunk_debug);
}

void
memproc_full_debug (MemProc *mp)
{
	bb_queue_debug_custom_data (mp->memchunks, memchunk_full_debug);
}

void
memproc_search_float (MemProc *mp, float value, void (*pre_search) (MemChunk *, float prct))
{
	char *mask = "xxxx";
	unsigned char pattern[4] = {0, 0, 0, 0};
	memcpy (pattern, &value, 4);

	memproc_search (mp, pattern, mask, pre_search, SEARCH_TYPE_FLOAT);
}

void
memproc_search_integer (MemProc *mp, int value, void (*pre_search) (MemChunk *, float prct))
{
	char *mask = "xxxx";
	unsigned char pattern[4] = {0, 0, 0, 0};
	memcpy (pattern, &value, 4);

	memproc_search (mp, pattern, mask, pre_search, SEARCH_TYPE_INTEGER);
}

void
memproc_search_text (MemProc *mp, char *text, char *mask, void (*pre_search) (MemChunk *, float prct))
{
	int len = strlen (text);

	if (mask == NULL)
	{
		mask = malloc (len + 1);
		memset (mask, 'x', len);
		mask[len] = '\0';
	}

	memproc_search (mp, (unsigned char *) text, mask, pre_search, SEARCH_TYPE_STRING);
}

void
memproc_update (MemProc *mp, BbQueue *memblocks)
{
	MemBlock *mb;

	foreach_bbqueue_item (memblocks, mb)
	{
		memblock_read_from_memory (mp, mb);
		// TODO : impact de l'update dans le buffer des mc
	}
}

BbQueue *
memblock_get_change (MemProc *mp, BbQueue *res)
{
	// TODO
	(void) mp;
	(void) res;

	return NULL;
}

void
memproc_search (MemProc *mp, unsigned char *pattern, char *mask, void (*pre_search) (MemChunk *, float prct), SearchType stype)
{
	MemChunk *mc;
	int offset;
	int total;
	int loop = 1;

	if (mp->memchunks == NULL)
	{
		warning ("No memchunks stored, you must call %s () first", str_make_macro (memproc_dump));
		return;
	}

	mp->mask_len = strlen (mask);

	foreach_bbqueue_item (mp->memchunks, mc)
	{
		offset = 0;
		total = 0;

		if (pre_search != NULL)
			pre_search (mc, (float) loop / (float) bb_queue_get_length (mp->memchunks));

		do
		{
			if ((offset = find_pattern (mc->buffer + total, mc->size - total, pattern, mask)) != -1)
			{
				bb_queue_add_raw (mc->matches, total + offset);
				total += offset + strlen (mask);
			}

		} while (offset != -1);

		loop++;
	}

	mp->stype = stype;
}

BbQueue *
memproc_get_res (MemProc *mp)
{
	int offset;
	void *data = NULL;
	MemBlock *b;
	MemChunk *mc;
	BbQueue *q = bb_queue_new ();

	foreach_bbqueue_item (mp->memchunks, mc)
	{
		while (bb_queue_get_length (mc->matches))
		{
			offset = (int) bb_queue_pop (mc->matches);

			switch (mp->stype)
			{
				case SEARCH_TYPE_BYTES:
				case SEARCH_TYPE_FLOAT:
				case SEARCH_TYPE_INTEGER:
					data = malloc (mp->mask_len);
				break;

				case SEARCH_TYPE_STRING:
					data = malloc (mp->mask_len + 1);
					 ((unsigned char *) data) [mp->mask_len] = '\0';
				break;
			}

			memcpy (data, &mc->buffer[(int) offset], mp->mask_len);
			b = memblock_new (data, mc->addr + offset, mp->mask_len, mc->type);
			bb_queue_add (q, b);
		}
	}

	return q;
}

MemBlock *
memblock_new (void *data, DWORD addr, int size, MemType type)
{
	MemBlock *r;

	if ((r = malloc (sizeof (MemBlock)) ) == NULL)
		return NULL;

	r->data = data;
	r->addr = addr;
	r->size = size;
	r->type = type;

	return r;
}

void
memproc_set_absolute_addr (MemProc *mp, DWORD *addr)
{
	 (*addr) = (*addr) + mp->base_addr;
};

inline void
memblock_read_from_memory (MemProc *mp, MemBlock *mem)
{
	read_from_memory (mp->proc, mem->data, mem->addr, mem->size);
}

void
memproc_free (MemProc *memproc)
{
	if (memproc != NULL)
	{
		bb_queue_free_all (memproc->memchunks, memchunk_free);
		free (memproc);
	}
}

void
memproc_clear (MemProc *memproc)
{
	if (memproc != NULL)
	{
		bb_queue_free_all (memproc->memchunks, memchunk_free);
	}
}

void
memblock_free (MemBlock *m)
{
	if (m != NULL)
	{
		free (m);
	}
}
