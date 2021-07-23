/*
 * media module api create processor
 *
 * @author rui.sun 2012-7-4 Wed 23:58
 */
#include "media_api.h"
#include "rpc_declare.h"
#include "media_api_public.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "rpc_request.h"
#include "rpc_buffer.h"
#include "rpc_keypair.h"
#include "rpc_client.h"


#include "log4c_extend.h"
#include "timer.h"
#include "xmalloc.h"

#ifdef WIN32
#define stricmp _stricmp
#else
#deifne stricmp strcasecmp
#endif

/*
 * heartbeat and check the recently changed status
 *
 * @param [in]session
 *      user session which got by mc_login, use this operate must be 
 *       have 1 rights
 *
 * @return
 *     mc_ok                         heartbeat ok
 *     mc_session_expired            session expired
 */
mc_status mc_heartbeat(const mc_session_t * session)
{
	rpc_handle_t * handle;
	rpc_req_t * req;
	mc_status status;
	int ret;

	handle = rpc_request_new(rpc_id_platform, rpc_id_heartbeat, rpc_id_encrypt, NULL);

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
		case -300:
		default:
			status = mc_session_expired;
			break;
		}
	}

ret_clean:
	assert(status != mc_syntax_error && status != mc_internal_error);
	rpc_request_destroy(&handle);
	return status;
}

/*
 * heartbeat timer
 */
void heartbeart_timer_clock(void * userdata, int * b_abort_clock)
{
	mc_session_t * session;
//	mc_event_t ** ev;
	mc_status r;
	
	session = (mc_session_t *)userdata;

	r = mc_heartbeat(session);
	switch (r)
	{
	case mc_ok:
		LOGD("heartbeat ok.");
		break;
	case mc_session_expired:
	default:
		{
			mc_account_logout(&session);

			/* session not valid or expired */
			//		session->ev_cb(session->ev_userdata, r, NULL, 0, b_abort_clock);

			/* no matter user want, we will just abort event notify 
			when session is invalid or expired */
			*b_abort_clock = 1;

			LOGD("session expired.");
			break;
		}
	}
}

/*
 * login and get session instance
 *
 * @param [out]session
 *      return the malloc session if mc_ok, you need free it by
 *       mc_logout when it no longer be used
 *
 * @return
 *     mc_ok login                   ok
 *     mc_server_not_response         server not response
 *     mc_invalid_user_or_password   not valid user or password
 *     mc_syntax_error               syntax error
 *     mc_session_expired            session expired
 */
mc_status mc_account_login(const char * uuid, const char * pwd, e_platfrom_id pid, mc_session_t ** session)
{
	rpc_handle_t * handle;
	rpc_req_t * req;
	rpc_buffer_t * in;
	mc_status status;
	int ret;

	if (uuid == NULL || pwd == NULL || session == NULL)
	{
		status = mc_syntax_error;
		assert(0);
		return status;
	}

	*session = NULL;
	in = NULL;
	in = rpc_keypair_append(in, "uuid", uuid, -1);
	in = rpc_keypair_append(in, "pwd", pwd, -1);

	{
		char c_pid[8];
		
		sprintf(c_pid, "%d", pid);
		in = rpc_keypair_append(in, "pid", c_pid, -1);
	}

	handle = rpc_request_new(rpc_id_account, rpc_id_account_login, rpc_id_encrypt, in);
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
			{
				const char * ref_admin_uuid;
				bool b_is_admin;
				mc_session_t * sess;


				{
					const char * c_is_admin;

					c_is_admin = rpc_parser_get(req, "is_admin", NULL);
					if (c_is_admin == NULL)
					{
						status = mc_internal_error;
						goto ret_clean;
					}

					b_is_admin = atoi(c_is_admin) != 0;
					if (!b_is_admin)
					{
						ref_admin_uuid = rpc_parser_get(req, "ref_admin_uuid", NULL);
						if (ref_admin_uuid == NULL)
						{
							status = mc_internal_error;
							goto ret_clean;
						}
					}
				}

				sess = (mc_session_t *)xcalloc(1, sizeof(mc_session_t));
				*session = sess;

				strncpy(sess->uuid, uuid, sizeof(sess->uuid));
				sess->b_is_admin = b_is_admin;

				if (!sess->b_is_admin)
				{
					strncpy(sess->ref_admin_uuid, ref_admin_uuid, sizeof(sess->ref_admin_uuid));
				}

				{
					int tickfps = 5000; /* check heartbeat every 5 seconds */
					sess->auto_heartbeat = timer_create(heartbeart_timer_clock, sess, tickfps, 0);
				}
			}
			status = mc_ok;
			LOGI("login success");
			break;
		case -1:
			status = mc_invalid_uuid_or_pwd;
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
 * check login power id
 *
 * @return
 *     mc_ok                         get power ok
 *     mc_syntax_error               syntax error
 */
mc_status mc_login_check_power(const mc_session_t * session, bool * b_is_admin)
{
	if (session != NULL && b_is_admin != NULL)
	{
		*b_is_admin = session->b_is_admin;
		return mc_ok;
	}

	return mc_syntax_error;
}

/*
 * logout from server, session will be set to NULL if mc_ok
 *
 * @return
 *     mc_ok                         logout ok
 *     mc_server_not_response        server no response
 *     mc_session_expired            session expired
 */
mc_status mc_account_logout(mc_session_t ** session)
{
	rpc_handle_t * handle;
	rpc_req_t * req;
	mc_status status;
	int ret;

	if (session == NULL || *session == NULL)
	{
		status = mc_session_expired;
		return status;
	}

	{
		/* first stop event clock */
		if ((*session)->auto_heartbeat != NULL)
		{
			timer_del((*session)->auto_heartbeat);
		}

		xfree(*session);
		*session = NULL;
	}

	handle = rpc_request_new(rpc_id_account, rpc_id_account_logout, rpc_id_encrypt, NULL);
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
			LOGI("logout success.");
			status = mc_ok;
			break;
		case -300:
		default:
			status = mc_session_expired;
			break;
		}
	}

ret_clean:
	assert(status != mc_syntax_error && status != mc_internal_error);
	rpc_request_destroy(&handle);
	return status;
}

/*
 * create new user
 *
 * @param [in]session
 *      user session which got by mc_login, use this operate must be 
 *       have admin rights
 * @return
 *     mc_ok                         create ok
 *     mc_no_permission              no permission
 *     mc_uuid_exist                 uuid exist
 *     mc_syntax_error               syntax error
 *     mc_internal_error             internal error
 *     mc_session_expired            session expired
 */
mc_status mc_account_create(const mc_session_t * session, db_account_basic_t * child)
{
	rpc_handle_t * handle;
	rpc_req_t * req;
	rpc_buffer_t * in;
	mc_status status;
	int ret;

	if (session == NULL ||child == NULL || !valid_uuid(child->uuid) ||
		!valid_pwd(child->pwd) || !valid_nickname(child->nickname) ||
		!valid_comment(child->comment))
	{
		status = mc_syntax_error;
		return status;
	}

	in = NULL;

	in = rpc_keypair_append(in, "uuid", child->uuid, -1);
	in = rpc_keypair_append(in, "pwd", child->pwd, -1);

	if (child->nickname != NULL)
	{
		in = rpc_keypair_append(in, "nickname", child->nickname, -1);
	}

	if (child->comment != NULL)
	{
		in = rpc_keypair_append(in, "comment", child->comment, -1);
	}
	
	handle = rpc_request_new(rpc_id_account, rpc_id_account_create, rpc_id_encrypt, in);
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
		case -1:
			status = mc_no_permission;
			break;
		case -2:
			status = mc_uuid_exist;
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
 * modify user
 *
 * @return
 *     mc_ok                         login ok
 *     mc_server_not_response        server not response
 *     mc_invalid_user_or_password   not valid user or password
 *     mc_syntax_error               syntax error
 *     mc_internal_error             internal error
 *     mc_session_expired            session expired
 */
mc_status mc_account_modify(const mc_session_t * session, const char * old_pwd, db_account_basic_t * basic)
{
	rpc_handle_t * handle;
	rpc_req_t * req;
	rpc_buffer_t * in;
	mc_status status;
	int ret;

	if (session == NULL || old_pwd == NULL || basic == NULL ||
		stricmp(session->uuid, basic->uuid) != 0)
	{
		status = mc_syntax_error;
		return status;
	}

	in = NULL;
	in = rpc_keypair_append(in, "old_pwd", old_pwd, -1);
	in = rpc_keypair_append(in, "new_pwd", basic->pwd, -1);
	in = rpc_keypair_append(in, "nickname", basic->nickname, -1);
	in = rpc_keypair_append(in, "comment", basic->comment, -1);

	handle = rpc_request_new(rpc_id_account, rpc_id_account_modify, rpc_id_encrypt, in);
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

	status = mc_ok;

	{
		int rid;

		check_rid(rid);

		switch (rid)
		{
		case 0:
			status = mc_ok;
			break;
		case -1:
			status = mc_old_pwd_not_correct;
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
 * delete user
 *
 * @param [out]session
 *      return the malloc session if mc_ok, you need free it by
 *       mc_logout when it no longer be used
 *
 * @return
 *     mc_ok                         ok
 *     mc_server_not_response        server not response
 *     mc_invalid_user_or_password   not valid user or password
 *     mc_no_permission              no permission
 *     mc_delete_cannot_be_root      cannot be root
 *     mc_syntax_error               syntax error
 *     mc_internal_error             internal error
 *     mc_session_expired            session expired
 */
mc_status mc_account_delete(const mc_session_t * session, const char * old_pwd, const char * del_uuid)
{
	rpc_handle_t * handle;
	rpc_req_t * req;
	rpc_buffer_t * in;
	mc_status status;
	int ret;

	if (session == NULL || old_pwd == NULL || del_uuid == NULL)
	{
		status = mc_syntax_error;
		return status;
	}

	if (!session->b_is_admin)
	{
		status = mc_no_permission;
		return status;
	}

	in = NULL;
	in = rpc_keypair_append(in, "check_pwd", old_pwd, -1);
	in = rpc_keypair_append(in, "del_uuid", del_uuid, -1);

	handle = rpc_request_new(rpc_id_account, rpc_id_account_delete, rpc_id_encrypt, in);
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
		case -1:
			status = mc_no_permission;
			break;
		case -2:
			status = mc_old_pwd_not_correct;
			break;
		case -3:
			status = mc_delete_cannot_be_owner;
			break;
		case -4:
			status = mc_uuid_not_exist;
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
 * get all registered users
 *
 * @return
 *     mc_ok                         ok
 *     mc_server_not_response        server not response
 *     mc_no_permission              no permission
 *     mc_syntax_error               syntax error
 *     mc_internal_error             internal error
 *     mc_session_expired            session expired
 */
mc_status mc_account_list(const mc_session_t * session, online_session_t *** basic, int * count)
{
	rpc_handle_t * handle;
	rpc_req_t * req;
	mc_status status;
	int ret;

	if (session == NULL || basic == NULL || count == NULL)
	{
		status = mc_syntax_error;
		return status;
	}

	*basic = NULL;
	*count = 0;

	handle = rpc_request_new(rpc_id_account, rpc_id_account_list, rpc_id_encrypt, NULL);
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
					online_session_t ** sess;
					int req_count;
					int index;
					int sess_index;
					int r;

					sess = (online_session_t **)xcalloc(1, (*_count) * sizeof(online_session_t *));
					req_count = rpc_parser_count(req);
					sess_index = 0;
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
							sess[sess_index++] = (online_session_t *)xcalloc(1, sizeof(online_session_t));
							strncpy(sess[sess_index - 1]->basic.uuid, value, sizeof(sess[sess_index - 1]->basic.uuid) - 1);
							strncpy(sess[sess_index - 1]->ref_admin_uuid, session->uuid, sizeof(sess[sess_index - 1]->ref_admin_uuid) - 1);
							sess[sess_index - 1]->is_admin = false;
							continue;
						}

						if (strcmp(key, "nickname") == 0)
						{
							strncpy(sess[sess_index - 1]->basic.nickname, value, sizeof(sess[sess_index - 1]->basic.nickname) - 1);
							continue;
						}

						if (strcmp(key, "comment") == 0)
						{
							strncpy(sess[sess_index - 1]->basic.comment, value, sizeof(sess[sess_index - 1]->basic.comment) - 1);
							continue;
						}

						if (strcmp(key, "created_time") == 0)
						{
							strncpy(sess[sess_index - 1]->basic.created_time, value, sizeof(sess[sess_index - 1]->basic.created_time) - 1);
							continue;
						}

						if (strcmp(key, "is_online") == 0)
						{
							sess[sess_index - 1]->is_online = atoi(value);
							continue;
						}

						if (strcmp(key, "ip") == 0)
						{
							strncpy(sess[sess_index - 1]->net.ip, value, sizeof(sess[sess_index - 1]->net.ip) - 1);
						}

						if (strcmp(key, "port") == 0)
						{
							sess[sess_index - 1]->net.port = atoi(value);
							continue;
						}
					}

					*basic = sess;
				}
				
				break;
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
 * free all users
 *
 * @return
 *     mc_ok                         free ok
 *     mc_syntax_error               syntax error
 */
mc_status mc_account_list_free(online_session_t ** basic, int count)
{
	int index;

	if (basic == NULL || *basic == NULL || count <= 0)
	{
		return mc_syntax_error;
	}

	for (index = 0; index < count; index++)
	{
		xfree(basic[index]);
	}

	xfree(basic);
	return mc_ok;
}