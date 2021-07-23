/*
 * rpc buffer helper, user for serial buffer streams
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2012-7-26 Thu 8:04
 */
#ifndef RPC_BUFFER_H
#define RPC_BUFFER_H

#include "rpc_proto.h"

typedef struct rpc_alloctor
{
	void * (* alloc)(int size);
	void (* free)(void * ptr);
} rpc_alloctor_t;

/*
 * register custom buffer allocator
 */
void rpc_buffer_allocator(rpc_alloctor_t * io);

/*
 * create a new rpc buffer
 */
rpc_buffer_t * rpc_buffer_new(const char * buffer, rpc_size_t size);

/*
 * clone a rpc buffer
 */
rpc_buffer_t * rpc_buffer_clone(rpc_buffer_t * rpc);

/*
 * rpc buffer get buffers
 */
const char * rpc_buffer_get_buffer(rpc_buffer_t * buffer);

/*
 * reset rpc parser position
 */
void rpc_buffer_rewind(rpc_buffer_t * buffer);

/*
 * rpc buffer get buffer size
 */
rpc_size_t rpc_buffer_get_size(rpc_buffer_t * buffer);

/*
 * free rpc buffers
 */
void rpc_buffer_free(rpc_buffer_t ** buffer);

/*
 * encrypt an rpc buffer
 */
rpc_buffer_t * rpc_buffer_encrypt(rpc_buffer_t * buffer, rpc_crypt_type type);

/*
 * decrypt an rpc buffer
 */
rpc_buffer_t * rpc_buffer_decrypt(rpc_buffer_t * buffer, rpc_crypt_type type);

/*
 * append a buffer to rpc buffer, rpc buffer can be null
 *  if first buffer append here.
 */
rpc_buffer_t * rpc_buffer_append(rpc_buffer_t * buffer, const char * in, rpc_size_t size);

/*
 * parse a rpc buffer and moved the position.
 *  return null if no any more buffers.
 */
const char * rpc_buffer_next(rpc_buffer_t * buffer, rpc_size_t * size);

#endif