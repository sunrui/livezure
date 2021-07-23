/*
 * online session
 * rui.sun 2012_12_21 4:19
 */
#ifndef SERVICE_SESSION_H
#define SERVICE_SESSION_H

#include "session_online_declare.h"

/*
 * online session init
 */
void online_session_init();

/*
 * online session clean
 */
void online_session_destroy();

/*
 * add a new session to session manager
 */
bool online_session_add(online_session_t * session);

/*
 * delete an exist session from session manager
 */
bool online_session_del(int fd);

/*
 * get specified session status
 * NOTE: you must manual call online_session_dref when no more used.
 */
bool online_session_ref(int fd, /*out*/online_session_t ** session);
void online_session_dref(online_session_t * session);

bool online_session_try_get_admin(const char * ref_admin_uuid, online_session_t ** session);
bool online_session_try_get_child(const char * ref_admin_uuid, const char * uuid, online_session_t ** session);
void online_session_free(online_session_t * session);

/*
 * get number of session count
 */
int online_session_count();

/*
 * get all sessions
 *
 * @fd	must be free fd by caller
 */
void online_session_get_all(int ** fd, int * count);

#endif