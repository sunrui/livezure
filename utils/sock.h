/*
 * basic socket functions
 * copyright (c) 2012 soho
 * author rui.sun <smallrui@live.com>
 * created 2012-5-5
 */
#ifndef SOCK_H
#define SOCK_H

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

// effect for win32 only
int socket_init();
int socket_uninit();

int socket_setblock(int fd);
int socket_setnonblock(int fd);
int socket_close(int fd);

// block functions
int socket_send(int fd, const char * inbuffer, int size);
int socket_recv(int fd, char * outbuffer, int maxsize);

/*
 * @return
 *     1 ok
 *     0 failed
 */
int socket_keepalive(int fd, int enable, int seconds);

#endif