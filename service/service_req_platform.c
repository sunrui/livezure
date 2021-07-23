/*
 * client request version processor
 *
 * update 2012-6-17 0:01 rui.sun
 */
#include "rpc_keypair.h"
#include "rpc_parser.h"

#include "db_account_declare.h"
#include "session_online_declare.h"
#include "service_session.h"

#include <stdlib.h>
#include <assert.h>

rpc_buffer_t * req_version_check(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
{
	rpc_buffer_t * buffer;
	const char * pid;
	const char * ver;

	pid = rpc_parser_get(req, "pid", NULL);
	ver = rpc_parser_get(req, "ver", NULL);

	/* illegal request, record to sys log? */
	if (ver == NULL || !valid_platform(atoi(pid)))
	{
		buffer = rpc_keypair_append(NULL, "rid", "-255", -1);
		assert(0);
		return buffer;
	}

	{
		buffer = rpc_keypair_append(NULL, "rid", "0", -1);
		buffer = rpc_keypair_append(buffer, "new_ver", "1.0.0.0", -1);
		buffer = rpc_keypair_append(buffer, "update_log", "no need update current.", -1);
		buffer = rpc_keypair_append(buffer, "force_update", "0", -1);
		
		return buffer;
	}
}

rpc_buffer_t * req_heartbeat(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
{
	rpc_buffer_t * buffer;
	online_session_t * session;
	bool r;

	r = online_session_ref(fd, &session);
	if (!r)
	{
		/* -300, session expired */
		buffer = rpc_keypair_append(NULL, "rid", "-300", -1);
		return buffer;
	}

	online_session_dref(session);
	session->net.is_heartbeated = true;

	/* 0, heartbeat ok */
	buffer = rpc_keypair_append(NULL, "rid", "0", -1);
	return buffer;
}