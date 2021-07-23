/*
 * rpc thread pool
 *
 * @copyright (c) livezure.com
 * @date 2013-4-12 0:48 rui.sun
 */
#ifndef RPC_THREADPOOL_H
#define RPC_THREADPOOL_H

typedef struct rpc_tp rpc_tp_t;
typedef struct rpc_tp_worker rpc_tp_worker_t;

typedef void (* rpc_tp_complete_cb)(rpc_tp_worker_t * worker, void * result, void * opaque);
typedef void * (* rpc_tp_process_cb)(void * opaque);

typedef enum rpc_tp_state
{
	tp_state_idle = 0,
	tp_state_busy,
	tp_state_complete,
	tp_state_invalid,
} rpc_tp_state;

typedef enum rpc_tp_kill
{
	tp_kill_immediate,
	tp_kill_wait
} rpc_tp_kill;

rpc_tp_t * rpc_tp_create(int nthreads);
rpc_tp_worker_t * rpc_tp_worker_register(rpc_tp_t * pool, rpc_tp_process_cb process_cb, rpc_tp_complete_cb complete_cb, void * opaque);
rpc_tp_state rpc_tp_worker_touch(rpc_tp_worker_t * worker);
void * rpc_tp_worker_wait(rpc_tp_worker_t * worker);
void rpc_tp_worker_free(rpc_tp_worker_t * worker);
void rpc_tp_destroy(rpc_tp_t * pool, rpc_tp_kill type);

#endif