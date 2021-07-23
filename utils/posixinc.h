#ifndef WPOSIX_H
#define WPOSIX_H

#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#else
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#endif

#include "posixtime.h"
#include "queue.h"
#include "list.h"

#ifdef WIN32
#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif
#pragma warning(disable: 4996)
#define itoa _itoa
#define strdup _strdup
#define stricmp _stricmp
#endif

#endif