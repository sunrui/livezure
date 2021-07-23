/*
 * rpc io header
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2012-7-31 Tue 10:41
 */
#ifndef RPC_IO_H
#define RPC_IO_H

#include "rpc_proto.h"
#include "rpc_queue.h"
#include "rpc_buffer.h"
#include "rpc_shell.h"

/*
 * register io send callback
 */
typedef int (* rpc_io_send_cb)(int fd, const char * buffer, int length, void * extra);

/*
 * rpc io module init
 */
void rpc_io_init(rpc_io_send_cb send_cb, int nprocessors);

/*
 * make a rpc shell manual
 */
rpc_buffer_t * rpc_io_make_shell(rpc_shell_t * shell, rpc_buffer_t * in);

/*
 * rpc io insert a new fd
 *
 * @return
 *     1 add ok
 *     0 exist no need add
 */
int rpc_io_add(int fd, void * extra);

/*
 * serialize received buffer to rpc_buffer_t
 *
 * @return
 *   >=1 get number of packet ok
 *     0 buffer is not enough
 *    -1 bad request, must clean current connection
 */
int rpc_io_enqueue(int fd, const char * inbuf, int insize);

/*
 * get a body from queue
 *
 * @body	body if get ok, you must manual free it.
 * @timeout	send timeout in milliseconds or < 0 if infinite
 *
 * @return
 *     1 get it ok
 *     0 no more buffers now (timeout reached)
 *    -1 by your abort
 */
int rpc_io_get(int fd, rpc_buffer_t ** out, int timeout);

/*
 * send client an message
 *
 * @curfd	current fd
 *
 * @notify_type
 *     0 notify all
 *     1 notify just curfd
 *     2 notify except curfd
 *
 * @return
 *     >=1 if notify ok
 *     0 failed
 */
int rpc_io_nodify_fd(int curfd, int notify_type, rpc_buffer_t * in);

/*
 * stop a rpc io
 */
void rpc_io_stop(int fd);

/*
 * discard a fd ensure that no longer used
 */
void rpc_io_del(int fd);

typedef struct rpc_fd
{
	int fd;
	void * extra;
} rpc_fd_t;

/*
 * get all child fd
 */
void rpc_io_all(rpc_fd_t ** fds, int * count);
void rpc_io_all_free(rpc_fd_t ** fds);

/*
 * rpc io module destroy
 */
void rpc_io_destroy();

#endif