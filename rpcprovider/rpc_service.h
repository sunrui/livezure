/*
 * rpc service engine
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2012-7-31 Tue 1:22
 */
#ifndef RPC_SERVICE_H
#define RPC_SERVICE_H

#include "rpc_proto.h"

/*
 * push parameter
 */
typedef struct rpc_push_param
{
	rpc_categroy category;		/* message category */
	rpc_categroy sub_category;	/* sub message category */
	rpc_crypt_type encrypt;		/* encrypt type */
	rpc_buffer_t * in;			/* none encrypted buffer */
} rpc_push_param_t;

typedef enum rpc_event_type
{
	event_fd_accept = 0,		/* fd accept */
	event_fd_close,				/* fd close */
	event_fd_timeout			/* user-specified timeout reached */
} rpc_event_type;

/*
 * event callback
 */
typedef void (* pfn_rpc_event_cb)(void * opaque, rpc_event_type type, int fd);

/*
 * start rpc engine
 *
 * @keepalive	>0 if need keepalive in seconds
 * @nprocessors how many threads for processors
 *
 * @return
 *     0 start ok
 *    -1 socket error
 *    -2 bind error
 *    -3 listen error
 */
int rpc_service_start(int listenport, int keepalive, int nprocessors);

/*
 * register fd event
 */
void rpc_service_event(pfn_rpc_event_cb event_cb, void * opaque);

/*
 * register a dispatch callback
 */
void rpc_service_register(rpc_categroy category, rpc_categroy sub_category, pfn_rpc_cb rpc_cb, void * opaque);

/*
 * push a message to client
 *
 * @fd	client fd
 *
 * @notify_type
 *     0 notify all (ignore fd)
 *     1 notify just fd
 *     2 notify except fd
 *
 * @return
 *     >=1 if notify ok
 *     0 failed
 */
int rpc_service_push(int fd, int notify_type, rpc_push_param_t * param);

/*
 * stop rpc engine
 */
void rpc_service_stop();

#endif