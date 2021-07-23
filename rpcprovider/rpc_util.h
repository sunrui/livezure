/*
 * rpc util header
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2012-12-22 Sta 4:45
 */
#ifndef RPC_UTIL_H
#define RPC_UTIL_H

/*
 * get socket address by fd
 *
 * @return
 *     1 ok
 *     0 failed
 */
int rpc_util_ip(int fd, char ip[16], int * port);

#endif