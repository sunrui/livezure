/*
 * rpc parser
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2012-8
 */
#ifndef RPC_PARSER_H
#define RPC_PARSER_H

#include "rpc_buffer.h"

/*
 * create a new rpc request by rpc_buffer_t
 */
rpc_req_t * rpc_parser_new(rpc_buffer_t * buffer);

/*
 * add a new key/value to rpc parser
 */
void rpc_parser_add(rpc_req_t * req, const char * key, const char * value, rpc_size_t valuesize);

/*
 * get how many key have in this request
 */
int rpc_parser_count(rpc_req_t * req);

/*
 * get user key by index
 *
 * @return
 *     1 ok
 *     0 failed
 */
int rpc_parser_index(rpc_req_t * req, int index, char ** key, char ** value, rpc_size_t * valuesize);

/*
 * get query value such as key=value
 *
 * @return
 *     the value string or NULL if not exist/cannot be read 
 */
const char * rpc_parser_get(rpc_req_t * req, const char * key, rpc_size_t * valuesize);

/*
 * free a rpc request
 */
void rpc_parser_free(rpc_req_t ** req);

#endif