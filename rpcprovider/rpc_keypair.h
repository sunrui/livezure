/*
 * rpc buffer helper, user for serial buffer streams
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2012-7-26 Thu 8:04
 */
#ifndef RPC_KEYPAIR_H
#define RPC_KEYPAIR_H

#include "rpc_buffer.h"

/*
 * append a query key, rpc buff can be NULL if first item append here
 */
rpc_buffer_t * rpc_keypair_append(rpc_buffer_t * buffer, const char * key, const char * body, rpc_size_t bodysize);

/*
 * get rpc next key
 *
 * @return
 *     1 if get ok
 *     0 get failed
 */
int rpc_keypair_next(rpc_buffer_t * buffer, const char ** key, const char ** body, rpc_size_t * bodysize);

#endif