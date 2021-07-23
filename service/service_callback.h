/*
 * service callback
 *
 * @author rui.sun 2012-7-11 Wed 4:55
 */
#ifndef SERVICE_CALLBACK_H
#define SERVICE_CALLBACK_H

#include "service_session.h"

/*
 * notify on user login
 */
void cb_on_login(online_session_t * session);

/*
 * notify on user logout
 */
void cb_on_logout(online_session_t * session);

/*
 * notify on user disconnect
 */
void cb_on_disconn(online_session_t * session);

/*
 * notify on user add
 */
void cb_on_create(online_session_t * session);

/*
 * notify on user delete
 */
void cb_on_delete(online_session_t * session);

#endif