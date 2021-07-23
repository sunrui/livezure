/*
 * rpc ring buffer
 * @copyright (c) livezure.com
 * @date 2013-4-13 0:45
 */
#include "rpc_ringbuffer.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

struct rpc_rb
{
	char * buffer;
	char * head;
	char * tail;

	int capacity;
	int expand;

	char * ret_buffer;
	int ret_buffer_size;
};

rpc_rb_t* rpc_rb_new(int capacity, int expand)
{
	rpc_rb_t * rb;

	assert(capacity > 0 && expand > 0);
	rb = (rpc_rb_t * )malloc(sizeof(rpc_rb_t));
	rb->capacity = capacity;
	rb->buffer = (char *)malloc(rb->capacity);
	rb->head = rb->buffer;
	rb->tail = rb->buffer;
	rb->expand = expand;
	rb->ret_buffer = NULL;
	rb->ret_buffer_size = 0;

	return rb;
};

void rpc_rb_free(rpc_rb_t * rb)
{
	assert(rb != NULL);

	if (rb->ret_buffer != NULL) free(rb->ret_buffer);
	free(rb->buffer);
	free(rb);
}

int rpc_rb_capacity(rpc_rb_t * rb)
{
	assert(rb != NULL);

	return rb->capacity;
}

int rpc_rb_can_read(rpc_rb_t * rb)
{
	assert(rb != NULL);

	if (rb->head == rb->tail)
	{
		return 0;
	}

	if (rb->head < rb->tail)
	{
		return rb->tail - rb->head;
	}

	return rb->capacity - (rb->head - rb->tail);
}

int rpc_rb_can_write(rpc_rb_t * rb)
{
	assert(rb != NULL);

	return rb->capacity - rpc_rb_can_read(rb);
}

int rpc_rb_read(rpc_rb_t * rb, char ** data, int count)
{
	assert(rb != NULL && data != NULL);

	if (rb->head < rb->tail)
	{
		int size;
		
		size = min(count, rpc_rb_can_read(rb));
		*data = rb->head;
		rb->head += size;

		return size;
	}
	else
	{
		if (rb->ret_buffer_size < count)
		{
			if (rb->ret_buffer_size > 0) free(rb->ret_buffer);
			rb->ret_buffer = (char *)malloc(count);
			rb->ret_buffer_size = count;
		}

		if (count < rpc_rb_capacity(rb) - (rb->head - rb->buffer))
		{
			int size;
			
			size = count;
			memcpy(rb->ret_buffer, rb->head, size);
			rb->head += size;
			*data = rb->ret_buffer;

			return size;
		}
		else
		{
			int size;
			
			size = rpc_rb_capacity(rb) - (rb->head - rb->buffer);
			memcpy(rb->ret_buffer, rb->head, size);
			rb->head = rb->buffer;
			size += rpc_rb_read(rb, &rb->ret_buffer + size, count-size);
			*data = rb->ret_buffer;

			return size;
		}
	}
}

int rpc_rb_write(rpc_rb_t * rb, const char * data, int count)
{
	assert(rb != NULL);
	assert(data != NULL);

	if (count >= rpc_rb_can_write(rb))
	{
		char * buffer_new;
		int capacity_new;

		capacity_new = rpc_rb_can_read(rb) + count + rb->expand;
		buffer_new = (char *)malloc(capacity_new);

		{
			char * readbuf;
			int nreads;
			int readed;

			nreads = rpc_rb_can_read(rb);
			readed = rpc_rb_read(rb, &readbuf, nreads);
			assert(readed == nreads);
			memcpy(buffer_new, readbuf, readed);
			free(rb->buffer);
			rb->buffer = buffer_new;
			rb->head = rb->buffer;
			rb->tail = rb->head + readed;
			rb->capacity = capacity_new;
		}
	}

	if (rb->head <= rb->tail)
	{
		int tail_size;
		
		tail_size = rpc_rb_capacity(rb) - (rb->tail - rb->buffer);
		if (count <= tail_size)
		{
			memcpy(rb->tail, data, count);
			rb->tail += count;
			if (rb->tail == rb->buffer + rpc_rb_capacity(rb))
			{
				rb->tail = rb->buffer;
			}

			return count;
		}
		else
		{
			int size;

			memcpy(rb->tail, data, tail_size);
			rb->tail = rb->buffer;
			size = tail_size + rpc_rb_write(rb, data + tail_size, count - tail_size);

			return size;
		}
	}
	else
	{
		memcpy(rb->tail, data, count);
		rb->tail += count;

		return count;
	}
}