/*
 * rpc buffer helper, user for serial buffer streams
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2012-7-26 Thu 8:04
 */
#include "rpc_buffer.h"
#include "rpc_alloctor.h"
#include "rpc_crypt.h"

#include <assert.h>
#include <string.h>

#define RPC_BUFFER_DEF_GROW_SIZE 64 /* 64b */

static unsigned char rpc_pri_key[16] = {
	0x33, 0x37, 0x44, 0x31, 0x31, 0x32, 0x32, 0x33, 0x32, 0x42, 0x42, 0x41, 0x34, 0x64, 0x34, 0x65
};

/*
 * max buffer size is 2G so rpc_size_t must be short than (rpc_size_t)
 */
struct rpc_buffer
{
	char * chunk;  /* chunk buffer, grow size in least RPC_BUFFER_DEF_GROW_SIZE */
	rpc_size_t chunk_size; /* chunk current size now */
	rpc_size_t chunk_used; /* check used now*/
	rpc_size_t parse_position; /* save current parse position */
};

/*
 * create a new rpc buffer
 */
rpc_buffer_t * rpc_buffer_new(const char * in, rpc_size_t size)
{
	rpc_buffer_t * buffer;

	if (in == NULL || size == 0) { assert(0); return NULL; }
	if (size < 0) size = strlen(in);

	buffer = (rpc_buffer_t *)rpc_calloc(1, sizeof(rpc_buffer_t));
	/* we alloc some more memory here for reducing memory fragmentation */
	buffer->chunk_size = size + 1 + RPC_BUFFER_DEF_GROW_SIZE;
	buffer->chunk = (char *)rpc_malloc(buffer->chunk_size);
	memcpy(buffer->chunk, in, size);
	buffer->chunk[size] = '\0';
	buffer->chunk_used = size;
	buffer->parse_position = 0;

	return buffer;
}

/*
 * clone a rpc buffer
 */
rpc_buffer_t * rpc_buffer_clone(rpc_buffer_t * rpc)
{
	return rpc_buffer_new(rpc_buffer_get_buffer(rpc), rpc_buffer_get_size(rpc));
}

/*
 * rpc buffer get buffers
 */
const char * rpc_buffer_get_buffer(rpc_buffer_t * buffer)
{
	return buffer->chunk;
}

/*
 * reset rpc parser position
 */
void rpc_buffer_rewind(rpc_buffer_t * buffer)
{
	assert(buffer != NULL);
	buffer->parse_position = 0;
}

/*
 * rpc buffer get buffer size
 */
rpc_size_t rpc_buffer_get_size(rpc_buffer_t * buffer)
{
	assert(buffer->chunk_used > 0);
	return buffer->chunk_used;
}

/*
 * free rpc buffers
 */
void rpc_buffer_free(rpc_buffer_t ** buffer)
{
	if (buffer != NULL && *buffer != NULL)
	{
		rpc_free((*buffer)->chunk);
		rpc_free(*buffer);
		*buffer = NULL;
	}
}

/*
 * encrypt an rpc buffer
 */
rpc_buffer_t * rpc_buffer_encrypt(rpc_buffer_t * buffer, rpc_crypt_type type)
{
	if (type == crypt_type_none)
	{
		return buffer;
	}

	{
		unsigned char * enc;
		int enc_size;
		
		/* if there was an encrypt stream the first 4 bytes will
			specify the decrypted stream size */
		switch (type)
		{
		case crypt_type_aes:
			{
				enc = rpc_aes_crypt((unsigned char *)buffer->chunk, buffer->chunk_used, &enc_size, rpc_pri_key, 1);
				break;
			}
		case crypt_type_tea:
			{
				enc = rpc_tea_crypt((unsigned char *)buffer->chunk, buffer->chunk_used, &enc_size, rpc_pri_key, 1);
				break;
			}
		default:
			assert(0);
			return buffer;
		}

		/* if we need to realloc a new memory */
		if ((rpc_size_t)(buffer->chunk_size - sizeof(rpc_size_t)) < enc_size)
		{
			rpc_free(buffer->chunk);
			buffer->chunk_size = enc_size + sizeof(rpc_size_t) + 1;
			buffer->chunk = (char *)rpc_malloc(buffer->chunk_size);
		}

		/* generate a new rpc buffer */
		{
			/* tell the original buffer size */
			*(rpc_size_t *)buffer->chunk = buffer->chunk_used;
			memcpy(buffer->chunk + sizeof(rpc_size_t), enc, enc_size);
			buffer->chunk_used = enc_size + sizeof(rpc_size_t);
		}

		rpc_free(enc);
	}

	return buffer;
}

/*
 * decrypt an rpc buffer
 */
rpc_buffer_t * rpc_buffer_decrypt(rpc_buffer_t * buffer, rpc_crypt_type type)
{	
	if (type == crypt_type_none)
	{
		return buffer;
	}

	{
		unsigned char * dec;
		int decsize;

		/* if there was an encrypt stream the first 4 bytes will 
		specify the decrypted stream size */
		switch (type)
		{
		case crypt_type_aes:
			{
				dec = rpc_aes_crypt((const unsigned char *)buffer->chunk + sizeof(rpc_size_t), buffer->chunk_used - sizeof(rpc_size_t), &decsize, rpc_pri_key, 0);
				break;
			}
		case crypt_type_tea:
			{
				dec = rpc_tea_crypt((unsigned char *)buffer->chunk + sizeof(rpc_size_t), buffer->chunk_used - sizeof(rpc_size_t), &decsize, rpc_pri_key, 0);
				break;
			}
		default:
			assert(0);
			return buffer;
		}

		/* if we need to realloc a new memory */
		if ((rpc_size_t)(buffer->chunk_size - sizeof(rpc_size_t)) < decsize)
		{
			rpc_free(buffer->chunk);
			buffer->chunk_size = decsize - sizeof(rpc_size_t) + 1;
			buffer->chunk = (char *)rpc_malloc(buffer->chunk_size);
		}

		/* generate a new rpc buffer */
		{
			/* get the original size */
			buffer->chunk_used = *(rpc_size_t *)buffer->chunk;
			memcpy(buffer->chunk, dec, buffer->chunk_used);
		}

		rpc_free(dec);
	}

	return buffer;
}

/*
 * append a buffer to rpc buffer, rpc buffer can be null
 *  if first buffer append here.
 */
rpc_buffer_t * rpc_buffer_append(rpc_buffer_t * buffer, const char * in, rpc_size_t size)
{
	assert(in != NULL);

	if (size <= 0)
	{
		size = (rpc_size_t)strlen(in);
	}

	if (buffer == NULL)
	{
		buffer = (rpc_buffer_t *)rpc_malloc(sizeof(rpc_buffer_t));
		buffer->chunk_size = RPC_BUFFER_DEF_GROW_SIZE + size;
		buffer->chunk = (char *)rpc_malloc(buffer->chunk_size);
		buffer->chunk_used = 0;
		buffer->parse_position = 0;
	}

	/* we will realloc an buffer if current memory is not enough */
	if (buffer->chunk_used + size > buffer->chunk_size)
	{
		char * chunk = buffer->chunk;
		buffer->chunk_size = buffer->chunk_used + size + RPC_BUFFER_DEF_GROW_SIZE;
		buffer->chunk = (char *)rpc_malloc(buffer->chunk_size);
		memcpy(buffer->chunk, chunk, buffer->chunk_used);
		rpc_free(chunk);
	}

	memcpy(buffer->chunk + buffer->chunk_used, &size, sizeof(size));
	buffer->chunk_used += sizeof(size);
	memcpy(buffer->chunk + buffer->chunk_used, in, size);
	buffer->chunk_used += size;

	return buffer;
}

/*
 * parse a rpc buffer and moved the position.
 *  return null if no any more buffers.
 */
const char * rpc_buffer_next(rpc_buffer_t * buffer, rpc_size_t * size)
{
	const char * buf;
	rpc_size_t bufsize;

	assert(buffer != NULL && size != NULL);

	if (buffer->parse_position + 2 /* for ': ' */ >= buffer->chunk_used)
	{
		return NULL;
	}

	bufsize = *(rpc_size_t *)(buffer->chunk + buffer->parse_position);
	if (bufsize == 0)
	{
		return NULL;
	}
	
	if (bufsize > buffer->chunk_used - buffer->parse_position)
	{
		assert(0);
		return NULL;
	}

	buffer->parse_position += sizeof(bufsize);
	buf = buffer->chunk + buffer->parse_position;
	buffer->parse_position += bufsize;
	*size = bufsize;

	return buf;
}