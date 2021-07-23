/*
 * rpc memory provider
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2013-1-28 15:51
 */
#include "rpc_alloctor.h"
#include "rpc_buffer.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#ifdef WIN32
#ifndef inline
#define inline __inline
#endif
#endif

static rpc_alloctor_t rpc_default_io;
static int rpc_malloc_inited = 0;

inline void * internal_malloc(int size)
{
	assert(size > 0);
	return malloc(size);
}

inline void internal_free(void * ptr)
{
	assert(ptr != NULL);
	free(ptr);
}

/*
 * register custom buffer allocator
 */
void rpc_buffer_allocator(rpc_alloctor_t * io)
{
	rpc_default_io.alloc = io->alloc;
	rpc_default_io.free = io->free;

	rpc_malloc_inited = 1;
}

/*
 * rpc allocator init
 */
void rpc_alloctor_init()
{
	if (!rpc_malloc_inited)
	{
		if (rpc_default_io.alloc == NULL)
		{
			rpc_default_io.alloc = internal_malloc;
		}

		if (rpc_default_io.free == NULL)
		{
			rpc_default_io.free = internal_free;
		}
		rpc_malloc_inited = 1;
	}
}

/*
 * rpc allocator destroy
 */
void rpc_alloctor_destroy()
{
	if (rpc_malloc_inited)
	{
		rpc_malloc_inited = 0;
	}
}

void * rpc_malloc(int size)
{
	assert(size > 0);

	return rpc_default_io.alloc(size);
}

void * rpc_calloc(int count, int size)
{
	int memsize;
	void * mem;

	assert(count > 0 && size > 0);
	memsize = count * size;
	mem = rpc_default_io.alloc(memsize);
	memset(mem, 0, memsize);

	return mem;
}

void rpc_free(void * ptr)
{
	if (ptr == NULL)
	{
		assert(0);
		return;
	}

	rpc_default_io.free(ptr);
}