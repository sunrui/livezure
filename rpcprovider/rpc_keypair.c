/*
 * rpc buffer helper, user for serial buffer streams
 *
 * Copyright (C) 2012-2013 livezure.com
 * rui.sun 2012-7-26 Thu 8:04
 */
#include "rpc_buffer.h"
#include "rpc_alloctor.h"

#include <assert.h>
#include <string.h>

const static char * divide = ": ";
const static int divide_len = 2;

/*
 * append a query key, rpc buff can be NULL if first item append here
 *
 * eg:
 * [total size] body {[size]key: value[size]key: value}
 */
rpc_buffer_t * rpc_keypair_append(rpc_buffer_t * buffer, const char * key, const char * body, rpc_size_t bodysize)
{
	int keybufsize;
	char * keybuf;
	int keysize;
	int keycur;

	assert(key != NULL && body != NULL);

	keysize = strlen(key);
	if (bodysize <= 0) bodysize = strlen(body);
	keybufsize = keysize + bodysize + divide_len;
	keybuf = (char *)rpc_malloc(keybufsize);
	keycur = 0;

	memcpy(keybuf + keycur, key, keysize);
	keycur += keysize;
	memcpy(keybuf + keycur, divide, divide_len);
	keycur += divide_len;
	memcpy(keybuf + keycur, body, bodysize);
	keycur += bodysize;

	buffer = rpc_buffer_append(buffer, keybuf, keycur);
	rpc_free(keybuf);

	return buffer;
}

/*
 * get rpc next key
 *
 * @return
 *     1 if get ok
 *     0 get failed
 */
int rpc_keypair_next(rpc_buffer_t * buffer, const char ** key, const char ** body, rpc_size_t * bodysize)
{
	const char * streambuf;
	rpc_size_t streamsize;

	char * retbuf;
	int b_ok;

	assert(key != NULL && body != NULL && bodysize != NULL);

	*key = NULL, *body = NULL, *bodysize = 0;
	retbuf = NULL;
	b_ok = 0;

	streambuf = rpc_buffer_next(buffer, &streamsize);
	if (streambuf == NULL)
	{
		goto ret_clean;
	}

	{
		const char * querykey;

		querykey = strstr(streambuf, divide);
		if (querykey == NULL)
		{
			goto ret_clean;
		}

		*key = streambuf;
		*bodysize = streamsize - (querykey - streambuf) - divide_len;
		*body = querykey + divide_len;
	}

	b_ok = 1;

ret_clean:
	return b_ok;
}