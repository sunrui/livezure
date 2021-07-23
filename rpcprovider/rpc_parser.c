/*
 * rpc parser
 *
 * @date 2012-8 rui.sun
 */
#include "rpc_parser.h"
#include "rpc_keypair.h"
#include "rpc_alloctor.h"

#include "list.h"

#include <assert.h>
#include <string.h>

struct rpc_req
{
	char * key; /* key string */
	int keysize;
	char * value; /* key value */
	int valuesize;

	struct list_head list;
};

/*
 * create a new rpc request by rpc_buffer_t
 */
rpc_req_t * rpc_parser_new(rpc_buffer_t * buffer)
{
	/* final get every query strings */
	rpc_req_t * req;
	const char * key;
	const char * value;
	rpc_size_t valuesize;

	if (buffer == NULL)
	{
		assert(0 && "parser buffer cannot be null.");
		return NULL;
	}

	req = (rpc_req_t *)rpc_calloc(1, sizeof(rpc_req_t));
	INIT_LIST_HEAD(&req->list);
	rpc_buffer_rewind(buffer);

	while (rpc_keypair_next(buffer, &key, &value, &valuesize))
	{
		rpc_parser_add(req, key, value, valuesize);
	}

	return req;
}

/*
 * add a new key/value to rpc parser
 */
void rpc_parser_add(rpc_req_t * req, const char * key, const char * value, rpc_size_t valuesize)
{
	rpc_req_t * newreq;
	int keysize;

	if (req == NULL || key == NULL || value == NULL)
	{
		assert(0 && "rpc_parser_add param error.");
		return;
	}

	keysize = strlen(key);
	if (value != NULL && valuesize < 0) valuesize = strlen(value);

	newreq = (rpc_req_t *)rpc_calloc(1, sizeof(rpc_req_t));
	newreq->key = (char *)rpc_calloc(1, keysize + 1);
	memcpy(newreq->key, key, keysize);
	newreq->key[keysize] = 0;
	newreq->keysize = keysize;

	if (value != NULL && valuesize != 0)
	{
		newreq->value = (char *)rpc_calloc(1, valuesize + 1);
		memcpy(newreq->value, value, valuesize);
		newreq->value[valuesize] = 0;
		newreq->valuesize = valuesize;
	}

	list_add_tail(&(*newreq).list, &(*req).list);
}

/*
 * get how many key have in this request
 */
int rpc_parser_count(rpc_req_t * req)
{
	struct list_head * pos;
	rpc_req_t * tmp;
	int count = 0;

	if (req == NULL)
	{
		assert(0 && "rpc_req_t is null.");
		return 0;
	}

	list_for_each(pos, &(*req).list)
	{
		tmp = list_entry(pos, rpc_req_t, list);
		count++;
	}

	return count;
}

/*
 * get user key by index
 *
 * @return
 *     1 ok
 *     0 failed
 */
int rpc_parser_index(rpc_req_t * req, int index, char ** key, char ** value, rpc_size_t * valuesize)
{
	struct list_head * pos;
	rpc_req_t * tmp;
	int count;

	if (req == NULL || key == NULL || value == NULL)
	{
		assert(0 && "rpc_parser_index param error.");
		return 0;
	}

	*key = NULL, *value = NULL, count = 0;
	list_for_each(pos, &(*req).list)
	{
		tmp = list_entry(pos, rpc_req_t, list);
		if (count == index)
		{
			*key = tmp->key;
			*value = tmp->value;
			if (valuesize != NULL) *valuesize = tmp->valuesize;
			return 1;
		}
		count++;
	}

	return 0;
}

/*
 * get query value such as key=value
 *
 * @return
 *     the value string or NULL if not exist/cannot be read 
 */
const char * rpc_parser_get(rpc_req_t * req, const char * key, rpc_size_t * valuesize)
{
	struct list_head * pos;
	rpc_req_t * tmp;

	if (req == NULL || key == NULL)
	{
		return NULL;
	}

	assert(req != NULL && key != NULL);
	list_for_each(pos, &(*req).list)
	{
		int keysize;

		tmp = list_entry(pos, rpc_req_t, list);
		keysize = strlen(key);

		if (memcmp(tmp->key, key, keysize) == 0)
		{
			if (valuesize != NULL) *valuesize = tmp->valuesize;
			return tmp->value;
		}
	}

	return NULL;
}

/*
 * free a rpc request
 */
void rpc_parser_free(rpc_req_t ** req)
{
	struct list_head * pos, * q;
	rpc_req_t * _req;
	rpc_req_t * tmp;

	if (req == NULL || *req == NULL)
	{
		return;
	}

	_req = *req;

	list_for_each_safe(pos, q, &(*_req).list)
	{
		tmp = list_entry(pos, rpc_req_t, list);
		list_del(pos);
		assert(tmp->key != NULL); rpc_free(tmp->key);
		if (tmp->value != NULL) rpc_free(tmp->value);
		rpc_free(tmp);
	}

	rpc_free(_req);
	*req = NULL;
}