/*
 * media module api logout processor
 *
 * @author rui.sun 2012-7-6 Fri 23:48
 */
#include "media_api.h"
#include "rpc_declare.h"
#include "media_api_public.h"

#include "rpc_request.h"
#include "rpc_buffer.h"
#include "rpc_keypair.h"
#include "rpc_client.h"

#include "log4c_extend.h"
#include "guid.h"

#include <xmalloc.h>

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

/*
 * add a new device, user must be a admin.
 *
 * NOTE:
 *     device.basic.uuid will be ignore and automate generate by internal.
 *
 * @uuid
 *     uuid will be automate generate by internal
 *
 * @return
 *     mc_ok                         logout ok
 *     mc_server_not_response        server no response
 *     mc_no_permission              no permission
 *     mc_uuid_exist                 uuid exist
 *     mc_workfolder_exist           work folder exist
 *     mc_syntax_error               syntax error
 *     mc_internal_error             internal error
 *     mc_session_expired            session expired
 */
mc_status mc_device_add(const mc_session_t * session, const db_account_device_t * device, /*out*/char uuid[38])
{
	rpc_handle_t * handle;
	rpc_req_t * req;
	rpc_buffer_t * in;
	mc_status status;
	char guid[39];
	int ret;
	
	if (session == NULL || device == NULL || uuid == NULL ||
		device->basic.plugin_name[0] == '\0' ||
		device->basic.ip[0] == '\0' ||
		device->basic.port <= 0 ||
		device->basic.user[0] == '\0' ||
		device->basic.pwd[0] == '\0' ||
		device->basic.channel < 0 ||
		device->basic.seg_folder[0] == '\0' ||
		device->basic.seg_in_count < 0 ||
		device->basic.seg_per_sec < 0 ||
		(device->is_top_level != 0 &&
		device->is_top_level != 1))
	{
		status = mc_syntax_error;
		assert(0);
		return status;
	}

	if (!device->is_top_level && device->ref_parent_uuid[0] == '\0')
	{
		LOGW("ref parent id cannot be null if device is not a top level.");
		status = mc_syntax_error;
		assert(0);
		return status;
	}
	
	*uuid = '\0';
	in = NULL;

	{
		guid_generate(guid);
		in = rpc_keypair_append(in, "uuid", guid, -1);
	}

	in = rpc_keypair_append(in, "plugin_name", device->basic.plugin_name, -1);
	in = rpc_keypair_append(in, "ip", device->basic.ip, -1);
	
	{
		char c_port[32];

		sprintf(c_port, "%d", device->basic.port);
		in = rpc_keypair_append(in, "port", c_port, -1);
	}

	in = rpc_keypair_append(in, "user", device->basic.user, -1);
	in = rpc_keypair_append(in, "pwd", device->basic.pwd, -1);

	{
		char c_channel[32];

		sprintf(c_channel, "%d", device->basic.channel);
		in = rpc_keypair_append(in, "channel", c_channel, -1);
	}

	in = rpc_keypair_append(in, "seg_folder", device->basic.seg_folder, -1);

	{
		char c_seg_in_count[32];

		sprintf(c_seg_in_count, "%d", device->basic.seg_in_count);
		in = rpc_keypair_append(in, "seg_in_count", c_seg_in_count, -1);
	}

	{
		char c_seg_per_sec[32];

		sprintf(c_seg_per_sec, "%d", device->basic.seg_per_sec);
		in = rpc_keypair_append(in, "seg_per_sec", c_seg_per_sec, -1);
	}

	if (device->nickname[0] != '\0')
	{
		in = rpc_keypair_append(in, "nickname", device->nickname, -1);
	}

	if (device->comment[0] != '\0')
	{
		in = rpc_keypair_append(in, "comment", device->comment, -1);
	}

	in = rpc_keypair_append(in, "is_top_level", device->is_top_level ? "1" : "0", -1);

	if (!device->is_top_level)
	{
		in = rpc_keypair_append(in, "ref_parent_uuid", device->ref_parent_uuid, -1);
	}

	handle = rpc_request_new(rpc_id_device, rpc_id_device_create, rpc_id_encrypt, in);
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
		int rid;

		check_rid(rid);

		switch (rid)
		{
		case 0:
			strcpy(uuid, guid);
			status = mc_ok;
			break;
		case -1 :
			status = mc_no_permission;
			break;
		case -2:
			status = mc_uuid_exist;
			break;
		case -3:
			status = mc_workfolder_exist;
			break;
		case -300:
			status = mc_session_expired;
			break;
		default:
			status = mc_internal_error;
			break;
		}
	}

ret_clean:
	assert(status != mc_syntax_error && status != mc_internal_error);
	rpc_request_destroy(&handle);
	return status;
}

/*
 * delete device, user must be a admin.
 *
 * @return
 *     mc_ok                         logout ok
 *     mc_no_permission              no permission
 *     mc_uuid_not_exist             uuid not exist
 *     mc_syntax_error               syntax error
 *     mc_internal_error             internal error
 *     mc_session_expired            session expired
 */
mc_status mc_device_del(const mc_session_t * session, const char * uuid)
{
	rpc_handle_t * handle;
	rpc_req_t * req;
	rpc_buffer_t * in;
	mc_status status;
	int ret;

	if (session == NULL || uuid == NULL || uuid[0] == '\0')
	{
		status = mc_syntax_error;
		return status;
	}

	in = NULL;
	in = rpc_keypair_append(in, "uuid", uuid, -1);

	handle = rpc_request_new(rpc_id_device, rpc_id_device_delete, rpc_id_encrypt, in);
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
		int rid;

		check_rid(rid);

		switch (rid)
		{
		case 0:
			status = mc_ok;
			break;
		case -1 :
			status = mc_no_permission;
			break;
		case -2:
			status = mc_uuid_not_exist;
			break;
		case -300:
			status = mc_session_expired;
			break;
		default:
			status = mc_internal_error;
			break;
		}
	}

ret_clean:
	assert(status != mc_syntax_error && status != mc_internal_error);
	rpc_request_destroy(&handle);
	return status;
}

/*
 * list device, user must be a admin.
 *
 * @return
 *     mc_ok                         logout ok
 *     mc_server_not_response        server no response
 *     mc_no_permission              no permission
 *     mc_syntax_error               syntax error
 *     mc_internal_error             internal error
 *     mc_session_expired            session expired
 */
mc_status mc_device_list(const mc_session_t * session, db_account_device_t *** device, int * count)
{
	rpc_handle_t * handle;
	rpc_req_t * req;
	mc_status status;
	int ret;

	if (session == NULL || device == NULL || count == NULL)
	{
		status = mc_syntax_error;
		return status;
	}

	*device = NULL;
	*count = 0;

	handle = rpc_request_new(rpc_id_device, rpc_id_device_list, rpc_id_encrypt, NULL);
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
		int rid;

		check_rid(rid);

		switch (rid)
		{
		case 0:
			{
				int * _count;

				_count = count;
				{
					int count;
					check_rid(count);
					status = mc_ok;

					if (count == 0)
					{
						goto ret_clean;
					}

					*_count = count;
				}

				{
					db_account_device_t ** dev;
					int req_count;
					int index;
					int dev_index;
					int r;

					dev = (db_account_device_t **)xcalloc(1, (*_count) * sizeof(db_account_device_t *));
					req_count = rpc_parser_count(req);
					dev_index = 0;
					assert(req_count > 2);

					for (index = 0; index < req_count; index++)
					{
						char * key;
						char * value;

						r = rpc_parser_index(req, index, &key, &value, NULL);
						assert(r);

						if (strcmp(key, "rid") == 0 || 
							strcmp(key, "count") == 0)
						{
							continue;
						}

						if (strcmp(key, "uuid") == 0)
						{
							dev[dev_index++] = (db_account_device_t *)xcalloc(1, sizeof(db_account_device_t));
							strncpy(dev[dev_index - 1]->basic.uuid, value, sizeof(dev[dev_index - 1]->basic.uuid) - 1);
							continue;
						}

						if (strcmp(key, "plugin_name") == 0)
						{
							strncpy(dev[dev_index - 1]->basic.plugin_name, value, sizeof(dev[dev_index - 1]->basic.plugin_name) - 1);
							continue;
						}

						if (strcmp(key, "ip") == 0)
						{
							strncpy(dev[dev_index - 1]->basic.ip, value, sizeof(dev[dev_index - 1]->basic.ip) - 1);
							continue;
						}

						if (strcmp(key, "plugin_name") == 0)
						{
							strncpy(dev[dev_index - 1]->basic.plugin_name, value, sizeof(dev[dev_index - 1]->basic.plugin_name) - 1);
							continue;
						}

						if (strcmp(key, "port") == 0)
						{
							dev[dev_index - 1]->basic.port = atoi(value);
							continue;
						}

						if (strcmp(key, "user") == 0)
						{
							strncpy(dev[dev_index - 1]->basic.user, value, sizeof(dev[dev_index - 1]->basic.user) - 1);
							continue;
						}

						if (strcmp(key, "pwd") == 0)
						{
							strncpy(dev[dev_index - 1]->basic.pwd, value, sizeof(dev[dev_index - 1]->basic.pwd) - 1);
							continue;
						}

						if (strcmp(key, "channel") == 0)
						{
							dev[dev_index - 1]->basic.channel = atoi(value);
							continue;
						}

						if (strcmp(key, "seg_folder") == 0)
						{
							strncpy(dev[dev_index - 1]->basic.seg_folder, value, sizeof(dev[dev_index - 1]->basic.seg_folder) - 1);
							continue;
						}

						if (strcmp(key, "seg_in_count") == 0)
						{
							dev[dev_index - 1]->basic.seg_in_count = atoi(value);
							continue;
						}

						if (strcmp(key, "seg_per_sec") == 0)
						{
							dev[dev_index - 1]->basic.seg_per_sec = atoi(value);
							continue;
						}

						if (strcmp(key, "created_time") == 0)
						{
							strncpy(dev[dev_index - 1]->basic.created_time, value, sizeof(dev[dev_index - 1]->basic.created_time) - 1);
							continue;
						}

						if (strcmp(key, "nickname") == 0)
						{
							strncpy(dev[dev_index - 1]->nickname, value, sizeof(dev[dev_index - 1]->nickname) - 1);
							continue;
						}

						if (strcmp(key, "comment") == 0)
						{
							strncpy(dev[dev_index - 1]->comment, value, sizeof(dev[dev_index - 1]->comment) - 1);
							continue;
						}

						if (strcmp(key, "is_top_level") == 0)
						{
							dev[dev_index - 1]->is_top_level, atoi(value);
							continue;
						}

						if (strcmp(key, "ref_parent_uuid") == 0)
						{
							strncpy(dev[dev_index - 1]->ref_parent_uuid, value ? value : "", sizeof(dev[dev_index - 1]->ref_parent_uuid) - 1);
							continue;
						}
					}

					*device = dev;
				}

				goto ret_clean;
			}
		case -1:
			status = mc_no_permission;
			break;
		case -255:
			status = mc_syntax_error;
			break;
		case -300:
			status = mc_session_expired;
			break;
		case -256:
		default:
			status = mc_internal_error;
			break;
		}
	}

ret_clean:
	assert(status != mc_syntax_error && status != mc_internal_error);
	rpc_request_destroy(&handle);
	return status;
}

/*
 * free all devices
 *
 * @return
 *     mc_ok                         free ok
 *     mc_syntax_error               syntax error
 */
mc_status mc_device_list_free(db_account_device_t ** device, int count)
{
	int index;

	if (device == NULL || *device == NULL || count <= 0)
	{
		return mc_syntax_error;
	}

	for (index = 0; index < count; index++)
	{
		xfree(device[index]);
	}

	xfree(device);
	return mc_ok;
}