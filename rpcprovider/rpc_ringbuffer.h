/*
 * rpc ring buffer
 * @copyright (c) livezure.com
 * @date 2013-4-13 0:45
 */
#ifndef RPC_RINGBUFFER_H
#define RPC_RINGBUFFER_H

typedef struct rpc_rb rpc_rb_t;

rpc_rb_t * rpc_rb_new(int capacity, int expand);

int rpc_rb_capacity(rpc_rb_t * rb);
int rpc_rb_can_read(rpc_rb_t * rb);
int rpc_rb_can_write(rpc_rb_t * rb);

int rpc_rb_read(rpc_rb_t * rb, char ** data, int count);
int rpc_rb_write(rpc_rb_t * rb, const char * data, int count);

void rpc_rb_free(rpc_rb_t * rb);

#endif