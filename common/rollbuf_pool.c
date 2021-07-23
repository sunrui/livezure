//
// brief memory pool wrapper, use for async buffer io
// copyright 2012 rui.sun
// created 2012-1-7
// update note:
//      2012-1-7, created, rui.sun
//
#include "rollbuf_pool.h"
#include "posixtime.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

typedef struct _rollbuf_pool_state_t
{
	unsigned char * buffer_pool;
	int buffer_size;

	int pool_size;

	pthread_mutex_t access_lock;
	pthread_cond_t access_singal;

	int finish_singal;
	int stop_signal;

	rollbuf_client_t * cb;
} rollbuf_pool_state;

rollbuf_pool_t rollbuf_pool_create(rollbuf_client_t * client, int pool_size)
{
	rollbuf_pool_state * state = (rollbuf_pool_state *)malloc(sizeof(rollbuf_pool_state));

	pthread_mutex_init(&state->access_lock, NULL);
	pthread_cond_init(&state->access_singal, NULL);

	if (pool_size <= 0)
	{
		pool_size = ROLLBUF_POOL_MAX_BUF_DEF_SIZE;
	}

	state->buffer_pool = (unsigned char *)malloc(pool_size);
	state->pool_size = pool_size;
	state->buffer_size = 0;
	state->finish_singal = 0;
	state->stop_signal = 0;
	state->cb = NULL;

	if (client != NULL)
	{
		state->cb = (rollbuf_client_t *)malloc(sizeof(rollbuf_client_t));
		state->cb->on_buffering_cb = client->on_buffering_cb;
		state->cb->on_buffering_ok_cb = client->on_buffering_ok_cb;
		state->cb->opaque = client->opaque;
	}

	return (rollbuf_pool_t)state;
}

bool rollbuf_pool_push(rollbuf_pool_t pool, const unsigned char * data, int size)
{
	rollbuf_pool_state * state = (rollbuf_pool_state *)pool;

	int max_sleep_time_ms = ROLLBUF_POOL_MAX_BUF_THROW_TIMEOUT;
	int every_sleep_time_ms = ROLLBUF_POOL_MAX_BUF_EVERY_WAIT_TIME;

	if (state == NULL)
	{
		return false;
	}

	if (state->stop_signal)
	{
		pthread_mutex_unlock(&state->access_lock);
		return false;
	}

	while (state->buffer_size + size >= state->pool_size)
	{
		if (max_sleep_time_ms > 0)
		{
			usleep(every_sleep_time_ms);
			max_sleep_time_ms -= every_sleep_time_ms;
		}
		else
		{
			return false;
		}
	}

	pthread_mutex_lock(&state->access_lock);
	memcpy(state->buffer_pool + state->buffer_size, data, size);
	state->buffer_size += size;
	pthread_cond_signal(&state->access_singal);
	pthread_mutex_unlock(&state->access_lock);

	return true;
}

int rollbuf_pool_get(rollbuf_pool_t pool, unsigned char ** needbuffer, int need_size, int * result_size)
{
	rollbuf_pool_state * state = (rollbuf_pool_state *)pool;
	bool b_buffering = false;

	if (state == NULL)
	{
		if (result_size != NULL)
		{
			*result_size = 0;
		}
		return -1;
	}

	pthread_mutex_lock(&state->access_lock);
	while (need_size > state->buffer_size)
	{
		if (state->stop_signal)
		{
			if (result_size != NULL)
			{
				*result_size = 0;
			}

			if (b_buffering && state->cb != NULL && state->cb->on_buffering_ok_cb != NULL)
			{
				state->cb->on_buffering_ok_cb(state->cb->opaque);
			}

			pthread_mutex_unlock(&state->access_lock);
			return -1;
		}

		// put buffer task is finished
		if (state->finish_singal)
		{
			memcpy(*needbuffer, state->buffer_pool, state->buffer_size);
			if (result_size != NULL)
			{
				*result_size = state->buffer_size;
			}
			
			if (b_buffering && state->cb != NULL && state->cb->on_buffering_ok_cb != NULL)
			{
				state->cb->on_buffering_ok_cb(state->cb->opaque);
			}

			state->buffer_size -= state->buffer_size;
			pthread_mutex_unlock(&state->access_lock);
			return 0;
		}

		if (state->cb != NULL && state->cb->on_buffering_cb != NULL)
		{
			state->cb->on_buffering_cb(state->cb->opaque, 0);
		}

		b_buffering = true;
		pthread_cond_wait(&state->access_singal, &state->access_lock);
	}

	if (b_buffering && state->cb != NULL && state->cb->on_buffering_ok_cb != NULL)
	{
		state->cb->on_buffering_ok_cb(state->cb->opaque);
	}

	if (state->stop_signal)
	{
		if (result_size != NULL)
		{
			*result_size = 0;
		}

		pthread_mutex_unlock(&state->access_lock);
		return -1;
	}

	memcpy(*needbuffer, state->buffer_pool, need_size);
	if (result_size != NULL)
	{
		*result_size = need_size;
	}

	state->buffer_size -= need_size;
	memmove(state->buffer_pool, state->buffer_pool + need_size, state->buffer_size);
	pthread_mutex_unlock(&state->access_lock);

	return 1;
}

bool rollbuf_pool_finish(rollbuf_pool_t pool)
{
	rollbuf_pool_state * state = (rollbuf_pool_state *)pool;
	state->finish_singal = 1;
	pthread_cond_signal(&state->access_singal);

	return true;
}

bool rollbuf_pool_stop(rollbuf_pool_t pool)
{
	rollbuf_pool_state * state = (rollbuf_pool_state *)pool;
	state->stop_signal = 1;
	pthread_cond_signal(&state->access_singal);

	return true;
}

void rollbuf_pool_destroy(rollbuf_pool_t * pool)
{
	rollbuf_pool_state * state;

	if (pool == NULL || *pool == NULL)
	{
		assert(false);
		return;
	}

	state = (rollbuf_pool_state *)*pool;

	if (!state->stop_signal || !state->finish_singal)
	{
		rollbuf_pool_stop(*pool);
		rollbuf_pool_finish(*pool);
	}

	pthread_mutex_lock(&state->access_lock);

	if (state->cb != NULL)
	{
		free(state->cb);
	}

	free(state->buffer_pool);

	pthread_mutex_destroy(&state->access_lock);
	pthread_cond_destroy(&state->access_singal);

	free(state);
	*pool = NULL;
}
