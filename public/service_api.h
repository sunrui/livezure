/*
 * service api
 *
 * update 2012-6-15 rui.sun <smallrui@live.com>
 */
#ifndef SERVICE_API_H
#define SERVICE_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rpc_declare.h"

/* db api export from db_util.h */
void db_init(const char * ip, int port, const char * user, const char * pwd, const char * db);
void db_clean();

/*
 * @return
 *    1 connect ok
 *    0 failed
 */
int db_test_connect(char err[64]);

typedef enum service_event_type
{
	service_fd_accept = 0, /* fd accept */
	service_fd_close, /* fd close */
	service_fd_timeout /* user-specified timeout reached */
} service_event_type;

/*
 * event callback
 */
typedef void (* pfn_service_event_cb)(void * opaque, service_event_type type, int fd);

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
int service_start_async(int listenport, int keepalive, int nprocessors);

/*
 * register fd event
 */
void service_event(pfn_service_event_cb event_cb, void * opaque);

/*
 * @return stop status 
 *    0 stop ok
 *	 -1 service start maybe with issues
 */
int service_stop();

#ifdef __cplusplus
}
#endif

#endif