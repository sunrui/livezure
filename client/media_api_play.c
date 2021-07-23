/*
 * media module api logout processor
 *
 * @author rui.sun 2012-7-6 Fri 23:48
 */
#include "media_api.h"
#include "media_api_public.h"
#include "rpc_declare.h"
#include "rpc_request.h"
#include "rpc_buffer.h"
#include "rpc_keypair.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

/*
 * add a new device and try to work
 *
 * @rid
 *    ref success start ok and returned current ref
 *     -1 plugin not support
 *     -2 could not created working folder
 *     -3 login to device failed.
 *     -4 play device failed
 *     -5 internal error[check segmenter_api.c]
 *
 * @return
 *     mc_ok                         logout ok
 *     mc_server_not_response        server no response
 *     mc_syntax_error               syntax error
 *     mc_internal_error             internal error
 */
mc_status mc_play_device(const mc_session_t * session, const char * uuid, int * rid)
{
	rpc_handle_t * handle;
	rpc_req_t * req;
	rpc_buffer_t * in;
	mc_status status;
	int ret;

	if (session == NULL || uuid == NULL || rid == NULL)
	{
		status = mc_syntax_error;
		assert(0);
		return status;
	}

	*rid = -5;
	in = NULL;
	in = rpc_keypair_append(in, "uuid", uuid, -1);
	handle = rpc_request_new(rpc_id_play, rpc_id_play_device, rpc_id_encrypt, in);
	rpc_buffer_free(&in);

	if (handle == NULL)
	{
		status = mc_server_not_response;
		goto ret_clean;
	}

	ret = rpc_request_wait(handle, RPC_REQUEST_DEF_TIMEOUT);
	if (!rpc_request_ok(handle, &req))
	{
		status = mc_server_not_response;
		goto ret_clean;
	}

	{
		int * _rid = rid;
		int rid;

		check_rid(rid);
		*_rid = rid;
	}

	status = mc_ok;

ret_clean:
	assert(status != mc_syntax_error && status != mc_internal_error);
	rpc_request_destroy(&handle);
	return status;
}

/*
 * stop current device and dref, device will be stop when no ref longer.
 *
 * @rid
 *   ref return current ref, 0 is device stopped
 *    -1 device uuid not valid
 *
 * @return
 *     mc_ok                         logout ok
 *     mc_server_not_response        server no response
 *     mc_syntax_error               syntax error
 *     mc_internal_error             internal error
 */
mc_status mc_play_stop(const mc_session_t * session, const char * uuid, int * rid)
{
	rpc_handle_t * handle;
	rpc_req_t * req;
	rpc_buffer_t * in;
	mc_status status;
	int ret;

	if (session == NULL || uuid == NULL || rid == NULL)
	{
		status = mc_syntax_error;
		assert(0);
		return status;
	}

	*rid = -5;
	in = NULL;
	in = rpc_keypair_append(in, "uuid", uuid, -1);
	handle = rpc_request_new(rpc_id_play, rpc_id_play_stop, rpc_id_encrypt, in);
	rpc_buffer_free(&in);

	if (handle == NULL)
	{
		status = mc_server_not_response;
		goto ret_clean;
	}

	ret = rpc_request_wait(handle, RPC_REQUEST_DEF_TIMEOUT);
	if (!rpc_request_ok(handle, &req))
	{
		status = mc_server_not_response;
		goto ret_clean;
	}

	{
		int * _rid = rid;
		int rid;

		check_rid(rid);
		*_rid = rid;
	}

	status = mc_ok;

ret_clean:
	assert(status != mc_syntax_error && status != mc_internal_error);
	rpc_request_destroy(&handle);
	return status;
}

/*
 * touch a device and check whether already work
 *
 * @rid
 *   ref device is current work status
 *     0 device is not work
 *    -1 device not exist
 *
 * @status
 *    just valid when in work status
 *
 * @return
 *     mc_ok                         logout ok
 *     mc_server_not_response        server no response
 *     mc_syntax_error               syntax error
 *     mc_internal_error             internal error
 */
mc_status mc_play_touch(const mc_session_t * session, const char * uuid, int * rid, device_status_t * device)
{
	rpc_handle_t * handle;
	rpc_req_t * req;
	rpc_buffer_t * in;
	mc_status status;
	int ret;

	if (session == NULL || uuid == NULL || rid == NULL)
	{
		status = mc_syntax_error;
		assert(0);
		return status;
	}

	memset(device, 0, sizeof(device_status_t));
	*rid = -5;

	in = NULL;
	in = rpc_keypair_append(in, "uuid", uuid, -1);
	handle = rpc_request_new(rpc_id_play, rpc_id_play_touch, rpc_id_encrypt, in);
	rpc_buffer_free(&in);

	if (handle == NULL)
	{
		status = mc_server_not_response;
		goto ret_clean;
	}

	ret = rpc_request_wait(handle, RPC_REQUEST_DEF_TIMEOUT);
	if (!rpc_request_ok(handle, &req))
	{
		status = mc_server_not_response;
		goto ret_clean;
	}

	{
		int * _rid = rid;
		int rid;

		check_rid(rid);
		*_rid = rid;

		if (rid > 0)
		{
			const char * path;
			int elapse;

			path = rpc_parser_get(req, "path", NULL);
			assert(path != uuid);
			strncpy(device->fullpath, path ? path : "", sizeof(device->fullpath) - 1);

			{
				const char * c_elapse;

				c_elapse = rpc_parser_get(req, "elapse", NULL);
				elapse = atoi(c_elapse ? c_elapse : "0");
			}

			device->elapsed = elapse;
		}
	}

	status = mc_ok;

ret_clean:
	assert(status != mc_syntax_error && status != mc_internal_error);
	rpc_request_destroy(&handle);
	return status;
}
