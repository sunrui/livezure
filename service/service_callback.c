/*
 * service callback
 *
 * @author rui.sun 2012-7-11 Wed 4:55
 */
#include "service_callback.h"
#include "service_push.h"
#include "service_api.h"
#include "service_session.h"

#include <stdio.h>
#include <assert.h>
//
//extern pfn_event_cb event_cb; /* event cb instance set by set_event_cb */
//extern void * event_cb_opaque; /* event cb user data */
//
///*
// * notify to hooker which by set_event_cb in service_api.h
// */
//void event_cb_notify_hook(int type, online_session_t * session);
//
///*
// * notify on user login
// */
//void cb_on_login(online_session_t * session)
//{
//	/* handle message in service core*/
////	event_cb_notify_hook(rpc_id_event_cb_login, session);
//}
//
///*
// * notify on user logout
// */
//void cb_on_logout(online_session_t * session)
//{
//	/* handle message in service core*/
////	event_cb_notify_hook(rpc_id_event_cb_logout, session);
//}
//
///*
// * notify on user disconnect
// */
//void cb_on_disconn(online_session_t * session)
//{
//	/* handle message in service core*/
////	event_cb_notify_hook(rpc_id_event_cb_discon, session);
//}
//
///*
// * notify on user add
// */
//void cb_on_create(online_session_t * session)
//{
//	/* handle message in service core*/
////	event_cb_notify_hook(rpc_id_event_cb_create, session);
//}
//
///*
// * notify on user delete
// */
//void cb_on_delete(online_session_t * session)
//{
//	/* handle message in service core*/
////	event_cb_notify_hook(rpc_id_event_cb_delete, session);
//}
//
///*
// * user login from another place
// */
//void event_cb_on_login_repeat(online_session_t * session)
//{
//	/* handle message in service core*/
////	event_cb_notify_hook(rpc_id_event_cb_repeat, session);
//}
//
///*
// * notify to hooker which by set_event_cb in service_api.h
// */
//void event_cb_notify_hook(int type, online_session_t * session)
//{
//	assert(session != NULL);
//
//	/* send event to client */
////	service_push(type, session);
//
//	/* send event to server callback */
//	if (event_cb != NULL)
//	{
//		event_cb(event_cb_opaque, type, session);
//	}
//}