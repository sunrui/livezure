/*
 * rpc request used for client2server only
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2012-8-14 Tue 18:41
 */
#include "rpc_request.h"
#include "rpc_io.h"

#include "rpc_proto.h"
#include "rpc_parser.h"
#include "rpc_dispatch.h"
#include "rpc_client.h"
#include "rpc_alloctor.h"

#include "list.h"
#include "queue.h"
#include "posixtime.h"

#include <pthread.h>
#include <assert.h>
#include <stdio.h>

extern int rc_client_fd;

struct rpc_handle
{
	rpc_req_t * req;

	rpc_categroy category;
	rpc_categroy sub_category;

	pthread_mutex_t mutex;
	pthread_cond_t cond;
};

rpc_buffer_t * rpc_default_dispatcher(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim)
{
	rpc_handle_t * handle;
	handle = (rpc_handle_t *)opaque;
	*b_req_reclaim = 0;
	handle->req = req;
	assert(fd != 0);

	pthread_mutex_lock(&handle->mutex);
	pthread_cond_signal(&handle->cond);
	pthread_mutex_unlock(&handle->mutex);

	return 0;
}

/*
 * send a request
 *
 * @return
 *     new rpc request handle
 *     NULL if cannot notify by current fd
 */
rpc_handle_t * rpc_request_new(rpc_categroy category, rpc_categroy sub_category, rpc_crypt_type enc_type, rpc_buffer_t * in)
{
	rpc_handle_t * handle;

	{
		rpc_buffer_t * shellbuf;
		rpc_shell_t shell;
		int notify_ret;
		int fd;

		memset(&shell, 0, sizeof(shell));
		shell.magic = RPC_MAGIC_NUMBER;
		shell.version = RPC_VERSION_SIGN;
		shell.category = category;
		shell.sub_category = sub_category;
		shell.crypt = enc_type;
		shell.req = req_type_get;
		shell.reserved = 0;

		shellbuf = rpc_io_make_shell(&shell, in);

		handle = (rpc_handle_t *)rpc_calloc(1, sizeof(rpc_handle_t));
		pthread_mutex_init(&handle->mutex, NULL);
		pthread_cond_init(&handle->cond, NULL);
		handle->req = NULL;
		handle->category = category;
		handle->sub_category = sub_category;

		rpc_dispatch_add_cb(handle->category, handle->sub_category, rpc_default_dispatcher, handle);

		fd = rc_client_fd;
		/* post event message which contoured by itself */
		notify_ret = rpc_io_nodify_fd(fd, 1, shellbuf);
		rpc_buffer_free(&shellbuf);
		if (notify_ret == 0)
		{
			rpc_request_destroy(&handle);
			return NULL;
		}
	}

	return handle;
}

/*
 * wait for response returned, operation will be returned 
 *  when time reached or response returned
 *
 * @param[in]
 *     timeout wait timeout in millisecond or <0 infinite
 * @return
 *     1 response ok
 *     0 time reached
 */
int rpc_request_wait(rpc_handle_t * handle, int timeout)
{
	struct timespec ts;
	struct timeval now;
	int wait_r;
	int status;

	pthread_mutex_lock(&handle->mutex);

	wait_r = 0;
	/* we do not need to wait if we have got req already */
	if (handle->req == NULL)
	{
		if (timeout >= 0)
		{
			/* get the current time */
			gettimeofday(&now, NULL); /* add the offset to get timeout value */ 
			ts.tv_nsec = now.tv_usec * 1000 + (timeout % 1000) * 1000000; 
			ts.tv_sec = now.tv_sec + timeout / 1000;
			wait_r = pthread_cond_timedwait(&handle->cond, &handle->mutex, &ts);
		}
		else
		{
			wait_r = pthread_cond_wait(&handle->cond, &handle->mutex);
		}
	}

	pthread_mutex_unlock(&handle->mutex);
	status = wait_r != ETIMEDOUT;

	return status;
}

/*
 * check whether response is ok
 *
 * @return
 *     1 ok
 *     0 failed
 */
int rpc_request_ok(rpc_handle_t * handle, const rpc_req_t ** out_req)
{
	if (handle->req != NULL)
	{
		*out_req = handle->req;
		return 1;
	}

	return 0;
}

/*
 * destroy current rpc request and cancel
 */
void rpc_request_destroy(rpc_handle_t ** handle)
{
	if (handle != NULL && *handle != NULL)
	{
		rpc_handle_t * p_handle = *handle;

		pthread_mutex_lock(&p_handle->mutex);
		pthread_cond_signal(&p_handle->cond);
		pthread_mutex_unlock(&p_handle->mutex);

		{
			int r;
			r = rpc_dispatch_del_cb(p_handle->category, p_handle->sub_category, rpc_default_dispatcher, p_handle);
			assert(r == 1);
		}

		if (p_handle->req != NULL)
		{
			rpc_parser_free(&p_handle->req);
		}

		pthread_mutex_destroy(&p_handle->mutex);
		pthread_cond_destroy(&p_handle->cond);

		rpc_free(p_handle);
		*handle = NULL;
	}
}