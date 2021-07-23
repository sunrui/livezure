/*
 * rpc send queue, use for maintenance send/recv pools
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2012-7-31 12:29
 */
#include "rpc_queue.h"
#include "rpc_alloctor.h"

#include "posixinc.h"

#include <pthread.h>

typedef struct rpc_queue_body_t
{
	void * body; /* need send/recv rpc buffers */
	TAILQ_ENTRY(rpc_queue_body_t) entries;
} rpc_queue_body_t;

struct rpc_queue
{
	TAILQ_HEAD(, rpc_queue_body_t) rpc_queue; /* unique rpc event queue instance */
	pthread_cond_t rpc_queue_cond; /* use for rpc queue thread-safe */
	pthread_mutex_t rpc_queue_mutex; /* use for rpc queue thread-safe */
	int rpc_queue_stop_sign; /* whether received a signal to stop */
};

/*
 * create a new rpc queue
 */
rpc_queue_t * rpc_queue_new()
{
	rpc_queue_t * rq;

	rq = (rpc_queue_t *)rpc_calloc(1, sizeof(rpc_queue_t));
	pthread_mutex_init(&rq->rpc_queue_mutex, NULL);
	pthread_cond_init(&rq->rpc_queue_cond, NULL);
	rq->rpc_queue_stop_sign = 0;
	TAILQ_INIT(&rq->rpc_queue);

	return rq;
}

/*
 * push a new rpc buffer to queue, must be alloc here
 */
void rpc_queue_push(rpc_queue_t * rq, void * body)
{
	rpc_queue_body_t * item;

	item = (rpc_queue_body_t *)rpc_calloc(1, sizeof(rpc_queue_body_t));
	item->body = body;

	pthread_mutex_lock(&rq->rpc_queue_mutex);
	TAILQ_INSERT_TAIL(&rq->rpc_queue, item, entries);
	pthread_cond_signal(&rq->rpc_queue_cond);
	pthread_mutex_unlock(&rq->rpc_queue_mutex);
}

/*
 * get a body from queue
 *
 * @body	return body get ok, you must manual free it.
 *
 * @timeout	send timeout in milliseconds or < 0 if infinite
 *
 * @return
 *     1 get it ok
 *     0 no more buffers now (timeout reached)
 *    -1 by your abort
 */
int rpc_queue_get(rpc_queue_t * rq, void ** body, int timeout)
{
	rpc_queue_body_t * item;
	int ret = 0;

	assert(body != NULL);

	pthread_mutex_lock(&rq->rpc_queue_mutex);

	*body = NULL;
	item = TAILQ_FIRST(&rq->rpc_queue);
	while (item == NULL && !rq->rpc_queue_stop_sign)
	{
		int ret_status;

		if (timeout > 0)
		{
			struct timespec ts;
			struct timeval now;

			/* get the current time */
			gettimeofday(&now, NULL); /* add the offset to get timeout value */ 
			ts.tv_nsec = now.tv_usec * 1000 + (timeout % 1000) * 1000000; 
			ts.tv_sec = now.tv_sec + timeout / 1000;

			ret_status = pthread_cond_timedwait(&rq->rpc_queue_cond, &rq->rpc_queue_mutex, &ts);
		}
		else if (timeout == 0)
		{
			*body = NULL;
			ret = 0;
			goto ret_clean;
		}
		else
		{
			ret_status = pthread_cond_wait(&rq->rpc_queue_cond, &rq->rpc_queue_mutex);
		}

		/* if waked by user stop sign */
		if (rq->rpc_queue_stop_sign)
		{
			*body = NULL;
			ret = -1;
			goto ret_clean;
		}

		item = TAILQ_FIRST(&rq->rpc_queue);
		if (item == NULL)
		{
			/* timeout reached */
			if (ret_status == ETIMEDOUT)
			{
				*body = NULL;
				ret = 0;
				goto ret_clean;
			}
			else
			/* by user abort */
			{
				*body = NULL;
				ret = -1;
				goto ret_clean;
			}
		}
		else
		{
			/* wait ok and receive a body */
			break;
		}
	}

	if (item != NULL)
	{
		*body = item->body;
		TAILQ_REMOVE(&rq->rpc_queue, item, entries);
		rpc_free(item);
		ret = 1;
	}

ret_clean:
	pthread_mutex_unlock(&rq->rpc_queue_mutex);

	return ret;
}

/*
 * rpc queue ensure stop and return
 */
void rpc_queue_stop(rpc_queue_t * rq)
{
	pthread_mutex_lock(&rq->rpc_queue_mutex);
	rq->rpc_queue_stop_sign = 1;
	pthread_cond_signal(&rq->rpc_queue_cond);
	pthread_mutex_unlock(&rq->rpc_queue_mutex);
}

/*
 * destroy rpc queue and tear down
 */
void rpc_queue_destroy(rpc_queue_t ** rq)
{
	rpc_queue_t * _rq;

	_rq = *rq;
	if (_rq != NULL)
	{
		rpc_queue_stop(_rq);
		assert(TAILQ_FIRST(&_rq->rpc_queue) == NULL);

		pthread_mutex_destroy(&_rq->rpc_queue_mutex);
		pthread_cond_destroy(&_rq->rpc_queue_cond);
		rpc_free(_rq);
		*rq = NULL;
	}
}