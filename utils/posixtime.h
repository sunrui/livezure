/*
 * posix gettimeofday which source from live555
 *
 * @author rui.sun Thu 23:23
 */
#ifndef POSIX_TIME_H
#define POSIX_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#include <winsock.h>
#if !defined(_WIN32_WCE)
#include <sys/timeb.h>
#endif

int gettimeofday(struct timeval*, int*);
int usleep(unsigned int useconds);

#endif

/*
 * count timer sub in milliseconds
 */
int timer_sub(struct timeval * now, struct timeval * last);

#ifdef __cplusplus
}
#endif

#endif
