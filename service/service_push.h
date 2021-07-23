/*
 * service automate push
 *
 * @author rui.sun 2012-7-10 Tue 12:00
 */
#ifndef SERVICE_PUSH_H
#define SERVICE_PUSH_H

#include "session_online_declare.h"

typedef enum service_push_type
{
	push_login_conflict = 0,
	push_session_expired,
} service_push_type;

/*
 * push a new event to queue, it will automate popped when max 
 *  [max_status_queue_time] is arrived
 *
 * @type	active event type
 *
 * @session	online_session_t
 *
 * @return
 *   >=1 event push ok
 *     0 fd is not valid
 */
int service_push(service_push_type type, int fd);

#endif