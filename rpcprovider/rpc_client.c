/*
 * rpc client core
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2012-8-13 13:40 Mon
 */
#include "rpc_client.h"

#include "rpc_io.h"
#include "rpc_dispatch.h"
#include "rpc_alloctor.h"
#include "sock.h"

#ifdef WIN32
#include "vld.h"
#endif

#include <pthread.h>
#include <stdio.h>
#include <assert.h>

#ifdef WIN32
#ifndef inline
#define inline __inline
#endif
#endif

static int rc_stop_sign;
static pthread_t rc_recv_thrd;
int rc_client_fd; /* rpc_request use it */
static pfn_rpc_disconn_cb rc_disconn_cb;
static void * rc_disconn_opaque;

inline int rpc_io_client_send(int fd, const char * buffer, int length, void * extra)
{
	return send(fd, buffer, length, 0);
}

#define RPC_MAX_LINE 4096
void * rpc_client_routine(void * param)
{
	int process_ret;
	char line[RPC_MAX_LINE + 1];
	int n;

	while (!rc_stop_sign)
	{
		n = recv(rc_client_fd, line, RPC_MAX_LINE, 0);
		if (n > 0)
		{
			line[n] = '\0';
			process_ret = rpc_io_enqueue(rc_client_fd, line, n);
			if (process_ret == -1)
			{
				assert(0 && "got a bad request.");
				socket_close(rc_client_fd);
				rpc_io_del(rc_client_fd);
				break;
			}
		}
		else
		{
			break;
		}
	}

	if (rc_disconn_cb != NULL)
	{
		rc_disconn_cb(rc_disconn_opaque);
	}

	return NULL;
}

/*
 * start rpc client core
 *
 * @keepalive	>0 if need keepalive in seconds
 *
 * @return
 *     0 start ok
 *    -1 connect error
 */
int rpc_client_start(const char * ip, int port, int keepalive)
{
	struct sockaddr_in svr_addr_in;
	int ret;

	rpc_alloctor_init();

#ifdef WIN32
	{
		static int winsock_inited = 0;
		
		if (!winsock_inited)
		{
			WSADATA wsadata;
			WSAStartup(MAKEWORD(1, 1), &wsadata);
			winsock_inited = 1;
		}
	}
#endif

	rc_stop_sign = 0;
	rc_client_fd = 0;

	rc_client_fd = socket(AF_INET, SOCK_STREAM, 0);
	svr_addr_in.sin_family = AF_INET;
	svr_addr_in.sin_addr.S_un.S_addr = inet_addr(ip);
	svr_addr_in.sin_port = htons(port);

	ret = connect(rc_client_fd, (struct sockaddr *)&svr_addr_in, sizeof(svr_addr_in));
	if (ret != 0)
	{
		assert(0 && "connect to server failed.");
		return -1;
	}
	
	rpc_io_init(rpc_io_client_send, 1);
	rpc_dispatch_init();

	socket_setblock(rc_client_fd);
	rpc_io_add(rc_client_fd, NULL);

	if (keepalive > 0)
	{
		socket_keepalive(rc_client_fd, 1, keepalive);
	}

	pthread_create(&rc_recv_thrd, NULL, rpc_client_routine, NULL);

	return 0;
}

/*
 * notify when fd disconnect
 */
void rpc_client_event(pfn_rpc_disconn_cb discon_cb, void * opaque)
{
	rc_disconn_cb = discon_cb;
	rc_disconn_opaque = opaque;
}

/*
 * register a dispatch callback
 */
void rpc_client_register(rpc_categroy category, rpc_categroy sub_category, pfn_rpc_cb rpc_cb, void * opaque)
{
	rpc_alloctor_init();

	rpc_dispatch_add_cb(category, sub_category, rpc_cb, opaque);
}

/*
 * stop rpc client core
 */
void rpc_client_stop()
{
	if (rc_stop_sign)
	{
		return;
	}

	rc_stop_sign = 1;
	socket_close(rc_client_fd);
	assert(rc_client_fd != 0);
	rpc_io_stop(rc_client_fd);
	rpc_io_destroy();
	pthread_join(rc_recv_thrd, NULL);
	rpc_dispatch_destroy();
	rpc_alloctor_destroy();
}
