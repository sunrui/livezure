//
// brief memory pool wrapper, use for async buffer io
// copyright 2012 rui.sun
// created 2012-1-8
// update note:
//      2012-1-8, created, rui.sun
//
#ifndef ROLLBUF_POOL_H
#define ROLLBUF_POOL_H

#include <stdbool.h>

#define ROLLBUF_POOL_MAX_BUF_DEF_SIZE          5 * 1024 * 1024 // 5M
#define ROLLBUF_POOL_MAX_BUF_THROW_TIMEOUT     500 // 500ms
#define ROLLBUF_POOL_MAX_BUF_EVERY_WAIT_TIME   100 // 100ms

typedef int (* pfn_on_rollbuf_buffering)(void * opaque, int percent);
typedef int (* pfn_on_rollbuf_buffering_ok)(void * opaque);

typedef struct rollbuf_client_t
{
	pfn_on_rollbuf_buffering on_buffering_cb;
	pfn_on_rollbuf_buffering_ok on_buffering_ok_cb;

	void * opaque;
} rollbuf_client_t;

typedef struct rollbuf_pool_state * rollbuf_pool_t;

rollbuf_pool_t rollbuf_pool_create(rollbuf_client_t * client, int pool_size); /* def pool size is 5M if no specified */
bool rollbuf_pool_push(rollbuf_pool_t pool, const unsigned char * data, int size);
int  rollbuf_pool_get(rollbuf_pool_t pool, unsigned char ** needbuffer, int need_size, int * result_size);
bool rollbuf_pool_finish(rollbuf_pool_t pool);
bool rollbuf_pool_stop(rollbuf_pool_t pool);
void rollbuf_pool_destroy(rollbuf_pool_t * pool);

#endif