/*
 * rpc io header
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2012-7-31 Tue 10:41
 */
#include "rpc_io.h"
#include "rpc_proto.h"
#include "rpc_parser.h"
#include "rpc_dispatch.h"
#include "rpc_alloctor.h"
#include "rpc_ringbuffer.h"
#include "rpc_threadpool.h"

#include "list.h"
#include "queue.h"

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define RPC_IO_DEF_GROW_SIZE 4 * 1024 /* 4k */

typedef struct rpc_io
{
	rpc_queue_t * send;
	rpc_rb_t * recv;

	/* specified whether last shell is not received enough,
	shell will be NULL if last shell is welled proceed. */
	rpc_shell_t shell;
	int shell_cached; /* whether shell is cached */

	int fd;
	void * extra;

	struct list_head list;
} rpc_io_t;

typedef struct rpc_io_send
{
	rpc_io_send_cb send;
	pthread_t thread;
	rpc_queue_t * signal_queue;
	int abort;
} rpc_io_send_t;

typedef struct rpc_io_task
{
	int fd;
	rpc_io_t * child;
	rpc_shell_t * shell;
	rpc_buffer_t * in;
} rpc_io_task_t;

static rpc_io_t io_list; /* all rpc io that received from server */
static int io_inited;
static rpc_io_send_t io_send;
static rpc_tp_t * io_processor_pool;

void * rpc_io_routine(void * param)
{
	rpc_io_send_t * send = (rpc_io_send_t *)param;

	while (!send->abort)
	{
		rpc_io_t * io;
		int got_ret;

		got_ret = rpc_queue_get(send->signal_queue, (void **)&io, -1 /* block way */);
		if (got_ret == 1)
		{
			const char * buf;
			int bufsize;
			int sendsize;

			{
				/* have process one or more request */
				rpc_buffer_t * out;
				int got_ret;

				while (got_ret = rpc_io_get(io->fd, &out, 0 /* no block */) == 1)
				{
					buf = rpc_buffer_get_buffer(out);
					bufsize = rpc_buffer_get_size(out);
					sendsize = send->send(io->fd, buf, bufsize, io->extra);
					rpc_buffer_free(&out);
				}
			}
		}
		else
		{
			break;
		}
	}

	return NULL;
}

/*
 * find a rpc_io by fd
 */
rpc_io_t * rpc_io_find(int fd)
{
	struct list_head * pos;
	rpc_io_t * tmp;

	tmp = NULL;
	list_for_each(pos, &io_list.list)
	{
		tmp = list_entry(pos, rpc_io_t, list);
		if (tmp->fd == fd)
		{
			return tmp;
		}
	}

	return NULL;
}

/*
 * rpc io module init
 */
void rpc_io_init(rpc_io_send_cb send_cb, int nprocessors)
{
	if (io_inited)
	{
		return;
	}

	assert(send_cb != NULL);
	INIT_LIST_HEAD(&io_list.list);
	io_send.send = send_cb;
	io_send.abort = 0;
	io_send.signal_queue = rpc_queue_new();
	pthread_create(&io_send.thread, NULL, rpc_io_routine, &io_send);
	io_processor_pool = rpc_tp_create(nprocessors);

	io_inited = 1;
}

void * rpc_io_processor(void * opaque)
{
	rpc_io_task_t * task;
	int fd;
	rpc_io_t * child;
	rpc_shell_t * shell;
	rpc_buffer_t * in;

	rpc_req_t * req;
	rpc_buffer_t * res_rpc;
	int ret;

	task = (rpc_io_task_t *)opaque;
	assert(task != NULL);
	fd = task->fd;
	child = task->child;
	shell = task->shell;
	in = task->in;
	rpc_free(task);

	assert(child != NULL && shell != NULL);

	/* some request maybe no have a body, such as heartbeat */
	ret = 0;
	req = (in != NULL) ? rpc_parser_new(in) : NULL;
	if (in != NULL) rpc_buffer_free(&in);
	res_rpc = rpc_dispatch_processor(shell->category, shell->sub_category, fd, req);

	/* if their is just a post request, no need reply here */
	if (shell->req == req_type_post)
	{
		rpc_queue_push(io_send.signal_queue, child);
		return NULL;
	}

	{
		rpc_buffer_t * body;
		shell->req = req_type_post;
		body = rpc_io_make_shell(shell, res_rpc);
		rpc_buffer_free(&res_rpc);

		rpc_queue_push(child->send, body);
		rpc_queue_push(io_send.signal_queue, child);

		ret = 1;
	}

	return NULL;
}

void rpc_io_processor_complete(rpc_tp_worker_t * worker, void * result, void * opaque)
{
	rpc_tp_worker_free(worker);
}

/*
 * io request dispatcher
 */
void rpc_io_request_dispatcher(int fd, rpc_io_t * child, rpc_shell_t * shell, rpc_buffer_t * in)
{
	rpc_io_task_t * task;

	task = (rpc_io_task_t *)rpc_malloc(sizeof(rpc_io_task_t));
	task->fd = fd;
	task->child = child;
	task->shell = shell;
	task->in = in;

	rpc_tp_worker_register(io_processor_pool, rpc_io_processor, rpc_io_processor_complete, task);
}

/*
 * make a rpc shell manual
 */
rpc_buffer_t * rpc_io_make_shell(rpc_shell_t * shell, rpc_buffer_t * in)
{
	rpc_buffer_t * out;
	char * out_buf;
	int out_size;
	rpc_buffer_t * body;
	const char * body_buffer;
	rpc_size_t body_size;

	body = NULL;
	if (in != NULL)
	{
		body = rpc_buffer_clone(in);
		body = rpc_buffer_encrypt(body, shell->crypt);

		body_buffer = rpc_buffer_get_buffer(body);
		body_size = rpc_buffer_get_size(body);
	}
	else
	{
		body_buffer = NULL;
		body_size = 0;
	}

	out_size = sizeof(rpc_shell_t) + body_size;
	out_buf = (char *)rpc_malloc(sizeof(rpc_shell_t) + body_size + 1);

	shell->body_size = body_size;
	memcpy(out_buf, shell, sizeof(rpc_shell_t));
	memcpy(out_buf + sizeof(rpc_shell_t), body_buffer, body_size);
	out = rpc_buffer_new(out_buf, out_size);

	rpc_buffer_free(&body);
	rpc_free(out_buf);

	return out;
}

/*
 * rpc io insert a new fd
 *
 * @return
 *     1 add ok
 *     0 exist no need add
 */
int rpc_io_add(int fd, void * extra)
{
	rpc_io_t * child;
	int ret = 0;

	child = rpc_io_find(fd);
	if (child == NULL)
	{
		child = (rpc_io_t *)rpc_calloc(1, sizeof(rpc_io_t));
		child->fd = fd;
		child->extra = extra;
		child->send = rpc_queue_new();
		child->recv = rpc_rb_new(RPC_IO_DEF_GROW_SIZE, RPC_IO_DEF_GROW_SIZE);
		child->shell_cached = 0;
		list_add_tail(&child->list, &io_list.list);
		ret = 1;
	}

	return ret;
}

/*
 * serialize received buffer to rpc_buffer_t 
 *
 * @return
 *     1 enqueue and send processor pool ok
 *     0 buffer is not enough
 *    -1 bad request, must clean current connection
 */
int rpc_io_enqueue(int fd, const char * inbuf, int insize)
{
	rpc_io_t * child;

	assert(inbuf != NULL && insize != 0);
	child = rpc_io_find(fd);
	assert(child != NULL);

	rpc_rb_write(child->recv, inbuf, insize);

	for (;;)
	{
		rpc_buffer_t * in;
		rpc_shell_t * shell;

		if (!child->shell_cached && rpc_rb_can_read(child->recv) < sizeof(*shell))
		{
			/* not enough for a package header */
			return 0;
		}

		if (!child->shell_cached)
		{
			rpc_rb_read(child->recv, (char **)&shell, sizeof(*shell));
		}

		if (shell->magic != RPC_MAGIC_NUMBER)
		{
			assert(0 && "received a bad request, magic != RPC_MAGIC_NUMBER");
			return -1;
		}

		if (shell->version != RPC_VERSION_SIGN)
		{
			assert(0 && "received a bad request, version != RPC_VERSION_SIGN");
			return -1;
		}

		if (rpc_rb_can_read(child->recv) < shell->body_size)
		{
			/* not enough for a package body */
			child->shell = *shell;
			child->shell_cached = 1;
			break;
		}

		in = NULL;
		/* if this request have a body */
		if (shell->body_size != 0)
		{
			char * body;

			rpc_rb_read(child->recv, &body, shell->body_size);
			in = rpc_buffer_new(body, shell->body_size);
			in = rpc_buffer_decrypt(in, shell->crypt);
		}

		/* dispatcher a io request */
		rpc_io_request_dispatcher(fd, child, shell, in);
		child->shell_cached = 0;
	}

	return 1;
}

/*
 * get a body from queue
 *
 * @void	body if get ok, you must manual free it.
 *
 * @timeout	send timeout in milliseconds or < 0 if infinite
 *
 * @return
 *     1 get it ok
 *     0 no more buffers now (timeout reached)
 *    -1 by your abort
 */
int rpc_io_get(int fd, rpc_buffer_t ** out, int timeout)
{
	rpc_io_t * child;
	int ret = 0;

	assert(out != NULL);

	*out = NULL;
	child = rpc_io_find(fd);
	if (child != NULL)
	{
		ret = rpc_queue_get(child->send, (void **)out, timeout);
	}

	return ret;
}

/*
 * send client an message
 *
 * @curfd	current fd
 * @notify_type
 *     0 notify all
 *     1 notify just curfd
 *     2 notify except curfd
 *
 * @return
 *     >=1 if notify ok
 *     0 failed
 */
int rpc_io_nodify_fd(int curfd, int notify_type, rpc_buffer_t * in)
{
	struct list_head * pos;
	rpc_io_t * tmp;
	int cnt;

	cnt = 0;
	list_for_each(pos, &io_list.list)
	{
		tmp = list_entry(pos, rpc_io_t, list);

		/* do not post any event message which contoured by itself */
		/*  we must notify user he is login from another place */
		if (notify_type == 1)
		{
			if (tmp->fd != curfd)
			{
				continue;
			}
		}
		else if (notify_type == 2)
		{
			if (tmp->fd == curfd)
			{
				continue;
			}
		}
		else
		{
			assert(notify_type == 0);
		}

		rpc_queue_push(tmp->send, rpc_buffer_clone(in));
		rpc_queue_push(io_send.signal_queue, rpc_io_find(curfd));
		cnt++;
	}

	return cnt;
}

/*
 * stop a rpc io
 */
void rpc_io_stop(int fd)
{
	struct list_head * pos;
	rpc_io_t * tmp;

	list_for_each(pos, &io_list.list)
	{
		tmp = list_entry(pos, rpc_io_t, list);
		if (tmp->fd == fd)
		{
			rpc_queue_stop(tmp->send);
			break;
		}
	}
}

/*
 * discard a fd ensure that no longer used
 */
void rpc_io_del(int fd)
{
	struct list_head * pos, * q;
	rpc_io_t * tmp;

	list_for_each_safe(pos, q, &io_list.list)
	{
		tmp = list_entry(pos, rpc_io_t, list);
		if (tmp->fd == fd)
		{
			list_del(pos);
			{
				rpc_buffer_t * buffer;
				rpc_queue_stop(tmp->send);

				while (rpc_queue_get(tmp->send, (void **)&buffer, 0) == 1)
				{
					rpc_buffer_free(&buffer);
				}

				rpc_queue_destroy(&tmp->send);
			}

			rpc_rb_free(tmp->recv);
			rpc_free(tmp);

			break;
		}
	}

	assert(rpc_io_find(fd) == NULL);
}

/*
 * get all child fd
 */
void rpc_io_all(rpc_fd_t ** fds, int * count)
{
	struct list_head * pos;
	rpc_io_t * tmp;
	rpc_fd_t * _fds;
	int cnt;

	cnt = 0;
	list_for_each(pos, &io_list.list) cnt++;
	if (count != 0) *count = cnt;
	if (fds == NULL || cnt == 0) return;

	{
		int idx;

		_fds = (rpc_fd_t *)rpc_calloc(1, cnt * sizeof(rpc_fd_t));
		idx = 0;
		list_for_each(pos, &io_list.list)
		{
			rpc_fd_t * fd;

			fd = &_fds[idx];
			tmp = list_entry(pos, rpc_io_t, list);
			fd->fd = tmp->fd;
			fd->extra = tmp->extra;
			idx++;
		}
		*fds = _fds;
	}
}

void rpc_io_all_free(rpc_fd_t ** fds)
{
	if (fds != NULL && *fds != NULL)
	{
		rpc_free(*fds);
		*fds = NULL;
	}
}

/*
 * rpc io module destroy
 */
void rpc_io_destroy()
{
	struct list_head * pos, * q;
	rpc_io_t * tmp;

	if (!io_inited)
	{
		return;
	}

	list_for_each_safe(pos, q, &io_list.list)
	{
		tmp = list_entry(pos, rpc_io_t, list);
		list_del(pos);

		{
			rpc_buffer_t * buffer;
			rpc_queue_stop(tmp->send);

			while (rpc_queue_get(tmp->send, (void **)&buffer, 0) == 1)
			{
				rpc_buffer_free(&buffer);
			}

			rpc_queue_destroy(&tmp->send);
		}

		rpc_rb_free(tmp->recv);
		rpc_free(tmp);
	}

	io_send.abort = 1;
	rpc_queue_stop(io_send.signal_queue);
	pthread_join(io_send.thread, NULL);

	{
		rpc_io_t * io;
		while (rpc_queue_get(io_send.signal_queue, (void **)&io, 0) == 1);
		rpc_queue_destroy(&io_send.signal_queue);
	}

	rpc_tp_destroy(io_processor_pool, tp_kill_wait);
	io_processor_pool = NULL;

	io_inited = 0;
}