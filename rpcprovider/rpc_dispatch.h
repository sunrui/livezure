/*
 * rpc request process dispatcher
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2012-7-31 Tue 18:14
 */
#ifndef RPC_DISPATH_H
#define RPC_DISPATH_H

#include "rpc_proto.h"
#include "rpc_parser.h"

/*
 * rpc dispatch module init
 */
void rpc_dispatch_init();

/*
 * add a dispatch callback
 */
void rpc_dispatch_add_cb(rpc_categroy category, rpc_categroy sub_category, pfn_rpc_cb rpc_cb, void * opaque);

/*
 * delete a dispatch callback
 *
 * @return
 *     1 success
 *     0 failed
 */
int rpc_dispatch_del_cb(rpc_categroy category, rpc_categroy sub_category, pfn_rpc_cb rpc_cb, void * opaque);

/*
 * processor request by category id
 */
rpc_buffer_t * rpc_dispatch_processor(rpc_categroy category, rpc_categroy sub_category, int fd, rpc_req_t * req);

/*
 * rpc dispatch module destroy
 */
void rpc_dispatch_destroy();

#endif