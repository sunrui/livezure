/*
 * rpc shell declare
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2012-7-30 Mon 17:42
 */
#ifndef RPC_PROTO_H
#define RPC_PROTO_H

/* rpc request type */
typedef enum rpc_req_type
{
	req_type_get = 0,
	req_type_post
} rpc_req_type;

/* rpc crypt type */
typedef enum rpc_crypt_type
{
	crypt_type_none = 0,
	crypt_type_aes,
	crypt_type_tea
} rpc_crypt_type;

/* rpc buffer size must be short than 2G */
typedef int rpc_size_t;

/* category id must be in 0-127 */
typedef char rpc_categroy;

typedef struct rpc_buffer rpc_buffer_t;
typedef struct rpc_req rpc_req_t;

/*
 * the callback function that gets invoked on requesting path
 *
 * @opaque
 *     defined user data
 *
 * @req
 *     callback that can be use for control header key
 *
 * @return
 *     return alloc-ed rpc buffer
 */
typedef rpc_buffer_t * (* pfn_rpc_cb)(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);

#endif