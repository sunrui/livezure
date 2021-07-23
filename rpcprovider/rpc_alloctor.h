/*
 * rpc memory allocator
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2013-1-28 15:51
 */
#ifndef RPC_ALLOCTOR_H
#define RPC_ALLOCTOR_H

/*
 * rpc allocator init/destroy
 */
void rpc_alloctor_init();
void rpc_alloctor_destroy();

/*
 * memory malloc, calloc, free
 */
void * rpc_malloc(int size);
void * rpc_calloc(int count, int size);
void rpc_free(void * ptr);

#endif