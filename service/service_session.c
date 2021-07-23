/*
 * online session
 * rui.sun 2012_12_21 4:19
 */
#include "service_session.h"

#include "service_push.h"
#include "db_account.h"
#include "timer.h"
#include "pref.h"
#include "xmalloc.h"

#include "log4c_extend.h"
#include "list.h"
#include "sock.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct session_list
{
	online_session_t session;
	pref_t * ref;

	int b_mark_deleted;

	struct list_head list;
} session_list_t;

session_list_t session_list; /* all sessions that registered to server */
int session_list_inited = 0; /* whether sessions list inited */

int session_stop_sign; /* whether received a signal to stop */
int session_expire_time; /* how long time session will be expired */
timer_t * session_heartbeat_timer; /* automate sync last user heartbeat status timer */

/*
 * session check all heartbeat if is 1 or user has been disconnect
 */
void online_session_check_all_hearbeat(int * b_stop)
{
	struct list_head * pos, * q;
	session_list_t * tmp;
	bool b_need_delete;

	list_for_each_safe(pos, q, &session_list.list)
	{
		if (*b_stop) break;

		tmp = list_entry(pos, session_list_t, list);
		b_need_delete = !tmp->session.net.is_heartbeated || tmp->b_mark_deleted;

		if (!tmp->session.net.is_heartbeated)
		{
			LOGW("[SESSION] detect '%s' disconnect.", tmp->session.basic.uuid);
		}

		if (tmp->b_mark_deleted)
		{
			LOGI("[SESSION] logout [%d]'%s'-'%s'.", tmp->session.is_admin, tmp->session.basic.uuid, tmp->session.ref_admin_uuid);
		}

		if (b_need_delete)
		{
			list_del(pos);
			pref_release(&tmp->ref);
			xfree(tmp);
			return;
		}

		tmp->session.net.is_heartbeated = false;
		//{
		//	int ret_offline;

		//	/* set user session expired */
		//	ret_offline = session_offline_session(tmp->item.fd);

		//	/* if user session is not exist or user session is expired,
		//    just ignore any message about disconnection here */
		//	if (ret_offline == 0)
		//	{
		//		/* notify event_cb_on_disconn */
		//		//event_cb_on_disconn(tmp->item.fd);

		//		LOGW("heartbeat dectect %s offline..", tmp->item.uuid);
		//	}
		//}

		//tmp->item.is_heartbeat = 0;
	}
}

void online_session_beartbeat_sync_clock(void * userdata, int * b_abort_clock)
{
	/* check all session heartbeat */
	online_session_check_all_hearbeat(&session_stop_sign);
}

/*
 * online session init
 */
void online_session_init()
{
	if (session_list_inited)
	{
		return;
	}

	INIT_LIST_HEAD(&session_list.list);

	/* session will be automate expired in 30 seconds */
	session_expire_time = 30 * 1000;

	session_stop_sign = 0;
	session_heartbeat_timer = timer_create(online_session_beartbeat_sync_clock, NULL, session_expire_time, 0);

	session_list_inited = 1;
}

/*
 * online session clean
 */
void online_session_destroy()
{
	struct list_head * pos, * q;
	session_list_t * tmp;
	int ret;

	if (!session_list_inited)
	{
		return;
	}

	session_stop_sign = 1;
	ret = timer_del(session_heartbeat_timer);
	assert(ret == 0);

	list_for_each_safe(pos, q, &session_list.list)
	{
		tmp = list_entry(pos, session_list_t, list);
		list_del(pos);
		pref_release(&tmp->ref);
		xfree(tmp);
	}

	session_list_inited = 0;
}

/*
 * add a new session to session manager
 */
bool online_session_add(online_session_t * session)
{
	session_list_t * tmp;

	assert(session != NULL);

	{
		struct list_head * pos, *q;
		session_list_t * tmp;
		bool b_exist_account;

		list_for_each_safe(pos, q, &session_list.list)
		{
			tmp = list_entry(pos, session_list_t, list);
			b_exist_account = false;

			if (session->is_admin &&
				strcmp(tmp->session.basic.uuid, session->basic.uuid) == 0)
			{
				/* admin conflict */
				LOGW("[SESSION] admin '%s' login conflict.", session->basic.uuid);
				b_exist_account = true;
			}
			else if (!session->is_admin &&
				strcmp(tmp->session.ref_admin_uuid, session->ref_admin_uuid) == 0 &&
				strcmp(tmp->session.basic.uuid, session->basic.uuid) == 0)
			{
				/* user conflict */
				LOGW("[SESSION] user '%s'-'%s' login conflict.", session->ref_admin_uuid, session->basic.uuid);
				b_exist_account = true;
			}
			else if (session->net.fd == tmp->session.net.fd)
			{
				LOGW("[SESSION] the same ip address login-ed different user: old-'%s', new-'%s'.", tmp->session.basic.uuid, session->basic.uuid);
				b_exist_account = true;
			}

			if (b_exist_account)
			{
				service_push(push_login_conflict, tmp->session.net.fd);
				list_del(pos);
				pref_release(&tmp->ref);
				xfree(tmp);
			}
		}
	}

	tmp = (session_list_t *)xcalloc(1, sizeof(session_list_t));
	memcpy(&tmp->session, session, sizeof(online_session_t));
	tmp->b_mark_deleted = 0;
	pref_init(&tmp->ref);
	list_add_tail(&tmp->list, &session_list.list);

	LOGI("[SESSION] login [%d]'%s'-'%s'.", session->is_admin, session->basic.uuid, session->ref_admin_uuid);

	return true;
}

/*
 * delete an exist session from session manager
 */
bool online_session_del(int fd)
{
	struct list_head * pos;
	session_list_t * tmp;
	bool r;

	r = false;
	list_for_each(pos, &session_list.list)
	{
		tmp = list_entry(pos, session_list_t, list);
		if (tmp->session.net.fd == fd)
		{
			tmp->b_mark_deleted = 1;
			r = true;
			break;
		}
	}

	return r;
}

/*
 * get specified session status
 *  note: need to be free item when no more used.
 */
bool online_session_ref(int fd, /*out*/online_session_t **  session)
{
	struct list_head * pos;
	session_list_t * tmp;
	bool r;

	assert(session != NULL && *session != NULL);

	*session = NULL;
	r = false;

	list_for_each(pos, &session_list.list)
	{
		tmp = list_entry(pos, session_list_t, list);

		if (tmp->session.net.fd == fd)
		{
			pref_get(tmp->ref);
			*session = &tmp->session;
			r = true;
			break;
		}
	}

	return r;
}

void online_session_dref(online_session_t * session)
{
	struct list_head * pos;
	session_list_t * tmp;
	bool r;

	assert(session != NULL);

	r = false;
	list_for_each(pos, &session_list.list)
	{
		tmp = list_entry(pos, session_list_t, list);

		if (&tmp->session == session)
		{
			pref_put(tmp->ref);
			r = true;
			break;
		}
	}

	assert(r);
}

bool online_session_try_get_admin(const char * ref_admin_uuid, online_session_t ** session)
{
	bool r;

	assert(ref_admin_uuid != NULL && session != NULL);
	/* first try to get it from online session */
	{
		struct list_head * pos;
		session_list_t * tmp;

		r = false;
		list_for_each(pos, &session_list.list)
		{
			tmp = list_entry(pos, session_list_t, list);

			if (tmp->session.is_admin &&
				strcmp(tmp->session.ref_admin_uuid, ref_admin_uuid) == 0)
			{
				if (session != NULL)
				{
					online_session_t * sess;

					sess = (online_session_t *)xcalloc(1, sizeof(online_session_t));
					memcpy(sess, &tmp->session, sizeof(online_session_t));
					*session = sess;
				}

				r = true;
				break;
			}
		}

		if (r) return r;
	}

	/* get session from db */
	{
		db_account_basic_t ** account;
		int count;

		r = db_account_list_admin_get(ref_admin_uuid, &account, &count);
		if (!r) return r;

		assert(count == 1);

		if (session != NULL);
		{
			online_session_t * tmp;

			tmp = (online_session_t *)xcalloc(1, sizeof(online_session_t));
			memcpy(&tmp->basic, account[0], sizeof(db_account_basic_t));

			strcpy(tmp->ref_admin_uuid, account[0]->uuid);
			tmp->is_admin = true;
			tmp->is_online = false;

			*session = tmp;
		}
	}

	return r;
}

bool online_session_try_get_child(const char * ref_admin_uuid, const char * uuid, online_session_t ** session)
{
	bool r;

	assert(ref_admin_uuid != NULL && uuid != NULL && session != NULL);
	/* first try to get it from online session */
	{
		struct list_head * pos;
		session_list_t * tmp;

		r = false;
		list_for_each(pos, &session_list.list)
		{
			tmp = list_entry(pos, session_list_t, list);

			if (!tmp->session.is_admin &&
				strcmp(tmp->session.ref_admin_uuid, ref_admin_uuid) == 0 &&
				strcmp(tmp->session.basic.uuid, uuid) == 0)
			{
				if (session != NULL)
				{
					online_session_t * sess;

					sess = (online_session_t *)xcalloc(1, sizeof(online_session_t));
					memcpy(sess, &tmp->session, sizeof(online_session_t));
					*session = sess;
				}

				r = true;
				break;
			}
		}

		if (r) return r;
	}

	/* get session from db */
	{
		db_account_basic_t ** account;
		int count;

		r = db_account_list_child_get(ref_admin_uuid, uuid, &account, &count);
		if (!r) return r;

		assert(count == 1);

		if (session != NULL)
		{
			online_session_t * tmp;

			tmp = (online_session_t *)xcalloc(1, sizeof(online_session_t));
			memcpy(&tmp->basic, account[0], sizeof(db_account_basic_t));
			strcpy(tmp->ref_admin_uuid, ref_admin_uuid);
			tmp->is_admin = false;
			tmp->is_online = false;

			*session = tmp;
		}
	}

	return r;
}

void online_session_free(online_session_t * session)
{
	xfree(session);
}

/*
 * get number of session count
 */
int online_session_count()
{
	struct list_head * pos;
	int cnt;

	cnt = 0;
	list_for_each(pos, &session_list.list) cnt++;

	return cnt;
}

/*
 * get all sessions
 *
 * @fd	must be free fd by caller
 */
void online_session_get_all(int ** fd, int * count)
{
	struct list_head * pos;
	session_list_t * tmp;
	int * all_fd;
	int cnt = 0;

	assert(fd != NULL && *fd != NULL && count != NULL);

	list_for_each(pos, &session_list.list) cnt++;

	if (cnt == 0)
	{
		*fd = NULL;
		*count = 0;
		return;
	}

	all_fd = (int *)xcalloc(1, cnt + 1);
	cnt = 0;
	list_for_each(pos, &session_list.list)
	{
		tmp = list_entry(pos, session_list_t, list);

		all_fd[cnt] = tmp->session.net.fd;
		cnt++;
	}

	*fd = all_fd;
	*count = cnt;
}
