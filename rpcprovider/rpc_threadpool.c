/*
 * rpc thread pool
 *
 * @copyright (c) livezure.com
 * @date 2013-4-12 0:48 rui.sun
 */
#include "rpc_threadpool.h"
#include "queue.h"

#include <semaphore.h>
#include <pthread.h>

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifdef WIN32
#include <Windows.h>
#ifndef usleep
#define usleep(n) Sleep(n / 1000);
#endif
#endif

struct rpc_tp_worker
{
	rpc_tp_process_cb process_cb;
	rpc_tp_complete_cb complete_cb;
	void * opaque;
	void * result;
	sem_t sem;
	rpc_tp_state state;
	TAILQ_ENTRY(rpc_tp_worker) entries;
};

struct rpc_tp
{
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	
	TAILQ_HEAD(, rpc_tp_worker) worker_queue;
	pthread_t * thread;
	int nthreads;
	int abort;
};

void * rpc_tp_routine(void * param)
{
	struct rpc_tp * pool = (struct rpc_tp *)param;

	for (;;)
	{
		rpc_tp_worker_t * worker;

		pthread_mutex_lock(&pool->mutex);
		while ((worker = TAILQ_FIRST(&pool->worker_queue)) == NULL)
		{
			if (pool->abort)
			{
				pthread_mutex_unlock(&pool->mutex);
				pthread_exit(NULL);
				return NULL;
			}

			pthread_cond_wait(&pool->cond, &pool->mutex);
		}

		TAILQ_REMOVE(&pool->worker_queue, worker, entries);
		pthread_mutex_unlock(&pool->mutex);
		assert(worker != NULL);

		worker->state = tp_state_busy;
		worker->result = (*worker->process_cb)(worker->opaque);
		worker->state = tp_state_complete;
		sem_post(&worker->sem);

		if (worker->complete_cb != NULL)
		{
			(*worker->complete_cb)(worker, worker->result, worker->opaque);
		}
	}

	return NULL;
}

rpc_tp_t * rpc_tp_create(int nthreads)
{
	rpc_tp_t * pool;

	assert(nthreads > 0);
	pool = (rpc_tp_t *)calloc(1, sizeof(rpc_tp_t));
	pthread_mutex_init(&pool->mutex, NULL);
	pthread_cond_init(&pool->cond, NULL);
	pool->nthreads = nthreads;
	TAILQ_INIT(&pool->worker_queue);
	pool->abort = 0;

	{
		int index;
		
		pool->thread = (pthread_t *)malloc(nthreads * sizeof(pthread_t));
		for (index = 0; index < nthreads; index++)
		{
			pthread_create(&pool->thread[index], NULL, rpc_tp_routine, pool);
		}
	}

	return pool;
}

rpc_tp_worker_t * rpc_tp_worker_register(rpc_tp_t * pool, rpc_tp_process_cb process_cb, rpc_tp_complete_cb complete_cb, void * opaque)
{
	rpc_tp_worker_t * worker;

	if (pool == NULL || pool->abort || process_cb == NULL || complete_cb == NULL)
	{
		assert(0);
		return NULL;
	}

	worker = (rpc_tp_worker_t *)calloc(1, sizeof(rpc_tp_worker_t));
	worker->process_cb = process_cb;
	worker->complete_cb = complete_cb;
	worker->opaque = opaque;
	worker->result = NULL;
	worker->state = tp_state_idle;
	sem_init(&worker->sem, 0, 0);

	pthread_mutex_lock(&pool->mutex);
	TAILQ_INSERT_TAIL(&pool->worker_queue, worker, entries);
	pthread_cond_signal(&pool->cond);
	pthread_mutex_unlock(&pool->mutex);

	return worker;
}

rpc_tp_state rpc_tp_worker_touch(rpc_tp_worker_t * worker)
{
	if (worker == NULL)
	{
		assert(0);
		return tp_state_invalid;
	}

	return worker->state;
}

void * rpc_tp_worker_wait(rpc_tp_worker_t * worker)
{
	if (worker == NULL)
	{
		assert(0);
		return NULL;
	}

	sem_wait(&worker->sem);
	return worker->result;
}

void rpc_tp_worker_free(rpc_tp_worker_t * worker)
{
	sem_post(&worker->sem);
	sem_destroy(&worker->sem);
	free(worker);
}

void rpc_tp_destroy(rpc_tp_t * pool, rpc_tp_kill type)
{
	if (pool == NULL || pool->abort)
	{
		return;
	}

	if (type == tp_kill_wait)
	{
		while (TAILQ_FIRST(&pool->worker_queue) != NULL)
		{
			usleep(50 * 1000);
		}
	}

	pthread_mutex_lock(&pool->mutex);
	pool->abort = 1;
	pthread_cond_broadcast(&pool->cond);
	pthread_mutex_unlock(&pool->mutex);

	{
		int index;

		for (index = 0; index < pool->nthreads; index++)
		{
			pthread_join(pool->thread[index], NULL);
		}
	}

	{
		rpc_tp_worker_t * worker;

		while ((worker = TAILQ_FIRST(&pool->worker_queue)) != NULL)
		{
			TAILQ_REMOVE(&pool->worker_queue, worker, entries);
			sem_post(&worker->sem);
			sem_destroy(&worker->sem);
			free(worker);
		}
	}

	pthread_mutex_destroy(&pool->mutex);
	pthread_cond_destroy(&pool->cond);
	free(pool->thread);
	free(pool);
}

#ifdef TEST_RPC_THREADPOOL_H
#include <windows.h>
#include <stdio.h>
#include <vld.h>

#include "rpc_threadpool.h"

void * process_cb(void * opaque)
{
	int taskid;
	taskid = (int)opaque;
	printf("taskid = %d.\n", taskid);
	Sleep(2000);

	return NULL;
}

void complete_cb(rpc_tp_worker_t * worker, void * result, void * opaque)
{
	rpc_tp_worker_free(worker);
}

int main()
{
	rpc_tp_t * pool;

	pool = rpc_tp_create(5);

	{
		rpc_tp_process_cb pcb;
		rpc_tp_complete_cb ccb;
		void * opaque;
		int tasks;

		pcb = process_cb;
		ccb = complete_cb;

		for (tasks = 0; tasks < 19; tasks++)
		{
			rpc_tp_worker_t * worker;
			rpc_tp_state state;
			void * result;

			opaque = (void *)tasks;
			worker = rpc_tp_worker_register(pool, pcb, ccb, opaque);
			state = rpc_tp_worker_touch(worker);

			result = NULL;
			//result = rpc_tp_worker_wait(worker);
		}

		rpc_tp_destroy(pool, tp_kill_wait);
		printf("destory ok.\n");
	}
}
#endif