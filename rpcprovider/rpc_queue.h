/*
 * rpc send queue, use for maintenance send/recv pools
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2012-7-31 12:29
 */
#ifndef RPC_QUEUE_H
#define RPC_QUEUE_H

typedef struct rpc_queue rpc_queue_t;

/*
 * create a new rpc queue
 */
rpc_queue_t * rpc_queue_new();

/*
 * push a new body to queue, must be alloc here
 */
void rpc_queue_push(rpc_queue_t * rq, void * item);

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
int rpc_queue_get(rpc_queue_t * rq, void ** body, int timeout);

/*
 * rpc queue ensure stop and return
 */
void rpc_queue_stop(rpc_queue_t * rq);

/*
 * destroy rpc queue and tear down
 */
void rpc_queue_destroy(rpc_queue_t ** rq);

#endif