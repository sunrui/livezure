#include "StdAfx.h"
#if 1

extern "C" {
#include "rpc_service.h"
#include "rpc_keypair.h"
#include "rpc_parser.h"
}

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef WIN32
#define stricmp _stricmp
#endif

#pragma comment(lib, "rpcprovider.lib")

#define rpc_account           0
#define rpc_account_login     1
#define rpc_account_logout    2
#define rpc_adv               1
#define rpc_adv_coming        2

rpc_buffer_t * req_account_login(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);
rpc_buffer_t * req_account_logout(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);

void rpc_event_cb(void * opaque, rpc_event_type type, int fd)
{
	switch (type)
	{
	case event_fd_accept:
		fprintf(stdout, "ACCEPT: %d.\n", fd);
		break;
	case event_fd_close:
		fprintf(stdout, "CLOSE: %d.\n", fd);
		break;
	case event_fd_timeout:
		fprintf(stderr, "TIMEOUT: %d.\n", fd);
		break;
	}
}

void * my_alloc(int size)
{
	return malloc(size);
}

void my_free(void * ptr)
{
	free(ptr);
}

rpc_io_t io;
int client_fd;
int loop;

int main()
{
	io.alloc = my_alloc;
	io.free = my_free;
//	rpc_buffer_io(&io);

	rpc_service_start(13111, 128, 0);
	rpc_service_event(rpc_event_cb, NULL);
	rpc_service_add_cb(rpc_account, rpc_account_login, req_account_login, NULL);
	rpc_service_add_cb(rpc_account, rpc_account_logout, req_account_logout, NULL);

	printf("service start...\n");

	for (loop = 0; loop < 300; loop++)
	{
		rpc_push_param_t push;
		rpc_buffer_t * in;
		int r;

		/* ad will coming */
		Sleep(100);

		push.category = rpc_adv;
		push.sub_category = rpc_adv_coming;
		push.encrypt = crypt_type_tea;

		printf("ad pushing...%d\n", loop);
		{
			char sz[64];

			sprintf(sz, "小葵花妈妈课堂开课啦！%d", loop);

			in = NULL;
			in = rpc_keypair_append(in, "adv", sz, -1, -1);
			push.in = in;
		}

		r = rpc_service_push(client_fd, 1, &push);
	}

	rpc_service_stop();

	return 0;
}

rpc_buffer_t * req_account_login(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
{
	rpc_buffer_t * buffer;
	const char * uuid;
	const char * pwd;

	uuid = rpc_parser_get(req, "uuid", -1, NULL);
	pwd = rpc_parser_get(req, "pwd", -1, NULL);
	buffer = NULL;

	if (uuid == NULL || pwd == NULL ||
		stricmp(uuid, "zhang3") != 0 ||
		strcmp(pwd, "123") != 0)
	{
		buffer = rpc_keypair_append(buffer, "result", "user or pwd failed", -1, -1);
		return buffer;
	}

	buffer = rpc_keypair_append(buffer, "result", "ok", -1, -1);

	{
		int is_admin = 1;
		buffer = rpc_keypair_append(buffer, "is_admin", (const char *)&is_admin, -1, sizeof(is_admin));
	}

	client_fd = fd;

	return buffer;
}

rpc_buffer_t * req_account_logout(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
{
	rpc_buffer_t * buffer;

	buffer = NULL;
	buffer = rpc_keypair_append(buffer, "result", "i know, bye byte!", -1, -1);

	return buffer;
}

#endif

#if 0
/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#include "uv.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct {
	uv_write_t write_req;
  uv_tcp_t handle;
  uv_shutdown_t shutdown_req;
} myconn_rec;

#define container_of(ptr, type, member) \
	((type *) ((char *) (ptr) - offsetof(type, member)))


static uv_tcp_t tcp_server;

static void connection_cb(uv_stream_t* stream, int status);
static uv_buf_t alloc_cb(uv_handle_t* handle, size_t suggested_size);
static void read_cb(uv_stream_t* stream, ssize_t nread, uv_buf_t buf);
static void shutdown_cb(uv_shutdown_t* req, int status);
static void close_cb(uv_handle_t* handle);


static void connection_cb(uv_stream_t* stream, int status) {
  myconn_rec* conn;
  int r;

  ASSERT(status == 0);
  ASSERT(stream == (uv_stream_t*)&tcp_server);

  conn = (myconn_rec *)malloc(sizeof *conn);
  ASSERT(conn != NULL);

  r = uv_tcp_init(stream->loop, &conn->handle);
  ASSERT(r == 0);

  r = uv_accept(stream, (uv_stream_t*)&conn->handle);
  ASSERT(r == 0);

  if (5 > 0)
  {
//	  uv_tcp_keepalive(&tcp_server, 1, 5);
  }

  r = uv_read_start((uv_stream_t*)&conn->handle, alloc_cb, read_cb);
  ASSERT(r == 0);
}


static uv_buf_t alloc_cb(uv_handle_t* handle, size_t suggested_size) {
  static char buf[65536];
  return uv_buf_init(buf, sizeof buf);
}

static void write_cb(uv_write_t* req, int status)
{
	assert(status == 0);
}

static void read_cb(uv_stream_t* stream, ssize_t nread, uv_buf_t buf) {
  myconn_rec* conn;
  int r;
  
  conn = container_of(stream, myconn_rec, handle);

  if (nread >= 0)
	// 写点东西
  {
	  uv_buf_t req_buf;

	  req_buf.len = 4;
	  req_buf.base = "haha";

	  
	  uv_write(&conn->write_req, stream, &req_buf, 1, write_cb);
	return;
  }

//  ASSERT(uv_last_error(stream->loop).code == UV_EOF);


  r = uv_shutdown(&conn->shutdown_req, stream, shutdown_cb);
  ASSERT(r == 0);
}


static void shutdown_cb(uv_shutdown_t* req, int status) {
  myconn_rec* conn = container_of(req, myconn_rec, shutdown_req);
  uv_close((uv_handle_t*)&conn->handle, close_cb);
}


static void close_cb(uv_handle_t* handle) {
  myconn_rec* conn = container_of(handle, myconn_rec, handle);
  free(conn);
}

#include <pthread.h>
uv_loop_t* loop;

void * starttest(void * param)
{
	int r;
	r = uv_run(loop, UV_RUN_DEFAULT);
	return NULL;
}

int main() {
  struct sockaddr_in addr;
  int r;

  loop = uv_default_loop();
  addr = uv_ip4_addr("127.0.0.1", 13111);

  r = uv_tcp_init(loop, &tcp_server);
  ASSERT(r == 0);

  r = uv_tcp_bind(&tcp_server, addr);
  ASSERT(r == 0);

  r = uv_listen((uv_stream_t*)&tcp_server, 128, connection_cb);
  ASSERT(r == 0);

//  ASSERT(0 && "Blackhole server dropped out of event loop.");
  /* set buffer event read timeval and write timeval rs_keepalive seconds*/
  if (5 > 0)
  {
	  uv_tcp_keepalive(&tcp_server, 1, 5);
  }

  {
	  pthread_t thread;

	  pthread_create(&thread, NULL, starttest, 0);
  }

  Sleep(20000);
  uv_close((uv_handle_t*)&tcp_server, NULL);

  getchar();

  return 0;
}
#endif