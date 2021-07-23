/*
 * rpc util header
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2012-12-22 Sta 4:45
 */
#include "rpc_util.h"

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include <assert.h>

/*
 * get socket address by fd
 *
 * @return
 *     1 ok
 *     0 failed
 */
int rpc_util_ip(int fd, char ip[16], int * port)
{
	struct sockaddr_in sin;
	int len;
	int r;

	assert(fd > 0 && ip != NULL && port != NULL);

	*ip = 0;
	*port = 0;
	len = sizeof(sin);
	r = !getpeername(fd, (struct sockaddr *)&sin, &len);
	if (r)
	{
		strncpy(ip, inet_ntoa(sin.sin_addr), 16);
		*port = ntohs(sin.sin_port);
	}

	return r;
}