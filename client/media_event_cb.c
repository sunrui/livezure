/*
 * media event callback
 *
 * @author rui.sun 2012-7-28 Sta 20:24 Olympic first days
 */
#include "media_api.h"
#include "rpc_declare.h"

#include "timer.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "rpc_buffer.h"
#include "rpc_parser.h"
#include "log4c_extend.h"

rpc_buffer_t * media_rpc_event_login_dispatcher(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
{
#if defined(_DEBUG) || defined(DEBUG)
	{
		int count;
		count = rpc_parser_count(req);
		LOGW("media_rpc_event_login_dispatcher:");
		{
			int index;
			for (index = 0; index < count; index++)
			{
				char * key, * value;
				int r;

				r = rpc_parser_index(req, index, &key, &value, NULL);
				LOGD("key = %s, value = %s.", key, value);
				assert(r == 1);
			}
		}
	}
#endif

	return NULL;
}

rpc_buffer_t * media_rpc_event_logout_dispatcher(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
{
#if defined(_DEBUG) || defined(DEBUG)
	{
		int count;
		count = rpc_parser_count(req);
		LOGW("media_rpc_event_logout_dispatcher:");
		{
			int index;
			for (index = 0; index < count; index++)
			{
				char * key, * value;
				int r;

				r = rpc_parser_index(req, index, &key, &value, NULL);
				LOGD("key = %s, value = %s.", key, value);
				assert(r == 1);
			}
		}
	}
#endif

	return NULL;
}

rpc_buffer_t * media_rpc_event_discon_dispatcher(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
{
#if defined(_DEBUG) || defined(DEBUG)
	{
		int count;
		count = rpc_parser_count(req);
		LOGW("media_rpc_event_discon_dispatcher:");
		{
			int index;
			for (index = 0; index < count; index++)
			{
				char * key, * value;
				int r;

				r = rpc_parser_index(req, index, &key, &value, NULL);
				LOGD("key = %s, value = %s.", key, value);
				assert(r == 1);
			}
		}
	}
#endif

	return NULL;
}

rpc_buffer_t * media_rpc_event_update_dispatcher(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
{
#if defined(_DEBUG) || defined(DEBUG)
	{
		int count;
		count = rpc_parser_count(req);
		LOGW("media_rpc_event_update_dispatcher:");
		{
			int index;
			for (index = 0; index < count; index++)
			{
				char * key, * value;
				int r;

				r = rpc_parser_index(req, index, &key, &value, NULL);
				LOGD("key = %s, value = %s.", key, value);
				assert(r == 1);
			}
		}
	}
#endif

	return NULL;
}

rpc_buffer_t * media_rpc_event_login_repeat_dispatcher(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
{
#if defined(_DEBUG) || defined(DEBUG)
	{
		int count;
		count = rpc_parser_count(req);
		LOGW("media_rpc_event_login_repeat_dispatcher:");
		{
			int index;
			for (index = 0; index < count; index++)
			{
				char * key, * value;
				int r;

				r = rpc_parser_index(req, index, &key, &value, NULL);
				LOGD("key = %s, value = %s.", key, value);
				assert(r == 1);
			}
		}
	}
#endif

	return NULL;
}
//
//
///*
// * register server event callback
// */
//void mc_register_cb(const mc_session_t * session, mc_event_cb cb, void * userdata)
//{
//	session->ev_cb = cb;
//	session->ev_userdata = userdata;
//}