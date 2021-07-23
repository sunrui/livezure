/*
 * rpc request process dispatcher
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2012-7-31 Tue 18:14
 */
#include "rpc_dispatch.h"
#include "rpc_alloctor.h"

#include "list.h"

#include <assert.h>
#include <string.h>

typedef struct rpc_dispatch_t
{
	pfn_rpc_cb cb;
	void * opaque;

	rpc_categroy category;
	rpc_categroy sub_category;

	struct list_head list;
} rpc_dispatch_t;

rpc_dispatch_t dispatch_list; /* all request dispatch list */
int dispatch_list_inited; /* whether dispatch list is init-ed */

/*
 * rpc dispatch module init
 */
void rpc_dispatch_init()
{
	if (dispatch_list_inited)
	{
		return;
	}

	INIT_LIST_HEAD(&dispatch_list.list);
	dispatch_list_inited = 1;
}

/*
 * add a dispatch callback
 */
void rpc_dispatch_add_cb(rpc_categroy category, rpc_categroy sub_category, pfn_rpc_cb rpc_cb, void * opaque)
{
	rpc_dispatch_t * dispatcher;

	dispatcher = (rpc_dispatch_t *)rpc_calloc(1, sizeof(rpc_dispatch_t));
	dispatcher->category = category;
	dispatcher->sub_category = sub_category;
	dispatcher->cb = rpc_cb;
	dispatcher->opaque = opaque;

	list_add_tail(&(*dispatcher).list, &dispatch_list.list);
}

/*
 * delete a dispatch callback
 *
 * @return
 *     1 success
 *     0 failed
 */
int rpc_dispatch_del_cb(rpc_categroy category, rpc_categroy sub_category, pfn_rpc_cb rpc_cb, void * opaque)
{
	struct list_head * pos, *q;
	rpc_dispatch_t * tmp;
	int ret;

	ret = 0;
	list_for_each_safe(pos, q, &dispatch_list.list)
	{
		tmp = list_entry(pos, rpc_dispatch_t, list);
		if (tmp->category == category && tmp->sub_category == sub_category &&
			tmp->cb == rpc_cb && tmp->opaque == opaque)
		{
			list_del(pos);
			rpc_free(tmp);
			ret = 1;
			break;
		}
	}
	assert(ret == 1);

	return ret;
}

/*
 * processor request by category id
 */
rpc_buffer_t * rpc_dispatch_processor(rpc_categroy category, rpc_categroy sub_category, int fd, rpc_req_t * req)
{
	rpc_buffer_t * response;
	struct list_head * pos;
	rpc_dispatch_t * tmp;
	int b_req_reclaim = 1;

	response = NULL;
	list_for_each(pos, &dispatch_list.list)
	{
		tmp = list_entry(pos, rpc_dispatch_t, list);
		if (tmp->category == category && tmp->sub_category == sub_category)
		{
			response = tmp->cb(tmp->opaque, fd, req, &b_req_reclaim);
			break;
		}
	}

	if (b_req_reclaim)
	{
		rpc_parser_free(&req);
	}

	return response;
}

/*
 * rpc dispatch module destroy
 */
void rpc_dispatch_destroy()
{
	struct list_head * pos, * q;
	rpc_dispatch_t * tmp;

	if (dispatch_list_inited)
	{
		list_for_each_safe(pos, q, &dispatch_list.list)
		{
			tmp = list_entry(pos, rpc_dispatch_t, list);
			list_del(pos);
			rpc_free(tmp);
		}

		dispatch_list_inited = 0;
	}
}