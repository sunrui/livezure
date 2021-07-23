/*
 * client request version processor
 *
 * update 2012-6-17 0:01 rui.sun
 */
#include "rpc_keypair.h"
#include "rpc_parser.h"

#include "db_account_declare.h"
#include "service_session.h"
#include "db_account.h"
#include "service_device.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

rpc_buffer_t * req_play_device(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
{
	rpc_buffer_t * buffer;
	const char * uuid;
	
	uuid = rpc_parser_get(req, "uuid", NULL);
	
	/* illegal request, record to sys log? */
	if (uuid == NULL)
	{
		buffer = rpc_keypair_append(NULL, "rid", "-255", -1);
		assert(0);
		return buffer;
	}

	{
		online_session_t * session;
		bool r;

		r = online_session_ref(fd, &session);
		if (!r)
		{
			/* -300, session expired */
			buffer = rpc_keypair_append(NULL, "rid", "-300", -1);
			return buffer;
		}

		/* check device weather ref to admin */
		{
			int count;
			const char * admin_uuid;

			admin_uuid = session->is_admin ? session->basic.uuid : session->ref_admin_uuid;
			assert(admin_uuid[0] != '\0');

			r = db_account_device_get(admin_uuid, uuid, NULL, &count);
			assert(!r || (r && count == 1));
			if (!r)
			{
				/* -1, uuid not exist */
				buffer = rpc_keypair_append(NULL, "rid", "-1", -1);
				return buffer;
			}

			{
				int work_r;
			
				work_r = session_device_ref_work(uuid);
				/* play failed maybe something wrong, see session_device_ref_work return result. */
				if (r < 0)
				{
					char c_ret[32];

					sprintf(c_ret, "%d", r - 1);
					buffer = rpc_keypair_append(NULL, "rid", c_ret, -1);
					return buffer;
				}

				/* device play ok */
				{
					char c_ret[32];

					sprintf(c_ret, "%d", work_r);
					buffer = rpc_keypair_append(NULL, "rid", c_ret, -1);
					return buffer;
				}
			}
		}
	}
}

rpc_buffer_t * req_play_stop(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
{
	rpc_buffer_t * buffer;
	const char * uuid;

	uuid = rpc_parser_get(req, "uuid", NULL);

	/* illegal request, record to sys log? */
	if (uuid == NULL)
	{
		buffer = rpc_keypair_append(NULL, "rid", "-255", -1);
		assert(0);
		return buffer;
	}

	{
		online_session_t * session;
		bool r;

		r = online_session_ref(fd, &session);
		if (!r)
		{
			/* -300, session expired */
			buffer = rpc_keypair_append(NULL, "rid", "-300", -1);
			return buffer;
		}

		/* check device weather ref to admin */
		{
			int count;
			const char * admin_uuid;

			admin_uuid = session->is_admin ? session->basic.uuid : session->ref_admin_uuid;
			assert(admin_uuid[0] != '\0');

			r = db_account_device_get(admin_uuid, uuid, NULL, &count);
			assert(!r || (r && count == 1));
			if (!r)
			{
				/* -1, uuid not exist */
				buffer = rpc_keypair_append(NULL, "rid", "-1", -1);
				return buffer;
			}

			{
				int work_r;

				work_r = session_device_dref(uuid);

				/* report current device ref */
				{
					char c_ret[32];

					sprintf(c_ret, "%d", r - 1);
					buffer = rpc_keypair_append(NULL, "rid", c_ret, -1);
					return buffer;
				}
			}
		}
	}
}

rpc_buffer_t * req_play_touch(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
{
	rpc_buffer_t * buffer;
	const char * uuid;

	uuid = rpc_parser_get(req, "uuid", NULL);

	/* illegal request, record to sys log? */
	if (uuid == NULL)
	{
		buffer = rpc_keypair_append(NULL, "rid", "-255", -1);
		assert(0);
		return buffer;
	}

	{
		online_session_t * session;
		bool r;

		r = online_session_ref(fd, &session);
		if (!r)
		{
			/* -300, session expired */
			buffer = rpc_keypair_append(NULL, "rid", "-300", -1);
			return buffer;
		}

		/* check device weather ref to admin */
		{
			int count;
			const char * admin_uuid;

			admin_uuid = session->is_admin ? session->basic.uuid : session->ref_admin_uuid;
			assert(admin_uuid[0] != '\0');

			r = db_account_device_get(admin_uuid, uuid, NULL, &count);
			assert(!r || (r && count == 1));
			if (!r)
			{
				/* -1, uuid not exist */
				buffer = rpc_keypair_append(NULL, "rid", "-1", -1);
				return buffer;
			}

			{
				device_status_t status;
				int work_r;

				work_r = session_device_touch(uuid, &status);

				/* report current device ref */
				{
					char c_ret[32];

					sprintf(c_ret, "%d", work_r);
					buffer = rpc_keypair_append(NULL, "rid", c_ret, -1);
				}

				/* if works */
				if (work_r > 0)
				{
					/* device full stream path */
					buffer = rpc_keypair_append(buffer, "path", status.fullpath, -1);

					/* device elapsed time in minute (if already started) */
					{
						char c_elapsed[32];

						sprintf(c_elapsed, "%d", status.elapsed);
						buffer = rpc_keypair_append(buffer, "elapsed", c_elapsed, -1);
					}
				}

				return buffer;
			}
		}
	}
}