/*
 * rpc request used for client2server only
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2012-8-14 Tue 18:41
 */
#ifndef RPC_REQUEST_H
#define RPC_REQUEST_H

#include "rpc_proto.h"

typedef struct rpc_handle rpc_handle_t;

/*
 * send a request
 *
 * @return
 *     new rpc request handle
 *     NULL if cannot notify by current fd
 */
rpc_handle_t * rpc_request_new(rpc_categroy category, rpc_categroy sub_category, rpc_crypt_type enc_type, rpc_buffer_t * in);

/*
 * wait for response returned, operation will be returned 
 *  when time reached or response returned
 *
 * @param[in]
 *     timeout wait timeout in millisecond or <0 infinite
 *
 * @return
 *     1 response ok
 *     0 time reached
 */
int rpc_request_wait(rpc_handle_t * handle, int timeout);

/*
 * check whether response is ok
 *
 * @return
 *     1 ok
 *     0 failed
 */
int rpc_request_ok(rpc_handle_t * handle, const rpc_req_t ** out_req);

/*
 * destroy current rpc request and cancel
 */
void rpc_request_destroy(rpc_handle_t ** handle);

#endif