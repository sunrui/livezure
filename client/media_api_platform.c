/*
 * media module api logout processor
 *
 * @author rui.sun 2012-7-6 Fri 23:48
 */
#include "media_api.h"
#include "rpc_declare.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "rpc_request.h"
#include "rpc_buffer.h"
#include "rpc_keypair.h"
#include "rpc_parser.h"

#include "rpc_client.h"
#include "log4c_extend.h"

#include <stdio.h>

/*
 * get local version
 */
void mc_current_version(char version[64])
{
	assert(version != 0);

	strncpy(version, "1.0.0.0", 63);
}

/*
 * check newest version from server
 *
 * @return
 *     0   
 *     
 *     mc_syntax_error               syntax error
 */
mc_status mc_check_version(e_platfrom_id pid, version_status_t * version)
{
	rpc_handle_t * handle;
	rpc_req_t * req;
	rpc_buffer_t * in;
	mc_status status;
	char cur_ver[64];
	int ret;

	assert(version != NULL);
	memset(version, 0, sizeof(version_status_t));
	in = NULL;

	{
		char cpid[8];
		sprintf(cpid, "%d", pid);
		in = rpc_keypair_append(in, "pid", cpid, -1);
	}

	mc_current_version(cur_ver);
	in = rpc_keypair_append(in, "ver", cur_ver, -1);

	handle = rpc_request_new(rpc_id_platform, rpc_id_version_check, rpc_id_encrypt, in);
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
		const char * rid;
		const char * new_ver;
		const char * update_logs;
		const char * force_update;

		rid = rpc_parser_get(req, "rid", NULL);
		new_ver = rpc_parser_get(req, "new_ver", NULL);
		update_logs = rpc_parser_get(req, "update_log", NULL);
		force_update = rpc_parser_get(req, "force_update", NULL);

		if (rid == NULL || new_ver == NULL || update_logs == NULL || force_update == NULL)
		{
			status = mc_internal_error;
			goto ret_clean;
		}

		mc_current_version(version->cur_ver);
		strncpy(version->new_ver, new_ver, sizeof(version->new_ver) - 1);
		strncpy(version->update_logs, update_logs, sizeof(version->update_logs) - 1);
		version->b_force_update = (atoi(force_update) != 0);
	}

	status = mc_ok;

ret_clean:
	assert(status != mc_syntax_error && status != mc_internal_error);
	rpc_request_destroy(&handle);
	return status;
}