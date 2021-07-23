/*
 * basic socket functions
 * copyright (c) 2012 soho
 * author rui.sun <smallrui@live.com>
 * created 2012-5-5
 */
#include <stdbool.h>
#include "posixinc.h"
#include "sock.h"

int socket_init()
{
#ifdef WIN32
	WSADATA wsaData;
	int result;
	
	result = WSAStartup(MAKEWORD(2,2), &wsaData);
	return (result == NO_ERROR);
#else
	return 1;
#endif
}

int socket_uninit()
{
#ifdef WIN32
	int result;
	result = WSACleanup();

	return (result == NO_ERROR);
#else
	return 1;
#endif
}

int socket_close(int fd)
{
#ifdef WIN32
	int result;
	result = closesocket(fd);
	return (result == NO_ERROR);
#else
	close(fd);
	return 1;
#endif
}

int socket_setblock(int fd)
{
#ifdef WIN32
	unsigned long fl = 0;
	return ioctlsocket(fd, FIONBIO, &fl);
#else
	int flags;

	flags = fcntl(fd, F_GETFL, 0);
	return fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
#endif
}

int socket_setnonblock(int fd)
{
#ifdef WIN32
	unsigned long fl = 1;
	return ioctlsocket(fd, FIONBIO, &fl);
#else
	int fl = fcntl(fd, F_GETFL);
	return fcntl(fd, F_SETFL, fl | O_NONBLOCK);
#endif
}

int socket_send(int fd, const char * inbuffer, int size)
{
	int sended = 0;
	int tmpsend = 0;

	while (sended < size)
	{
		tmpsend = send(fd, inbuffer + sended, size - sended, 0);
		if (tmpsend <= 0)
		{
			return sended;
		}

		sended += tmpsend;
	}

	return sended;
}

int socket_recv(int fd, char * outbuffer, int maxsize)
{
	const int max_per_recv = 4096;
	int recved = 0;
	int tmprecved = 0;

	while (1)
	{
		if (recved == maxsize)
		{
			return recved;
		}

		tmprecved = recv(fd, outbuffer + recved, max_per_recv, 0);
		if (tmprecved == SOCKET_ERROR)
		{
			return recved;
		}

		if (tmprecved == 0)
		{
			if (recved == maxsize)
			{
				return recved;
			}
		}

		recved += tmprecved;
	}

	return recved;
}

/*
 * @return
 *     1 ok
 *     0 failed
 */
int socket_keepalive(int fd, int enable, int seconds)
{
	if (setsockopt(fd,
		SOL_SOCKET,
		SO_KEEPALIVE,
		(const char*)&enable,
		sizeof enable) == -1) {
			return 0;
	}

	if (enable && setsockopt(fd,
		IPPROTO_TCP,
		TCP_KEEPALIVE,
		(const char*)&seconds,
		sizeof seconds) == -1) {
			return 0;
	}

	return 1;
}