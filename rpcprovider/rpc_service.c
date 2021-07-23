/*
* rpc service engine libuv version
*
* Copyright (C) 2012-2013 livezure.com
* rui.sun 2013-1-23 Tue 15:27
*/
#include "rpc_service.h"

#include "rpc_io.h"
#include "rpc_dispatch.h"
#include "rpc_util.h"
#include "rpc_alloctor.h"

#include "uv.h"
#include "posixtime.h"
#include "list.h"

#ifdef WIN32
#include "vld.h"
#endif

#include <pthread.h>
#include <assert.h>
#include <stdio.h>

#ifdef WIN32
#ifndef inline
#define inline __inline
#endif
#endif

static const char * appInfo = "rpc network interaction engine, powered by livezure.com.";

typedef struct {
	uv_write_t req;
	uv_buf_t buf;
} write_req_t;

static uv_async_t rs_loop_exit_handle;
static uv_loop_t * rs_loop = NULL;

static uv_tcp_t rs_fd;
static pfn_rpc_event_cb rs_event_cb;
static void * rs_event_opaque;
static int rs_keepalive = 0;
static int rs_env_inited = 0;

static pthread_t rs_check_thread; /* check if service push a msg that in msg queue */
static int rs_check_stop_sign = 0; /* stop service sign */

void connection_cb(uv_stream_t* server, int status);
uv_buf_t alloc_cb(uv_handle_t* handle, size_t suggested_size);
void read_cb(uv_stream_t* stream, ssize_t nread, uv_buf_t buf);
int fd_write(uv_stream_t* stream, const char * buffer, int length);
void write_cb(uv_write_t* req, int status);
void shutdown_cb(uv_shutdown_t* req, int status);
void close_cb(uv_handle_t* handle);

inline int rpc_io_server_send(int fd, const char * buffer, int length, void * extra)
{
	return fd_write((uv_stream_t *)extra, buffer, length);
}

void connection_cb(uv_stream_t* server, int status)
{
	uv_stream_t * stream;
	int fd;
	int r;

	if (status != 0 || server != (uv_stream_t*)&rs_fd)
	{
		return;
	}

	stream = (uv_stream_t *)malloc(sizeof(uv_tcp_t));
	r = uv_tcp_init(rs_loop, (uv_tcp_t *)stream);
	assert(r == 0);

	stream->data = server;
	r = uv_accept(server, (uv_stream_t*)stream);
	assert(r == 0);

	{
		uv_tcp_t* tcp = (uv_tcp_t *)stream;
		fd = tcp->socket;
	}

	{
		int ret;
		ret = rpc_io_add(fd, stream);
		assert(ret == 1);
	}

#ifdef _DEBUG
	{
		char ip[17];
		int port;
		int r;

		r = rpc_util_ip(fd, ip, &port);
		assert(r);

		fprintf(stdout, "ACCEPT: fd = %u, ip = %s, port = %d.\n", fd, ip, port);
	}
#endif

	/* set buffer event read timeval and write timeval rs_keepalive seconds*/
	if (rs_keepalive > 0)
	{
		uv_tcp_keepalive((uv_tcp_t *)stream, 1, rs_keepalive);
	}

	/* notify on fd accept */
	if (rs_event_cb != NULL)
	{
		rs_event_cb(rs_event_opaque, event_fd_accept, fd);
	}

	r = uv_read_start((uv_stream_t*)stream, alloc_cb, read_cb);
	assert(r == 0);
}

uv_buf_t alloc_cb(uv_handle_t* handle, size_t suggested_size)
{
	static char buf[65536];
	return uv_buf_init(buf, sizeof buf);
}

void write_cb(uv_write_t* req, int status)
{
	write_req_t* wr;
	uv_err_t err;

	wr = (write_req_t*) req;
	free(wr);

	if (status == 0)
	{
		return;
	}

	err = uv_last_error(rs_loop);
	if (err.code == UV_ECANCELED)
	{
		return;
	}

	assert(err.code == UV_EPIPE);
	uv_close((uv_handle_t*)req->handle, close_cb);
}

int fd_write(uv_stream_t* stream, const char * buffer, int length)
{
	int fd;

	{
		uv_tcp_t* tcp = (uv_tcp_t *)stream;
		fd = tcp->socket;
	}

	{
		write_req_t * wr;
		uv_buf_t req_buf;

		req_buf.len = length;
		req_buf.base = (char *)buffer;

		wr = (write_req_t*) malloc(sizeof(*wr));
		wr->buf = req_buf;
		uv_write(&wr->req, stream, &req_buf, 1, write_cb);
	}

	//- fixed me for real size
	return length;
}

void read_cb(uv_stream_t* stream, ssize_t nread, uv_buf_t buf)
{
	int fd;

	{
		uv_tcp_t* tcp = (uv_tcp_t *)stream;
		fd = tcp->socket;
	}

	if (nread >= 0)
	{
		int ret;
		ret = rpc_io_enqueue(fd, buf.base, nread);
		if (ret == -1)
		{
			assert(0 && "got a bad request.");
			goto ret_shuwdown;
		}

		return;
	}

	/* notify on fd close */
	if (rs_event_cb != NULL)
	{
		rpc_event_type type;

		switch (uv_last_error(stream->loop).code)
		{
		case UV_ETIMEDOUT:
			type = event_fd_timeout;
			break;
		case UV_EOF:
		default:
			type = event_fd_close;
			break;
		}

		rs_event_cb(rs_event_opaque, type, fd);
	}

ret_shuwdown:
	{
		uv_shutdown_t* req;
		int r;

		req = (uv_shutdown_t*) malloc(sizeof *req);
		r = uv_shutdown(req, stream, shutdown_cb);
		assert(r == 0);
	}
}

void shutdown_cb(uv_shutdown_t* req, int status) 
{
	{
		uv_tcp_t* tcp = (uv_tcp_t*)req->handle;
		int fd;

		fd = tcp->socket;
		rpc_io_del(fd);

#ifdef _DEBUG
		{
			char ip[17];
			int port;
			int r;

			r = rpc_util_ip(fd, ip, &port);
			assert(r);

			fprintf(stdout, "CLOSE: fd = %u, ip = %s, port = %d.\n", fd, ip, port);
		}
#endif
	}

	uv_close((uv_handle_t*)req->handle, close_cb);
	free(req);
}

void close_cb(uv_handle_t* handle)
{
	free(handle);
}

void exit_async_cb(uv_async_t* handle, int status) {
	/* After closing the async handle, it will no longer keep the loop alive. */
	uv_close((uv_handle_t*)&rs_loop_exit_handle, NULL);
}

void * rpc_core_uvloop_env(void * param)
{
	uv_run(rs_loop, UV_RUN_DEFAULT);
	rs_loop = NULL;
	rs_env_inited = 0;

	return (void *)0;
}

void rpc_ensure_uv_loop_inited()
{
	if (!rs_env_inited)
	{
		assert(rs_loop == NULL);
		rs_loop = uv_default_loop();

		/* The existence of the async handle will keep the loop alive. */
		uv_async_init(rs_loop, &rs_loop_exit_handle, exit_async_cb);

		{
			pthread_t rs_thread;
			pthread_create(&rs_thread, NULL, rpc_core_uvloop_env, NULL);
			pthread_detach(rs_thread);
		}

		rs_env_inited = 1;
	}
}

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
int rpc_service_start(int listenport, int keepalive, int nprocessors)
{
	struct sockaddr_in sin;
	int r;

	if (!appInfo || strlen(appInfo) != 56)
	{
		return -1;
	}

	rpc_ensure_uv_loop_inited();
	rpc_alloctor_init();
	rpc_io_init(rpc_io_server_send, nprocessors);
	rpc_dispatch_init();
	rs_check_stop_sign = 0;

	sin = uv_ip4_addr("127.0.0.1", listenport);
	r = uv_tcp_init(rs_loop, &rs_fd);
	assert(r == 0);

	r = uv_tcp_bind(&rs_fd, sin);
	if (r < 0)
	{
		assert(0 && "bind error.");
		return -2;
	}

	r = uv_listen((uv_stream_t*)&rs_fd, 32, connection_cb);
	if (r < 0)
	{
		assert(0 && "listen error.");
		return -3;
	}

	/* set buffer event read timeval and write timeval rs_keepalive seconds*/
	if (keepalive > 0)
	{
		uv_tcp_keepalive(&rs_fd, 1, keepalive);
	}

	return 0;
}

/*
 * register fd event
 */
void rpc_service_event(pfn_rpc_event_cb event_cb, void * opaque)
{
	rs_event_cb = event_cb;
	rs_event_opaque = opaque;
}

/*
 * register a dispatch callback
 */
void rpc_service_register(rpc_categroy category, rpc_categroy sub_category, pfn_rpc_cb rpc_cb, void * opaque)
{
	rpc_alloctor_init();

	rpc_dispatch_add_cb(category, sub_category, rpc_cb, opaque);
}

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
int rpc_service_push(int fd, int notify_type, rpc_push_param_t * param)
{
	rpc_buffer_t * rpc;
	rpc_shell_t shell;
	int ret;

	memset(&shell, 0, sizeof(shell));
	shell.magic = RPC_MAGIC_NUMBER;
	shell.version = RPC_VERSION_SIGN;
	shell.category = param->category;
	shell.sub_category = param->sub_category;
	shell.crypt = param->encrypt;
	shell.req = req_type_post;

	rpc = rpc_io_make_shell(&shell, param->in);
	ret = rpc_io_nodify_fd(fd, notify_type, rpc);

	rpc_buffer_free(&param->in);
	rpc_buffer_free(&rpc);

	return ret;
}

/*
* stop rpc engine
*/
void rpc_service_stop()
{
	if (!rs_check_stop_sign)
	{
		rs_check_stop_sign = 1;
		uv_close((uv_handle_t*)&rs_fd, NULL);
		pthread_join(rs_check_thread, NULL);
		uv_async_send(&rs_loop_exit_handle);

		rpc_dispatch_destroy();
		rpc_io_destroy();
		rpc_alloctor_destroy();

		rs_check_stop_sign = 0;
	}
}