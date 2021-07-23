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

#ifdef WIN32
#define stricmp _stricmp
#else
#define stricmp strcasecmp
#endif

rpc_buffer_t * req_account_login(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
{
	rpc_buffer_t * buffer;
	const char * pid;
	const char * uuid;
	const char * pwd;
	
	pid = rpc_parser_get(req, "pid", NULL);
	uuid = rpc_parser_get(req, "uuid", NULL);
	pwd = rpc_parser_get(req, "pwd", NULL);

	/* illegal request, record to sys log? */
	if (pid == NULL || !valid_platform(atoi(pid)) ||
		!valid_uuid(uuid) || !valid_pwd(pwd))
	{
		/* -255, syntax error */
		buffer = rpc_keypair_append(NULL, "rid", "-255", -1);
		assert(0);
		return buffer;
	}

	/* log only */
	{
		pid;
	}

	{
		char ref_admin_uuid[33];
		bool b_is_admin;
		bool r;

		r = db_account_check_user_is_ok(uuid, pwd, &b_is_admin, ref_admin_uuid);
		if (!r)
		{
			/* -1, uuid or pwd invalid */
			buffer = rpc_keypair_append(NULL, "rid", "-1", -1);
			return buffer;
		}

		/* add to service_session inst */
		{
			online_session_t new_session;
			db_account_basic_t ** basic;
			int count;

			memset(&new_session, 0, sizeof(online_session_t));

			if (b_is_admin)
			{
				r = db_account_list_admin_get(uuid, &basic, &count);
			}
			else
			{
				r = db_account_list_child_get(ref_admin_uuid, uuid, &basic, &count);
				strncpy(new_session.ref_admin_uuid, ref_admin_uuid, sizeof(new_session.ref_admin_uuid));
			}
			assert(r && count == 1);

			memcpy(&new_session.basic, basic[0], sizeof(new_session.basic));
			new_session.is_admin = b_is_admin;
			new_session.net.fd = fd;
			new_session.is_online = true;
			new_session.net.is_heartbeated = true;

			{
				char ip[17];
				int port;

				r = rpc_util_ip(fd, ip, &port);
				assert(r);

				strncpy(new_session.net.ip, ip, sizeof(new_session.net.ip));
				new_session.net.port = port;
			}

			r = online_session_add(&new_session);
			assert(r);
		}

		{
			buffer = rpc_keypair_append(NULL, "rid", "0", -1);

			{
				char c_is_admin[32];

				sprintf(c_is_admin, "%d", b_is_admin);
				buffer = rpc_keypair_append(buffer, "is_admin", c_is_admin, -1);
			}

			if (!b_is_admin)
			{
				buffer = rpc_keypair_append(buffer, "ref_admin_uuid", ref_admin_uuid, -1);
			}
		}

		return buffer;
	}
}

rpc_buffer_t * req_account_logout(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
{
	online_session_t * session;
	rpc_buffer_t * buffer;
	bool r;

	r = online_session_ref(fd, &session);
	if (!r)
	{
		/* -300, session expired */
		buffer = rpc_keypair_append(NULL, "rid", "-300", -1);
		return buffer;
	}

	online_session_dref(session);
	r = online_session_del(fd);
	assert(r);

	/* 0, logout ok */
	buffer = rpc_keypair_append(NULL, "rid", "0", -1);
	return buffer;
}

rpc_buffer_t * req_account_create(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
{
	rpc_buffer_t * buffer;
	const char * uuid;
	const char * pwd;
	const char * nickname;
	const char * comment;
	bool r;

	uuid = rpc_parser_get(req, "uuid", NULL);
	pwd = rpc_parser_get(req, "pwd", NULL);
	nickname = rpc_parser_get(req, "nickname", NULL);
	comment = rpc_parser_get(req, "comment", NULL);

	/* illegal request, record to sys log? */
	if (!valid_uuid(uuid) || !valid_pwd(pwd) ||
		!valid_nickname(nickname) || !valid_comment(comment))
	{
		/* -255, syntax error */
		buffer = rpc_keypair_append(NULL, "rid", "-255", -1);
		assert(0);
		return buffer;
	}

	if (nickname == NULL) nickname = "";
	if (comment == NULL) comment = "";

	r = db_account_check_user_exist(uuid, NULL, NULL);
	if (r)
	{
		/* -2, uuid exist */
		buffer = rpc_keypair_append(NULL, "rid", "-2", -1);
		return buffer;
	}

	{
		online_session_t * session;

		r = online_session_ref(fd, &session);
		if (!r)
		{
			/* -300, session expired */
			buffer = rpc_keypair_append(NULL, "rid", "-300", -1);
			return buffer;
		}

		if (!session->is_admin)
		{
			online_session_dref(session);
			/* -1, no permission */
			buffer = rpc_keypair_append(NULL, "rid", "-1", -1);
			return buffer;
		}

		{
			db_account_basic_t basic;

			memset(&basic, 0, sizeof(basic) - 1);
			strncpy(basic.uuid, uuid, sizeof(basic.uuid) - 1);
			strncpy(basic.pwd, pwd, sizeof(basic.pwd) - 1);
			strncpy(basic.nickname, nickname, sizeof(basic.nickname) - 1);
			strncpy(basic.comment, comment, sizeof(basic.comment) - 1);
			r = db_account_child_add(session->basic.uuid, &basic, NULL);

			if (!r)
			{
				online_session_dref(session);
				/* -256, internal error */
				buffer = rpc_keypair_append(NULL, "rid", "-256", -1);
				assert(0);
				return buffer;
			}

			online_session_dref(session);
			/* create success */
			buffer = rpc_keypair_append(NULL, "rid", "0", -1);
			return buffer;
		}
	}
}

rpc_buffer_t * req_account_modify(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
{
	rpc_buffer_t * buffer;
	const char * old_pwd;
	const char * new_pwd;
	const char * nickname;
	const char * comment;
	bool r;

	old_pwd = rpc_parser_get(req, "old_pwd", NULL);
	new_pwd = rpc_parser_get(req, "new_pwd", NULL);
	nickname = rpc_parser_get(req, "nickname", NULL);
	comment = rpc_parser_get(req, "comment", NULL);

	/* illegal request, record to sys log? */
	if (!valid_pwd(old_pwd) || !valid_pwd(new_pwd) ||
		!valid_nickname(nickname) || !valid_comment(comment))
	{
		/* -255, syntax error */
		buffer = rpc_keypair_append(NULL, "rid", "-255", -1);
		assert(0);
		return buffer;
	}

	if (nickname == NULL) nickname = "";
	if (comment == NULL) comment = "";

	{
		online_session_t * session;

		r = online_session_ref(fd, &session);
		if (!r)
		{
			/* -300, session expired */
			buffer = rpc_keypair_append(NULL, "rid", "-300", -1);
			return buffer;
		}

		r = db_account_check_user_is_ok(session->basic.uuid, old_pwd, NULL, NULL);
		if (!r)
		{
			online_session_dref(session);
			/* -1, old pwd not correct */
			buffer = rpc_keypair_append(NULL, "rid", "-1", -1);
			return buffer;
		}

		{
			db_account_basic_t basic;

			memcpy(&basic, &session->basic, sizeof(basic));
			strncpy(basic.pwd, new_pwd, sizeof(basic.pwd) - 1);
			r = db_account_update(session->basic.uuid, &basic, NULL);
			if (!r)
			{
				online_session_dref(session);
				/* -256, internal error */
				buffer = rpc_keypair_append(NULL, "rid", "-256", -1);
				assert(0);
				return buffer;
			}

			online_session_dref(session);
			/* 0, modify success */
			buffer = rpc_keypair_append(NULL, "rid", "0", -1);
			return buffer;
		}
	}
}

rpc_buffer_t * req_account_delete(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
{
	rpc_buffer_t * buffer;
	const char * check_pwd;
	const char * del_uuid;
	bool r;

	check_pwd = rpc_parser_get(req, "check_pwd", NULL);
	del_uuid = rpc_parser_get(req, "del_uuid", NULL);

	/* illegal request, record to sys log? */
	if (!valid_uuid(del_uuid) || !valid_pwd(check_pwd))
	{
		/* -255, syntax error */
		buffer = rpc_keypair_append(NULL, "rid", "-255", -1);
		assert(0);
		return buffer;
	}

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

			r = db_account_check_user_is_ok(session->basic.uuid, check_pwd, &b_is_admin, NULL);
			if (!r)
			{
				online_session_dref(session);
				/* -2, old pwd not correct */
				buffer = rpc_keypair_append(NULL, "rid", "-2", -1);
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

		if (stricmp(del_uuid, session->basic.uuid) == 0)
		{
			online_session_dref(session);
			/* -3, cannot be admin owner */
			buffer = rpc_keypair_append(NULL, "rid", "-3", -1);
			return buffer;
		}

		/* check whether current user is ref to admin */
		{
			int count;

			r = db_account_list_child_get(session->basic.uuid, del_uuid, NULL, &count);
			if (!r || count != 1)
			{
				online_session_dref(session);
				/* -4, user not exist */
				buffer = rpc_keypair_append(NULL, "rid", "-4", -1);
				return buffer;
			}

			/* cannot delete any other user if he is admin, should never happen!*/
			{
				bool b_is_admin;

				r = db_account_check_user_exist(del_uuid, &b_is_admin, NULL);
				if (r && b_is_admin)
				{
					online_session_dref(session);
					/* -256, internal error */
					buffer = rpc_keypair_append(NULL, "rid", "-256", -1);
					assert(0);
					return buffer;
				}
			}

			r = db_account_child_del(session->basic.uuid, del_uuid);
			if (!r)
			{
				online_session_dref(session);
				/* -256, internal error */
				buffer = rpc_keypair_append(NULL, "rid", "-256", -1);
				assert(0);
				return buffer;
			}

			online_session_dref(session);
			/* 0, delete success */
			buffer = rpc_keypair_append(NULL, "rid", "0", -1);
			return buffer;
		}
	}
}

rpc_buffer_t * req_account_list(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
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
			db_account_basic_t ** basic;
			int count;
			bool r;
			int index;

			r = db_account_list_child_get(session->basic.uuid, NULL, &basic, &count);
			assert(r);

			if (!r)
			{
				online_session_dref(session);
				/* -256, internal error */
				buffer = rpc_keypair_append(NULL, "rid", "-256", -1);
				assert(0);
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
				online_session_t * online;

				r = online_session_try_get_child(session->basic.uuid, basic[index]->uuid, &online);
				assert(r);
				
				if (r)
				{
					buffer = rpc_keypair_append(buffer, "uuid", online->basic.uuid, -1);
					buffer = rpc_keypair_append(buffer, "nickname", online->basic.nickname, -1);
					buffer = rpc_keypair_append(buffer, "comment", online->basic.comment, -1);
					buffer = rpc_keypair_append(buffer, "created_time", online->basic.created_time, -1);
					buffer = rpc_keypair_append(buffer, "is_online", online->is_online ? "1" : "0", -1);
					buffer = rpc_keypair_append(buffer, "ip", online->net.ip[0] == '\0' ? "0.0.0.0" : online->net.ip, -1);

					{
						char c_port[32];

						sprintf(c_port, "%d", online->net.port);
						buffer = rpc_keypair_append(buffer, "port", c_port, -1);
					}
				}
			}

			online_session_dref(session);
			db_account_list_free(&basic, count);
			return buffer;
		}
	}
}