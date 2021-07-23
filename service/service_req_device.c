/*
 * client request version processor
 *
 * update 2012-6-17 0:01 rui.sun
 */
#include "rpc_keypair.h"
#include "rpc_parser.h"
#include "rpc_util.h"

#include "service_session.h"
#include "db_account.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define req_device_basic_copy(x) { \
	const char * __##x; \
	__##x = rpc_parser_get(req, "" #x "", NULL); \
	if (__##x == NULL) goto exit_syntax_error; \
	strncpy(device.basic.##x, __##x, sizeof(device.basic.##x) - 1); \
	}

#define req_device_basic_set(x)  { \
	const char * __##x; \
	__##x = rpc_parser_get(req, "" #x "", NULL); \
	if (__##x == NULL) goto exit_syntax_error; \
	device.basic.##x = atoi(__##x);\
	}

#define req_device_copy(x) { \
	const char * __##x; \
	__##x = rpc_parser_get(req, "" #x "", NULL); \
	if (__##x == NULL) goto exit_syntax_error; \
	strncpy(device.##x, __##x, sizeof(device.##x) - 1); \
	}

#define req_device_if_copy(x) { \
	const char * __##x; \
	__##x = rpc_parser_get(req, "" #x "", NULL); \
	if (__##x == NULL) __##x = ""; \
	strncpy(device.##x, __##x, sizeof(device.##x) - 1); \
	}

#define req_device_set(x)  { \
	const char * __##x; \
	__##x = rpc_parser_get(req, "" #x "", NULL); \
	if (__##x == NULL) goto exit_syntax_error; \
	device.##x = atoi(__##x);\
	}

rpc_buffer_t * req_device_create(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
{
	rpc_buffer_t * buffer;
	db_account_device_t device;
	online_session_t * session;
	const char * cur_uuid;
	bool r;

	{
		r = online_session_ref(fd, &session);
		if (!r)
		{
			/* -300, session expired */
			buffer = rpc_keypair_append(NULL, "rid", "-300", -1);
			return buffer;
		}

		{
			bool b_is_admin;

			r = db_account_check_user_exist(session->basic.uuid, &b_is_admin, NULL);
			if (!b_is_admin)
			{
				online_session_dref(session);
				/* -1, no permission */
				buffer = rpc_keypair_append(NULL, "rid", "-1", -1);
				return buffer;
			}
		}

		online_session_dref(session);
		cur_uuid = session->basic.uuid;
	}

	memset(&device, 0, sizeof(db_account_device_t));

	req_device_basic_copy(uuid);
	req_device_basic_copy(plugin_name);
	req_device_basic_copy(ip);
	req_device_basic_set(port);
	req_device_basic_copy(user);
	req_device_basic_copy(pwd);
	req_device_basic_set(channel);

	r = db_account_device_check_exist(cur_uuid, device.basic.ip, device.basic.port, device.basic.channel);
	if (r)
	{
		/* -2, session exist */
		buffer = rpc_keypair_append(NULL, "rid", "-2", -1);
		return buffer;
	}

	r = db_account_device_get(cur_uuid, device.basic.uuid, NULL, NULL);
	if (r)
	{
		/* -2, uuid exist */
		buffer = rpc_keypair_append(NULL, "rid", "-2", -1);
		return buffer;
	}

	/* will random set this in server? */
	req_device_basic_copy(seg_folder);
	req_device_basic_set(seg_in_count);
	req_device_basic_set(seg_per_sec);

	r = db_account_device_workpath_valid(device.basic.seg_folder);
	if (r)
	{
		/* -3, work folder exist */
		buffer = rpc_keypair_append(NULL, "rid", "-3", -1);
		return buffer;
	}

	req_device_if_copy(nickname);
	req_device_if_copy(comment);

	req_device_set(is_top_level);

	if (!device.is_top_level)
	{
		req_device_copy(ref_parent_uuid);
	}

	assert(cur_uuid != NULL);
	r = db_account_device_add(cur_uuid, &device, NULL);
	if (!r)
	{
		/* -256, internal error */
		buffer = rpc_keypair_append(NULL, "rid", "-256", -1);
		assert(0);
		return buffer;
	}

	/* 0, ok */
	buffer = rpc_keypair_append(NULL, "rid", "0", -1);
	return buffer;

exit_syntax_error:
	/* -255, syntax error */
	buffer = rpc_keypair_append(NULL, "rid", "-255", -1);
	assert(0);
	return buffer;
}

rpc_buffer_t * req_device_delete(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
{
	rpc_buffer_t * buffer;
	online_session_t * session;
	const char * cur_uuid;
	const char * del_uuid;
	bool r;

	del_uuid = rpc_parser_get(req, "uuid", NULL);
	if (del_uuid == NULL)
	{
		/* -255, syntax error */
		buffer = rpc_keypair_append(NULL, "rid", "-255", -1);
		assert(0);
		return buffer;
	}

	{
		r = online_session_ref(fd, &session);
		if (!r)
		{
			/* -300, session expired */
			buffer = rpc_keypair_append(NULL, "rid", "-300", -1);
			return buffer;
		}

		{
			bool b_is_admin;

			r = db_account_check_user_exist(session->basic.uuid, &b_is_admin, NULL);
			if (!b_is_admin)
			{
				online_session_dref(session);
				/* -1, no permission */
				buffer = rpc_keypair_append(NULL, "rid", "-1", -1);
				return buffer;
			}
		}

		online_session_dref(session);
		cur_uuid = session->basic.uuid;
	}

	r = db_account_device_get(cur_uuid, del_uuid, NULL, NULL);
	if (!r)
	{
		/* -2, no exist */
		buffer = rpc_keypair_append(NULL, "rid", "-2", -1);
		return buffer;
	}

	r = db_account_device_del(cur_uuid, del_uuid);
	if (!r)
	{
		/* -256, internal error */
		buffer = rpc_keypair_append(NULL, "rid", "-256", -1);
		assert(0);
		return buffer;
	}

	/* 0, ok */
	buffer = rpc_keypair_append(NULL, "rid", "0", -1);
	return buffer;
}

rpc_buffer_t * req_device_list(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
{
	rpc_buffer_t * buffer;
	bool r;

	{
		online_session_t * session;

		r = online_session_ref(fd, &session);
		if (!r)
		{
			/* -300, session expired */
			buffer = rpc_keypair_append(NULL, "rid", "-300", -1);
			return buffer;
		}

		{
			bool b_is_admin;

			r = db_account_check_user_exist(session->basic.uuid, &b_is_admin, NULL);
			if (!r)
			{
				online_session_dref(session);
				/* -256, internal error */
				buffer = rpc_keypair_append(NULL, "rid", "-256", -1);
				assert(0);
				return buffer;
			}

			if (!b_is_admin)
			{
				online_session_dref(session);
				/* -1, no permission */
				buffer = rpc_keypair_append(NULL, "rid", "-1", -1);
				return buffer;
			}
		}

		{
			db_account_device_t ** device;
			int count;
			bool r;
			int index;

			r = db_account_device_get(session->basic.uuid, NULL, &device, &count);

			if (!r)
			{
				online_session_dref(session);
				/* 0, list success */
				buffer = rpc_keypair_append(NULL, "rid", "0", -1);
				buffer = rpc_keypair_append(buffer, "count", "0", -1);

				return buffer;
			}

			/* 0, list success */
			buffer = rpc_keypair_append(NULL, "rid", "0", -1);

			{
				char c_count[32];

				sprintf(c_count, "%d", count);
				buffer = rpc_keypair_append(buffer, "count", c_count, -1);
			}

			for (index = 0; index < count; index++)
			{
				buffer = rpc_keypair_append(buffer, "uuid", device[index]->basic.uuid, -1);
				buffer = rpc_keypair_append(buffer, "plugin_name", device[index]->basic.plugin_name, -1);
				buffer = rpc_keypair_append(buffer, "ip", device[index]->basic.ip, -1);

				{
					char c_port[32];

					sprintf(c_port, "%d", device[index]->basic.ip);
					buffer = rpc_keypair_append(buffer, "port", device[index]->basic.ip, -1);
				}

				buffer = rpc_keypair_append(buffer, "user", device[index]->basic.user, -1);
				buffer = rpc_keypair_append(buffer, "pwd", device[index]->basic.pwd, -1);

				{
					char c_channel[32];

					sprintf(c_channel, "%d", device[index]->basic.channel);
					buffer = rpc_keypair_append(buffer, "channel", c_channel, -1);
				}

				buffer = rpc_keypair_append(buffer, "seg_folder", device[index]->basic.seg_folder, -1);

				{
					char c_seg_in_count[32];

					sprintf(c_seg_in_count, "%d", device[index]->basic.seg_in_count);
					buffer = rpc_keypair_append(buffer, "seg_in_count", c_seg_in_count, -1);
				}

				{
					char c_seg_per_sec[32];

					sprintf(c_seg_per_sec, "%d", device[index]->basic.seg_per_sec);
					buffer = rpc_keypair_append(buffer, "seg_per_sec", c_seg_per_sec, -1);
				}

				buffer = rpc_keypair_append(buffer, "created_time", device[index]->basic.created_time, -1);

				buffer = rpc_keypair_append(buffer, "nickname", device[index]->nickname, -1);
				buffer = rpc_keypair_append(buffer, "comment", device[index]->comment, -1);

				{
					char c_is_top_level[32];

					sprintf(c_is_top_level, "%d", device[index]->is_top_level);
					buffer = rpc_keypair_append(buffer, "is_top_level", c_is_top_level, -1);
				}

				buffer = rpc_keypair_append(buffer, "ref_parent_uuid", device[index]->ref_parent_uuid, -1);
			}

			online_session_dref(session);
			db_account_device_free(&device, count);
			return buffer;
		}
	}
}