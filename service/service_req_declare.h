/*
 * service request handler declare
 *
 * created rui.sun 2012-6-24
 */
#ifndef SERVICE_REQ_DECLARE_H
#define SERVICE_REQ_DECLARE_H

#include <rpc_buffer.h>
#include <rpc_parser.h>

/* platform category */
rpc_buffer_t * req_version_check(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);
rpc_buffer_t * req_heartbeat(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);

/* account category */
rpc_buffer_t * req_account_login(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);
rpc_buffer_t * req_account_logout(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);
rpc_buffer_t * req_account_create(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);
rpc_buffer_t * req_account_modify(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);
rpc_buffer_t * req_account_delete(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);
rpc_buffer_t * req_account_list(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);

/* agent category */
rpc_buffer_t * req_device_create(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);
rpc_buffer_t * req_device_delete(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);
rpc_buffer_t * req_device_list(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);

/* play category */
rpc_buffer_t * req_play_device(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);
rpc_buffer_t * req_play_stop(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);
rpc_buffer_t * req_play_touch(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);


#endif