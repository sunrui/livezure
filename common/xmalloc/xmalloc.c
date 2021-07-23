#include "../xmalloc.h"

#ifdef USE_XMALLOC
#include "imembase.h"
#include <assert.h>

void xmalloc_init()
{
	ikmem_init(0, 0, NULL);
}

void xmalloc_destroy()
{
	ikmem_destroy();
}

void * xmalloc(int size)
{
	assert(size > 0);
	return ikmem_malloc(size);
}

void * xcalloc(int count, int size)
{
	char * ptr;
	int n;

	n = count * size;
	assert(n > 0);
	ptr = (char *)ikmem_malloc(n);
	memset(ptr, 0, n);

	return ptr;
}

void xfree(void * ptr)
{
	assert(ptr != NULL);
	ikmem_free(ptr);
}

#endif