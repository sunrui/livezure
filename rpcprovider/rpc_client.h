/*
 * rpc client core
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2012-8-13 13:40 Mon
 */
#ifndef RPC_CLIENT_H
#define RPC_CLIENT_H

#include "rpc_proto.h"

/*
 * client stopped callback
 */
typedef void (* pfn_rpc_disconn_cb)(void * opaque);

/*
 * start rpc client core
 *
 * @keepalive	>0 if need keepalive in seconds
 *
 * @return
 *     0 start ok
 *    -1 connect error
 */
int rpc_client_start(const char * ip, int port, int keepalive);

/*
 * notify when fd disconnect
 */
void rpc_client_event(pfn_rpc_disconn_cb discon_cb, void * opaque);

/*
 * register a dispatch callback
 */
void rpc_client_register(rpc_categroy category, rpc_categroy sub_category, pfn_rpc_cb rpc_cb, void * opaque);

/*
 * stop rpc client core
 */
void rpc_client_stop();

#endif
