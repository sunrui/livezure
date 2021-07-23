/*
 * service automate push
 *
 * @author rui.sun 2012-7-10 Tue 12:00
 */
#include "service_push.h"
#include "rpc_io.h"
#include "rpc_keypair.h"
#include "rpc_request.h"

#include <assert.h>
#include <stdio.h>

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
int service_push(service_push_type type, int fd)
{
///	rpc_io_nodify_fd;
	//rpc_buffer_t * rpc;
	//rpc_buffer_t * in;
	//int notify_type;
	//int ret;

	//ret = 0;
	//in = NULL;
	//in = rpc_keypair_append(in, "uuid", session->uuid, -1);

	//{
	//	char cplatformid[8];
	//	sprintf(cplatformid, "%d", session->pid);
	//	in = rpc_keypair_append(in, "pid", cplatformid, -1);
	//}

	//if (type == rpc_id_event_cb_repeat)
	//{
	//	notify_type = 1;
	//}
	//else
	//{
	//	notify_type = 2;
	//}

	//rpc = rpc_io_make_shell(rpc_id_event_cb, type, rpc_id_encrypt, rpc_id_version, in);
	//ret = rpc_io_nodify_fd(session->fd, notify_type, rpc);
	//rpc_buffer_free(&in);
	//rpc_buffer_free(&rpc);

	//return ret;

	return 0;
}
