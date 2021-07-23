/*
 * http cgi wrapper
 *
 * @author rui.sun 2012-7-13 Fri 7:01
 */
#include "https.h"

#include "mongoose.h"

#include "rpc_keypair.h"
#include "urlbase64.h"

#include "posixinc.h"
#include "queue.h"
#include "list.h"

struct https_req_t
{
	char * key; /* head key string */
	char * value; /* head key value */
	struct list_head list;
};

typedef struct https_cb_t
{
	pfn_https_cb cb;
	char * paths;
	void * opaque;
	struct list_head list;
} https_cb_t;

char https_uri_root[256]; /* uri root full name, eg: http://127.0.0.1:8580/ */
struct mg_context *ctx; /* http server instance */
https_cb_t dispatch_list; /* all user callback list */
int https_inited; /* whether http module inited */

/*
 * get http query value such as http://xxx.req?name=?&keyvalue=?
 *
 * @param [out] https_req
 *     the value string or NULL if not exist/cannot be read
 *
 * @return
 *     0 success
 *    -1 error
 */
int req_get_query_string(struct mg_connection *conn, const struct mg_request_info *request_info, struct https_req_t * https_req)
{
	char * user_agent = NULL;
	char * sessionb64 = NULL;
	const char * ip = NULL;
	int port = 0;

	int header;
	for (header = 0; header < request_info->num_headers; header++)
	{
		const struct mg_header * tmp = &request_info->http_headers[header];
		if (stricmp(tmp->name, "User-Agent") == 0)
		{
			user_agent = tmp->value;
		}
		else if (stricmp(tmp->name, "session") == 0)
		{
			sessionb64 = tmp->value;
		}

		/* we want not to parse everything if user header is so many */
		if (user_agent != NULL && sessionb64 != NULL)
		{
			break;
		}
	}

	if (user_agent == NULL || stricmp(user_agent, "mediacore") != 0 || sessionb64 == NULL)
	{
		return -1;
	}

	/* first decode session base64 */
	{
		char * sess_raw = NULL;
		int sess_rawsize = 0;

		sess_raw = urlbase64_decode(sessionb64, -1, &sess_rawsize);
		assert(sess_raw != NULL);

		/* second decrypt rpc buffer */
		{
			rpcbuffer_t * rpcbuf;
			rpcbuf = rpcbuffer_new(sess_raw, sess_rawsize);
			rpcbuf = rpcbuffer_decrypt(rpcbuf);

			/* final get every query strings */
			{
				const char * key;
				int keysize;
				const char * value;
				int valuesize;

				while (rpc_keypair_next(rpcbuf, &key, &keysize, &value, &valuesize))
				{
					struct https_req_t * newreq;
					newreq = (struct https_req_t *)malloc(sizeof(struct https_req_t));
					newreq->key = malloc(keysize + 1);
					memcpy(newreq->key, key, keysize);
					newreq->key[keysize] = 0;
					newreq->value = malloc(valuesize + 1);
					memcpy(newreq->value, value, valuesize);
					newreq->value[valuesize] = 0;

					list_add_tail(&(*newreq).list, &(*https_req).list);
				}

			}
			rpcbuffer_free(&rpcbuf);
		}
		free(sess_raw);
	}

	/* insert custom data here */
	{
		struct https_req_t * newreq;
		struct in_addr addr;

		addr.s_addr = request_info->remote_ip;
		ip = inet_ntoa(addr);

		/* insert user address */
		newreq = (struct https_req_t *)malloc(sizeof(struct https_req_t));
		newreq->key = strdup("$address");
		newreq->value = strdup(ip);
		list_add_tail(&(*newreq).list, &(*https_req).list);

		/* insert user port */
		{
			char cport[32];

			port = request_info->remote_port;
			itoa(port, cport, 10);
			newreq = (struct https_req_t *)malloc(sizeof(struct https_req_t));
			newreq->key = strdup("$port");
			newreq->value = strdup(cport);
			list_add_tail(&(*newreq).list, &(*https_req).list);
		}
	}

	return 0;
}

static void * processed = "";

static void * req_dispath(enum mg_event evt,
	struct mg_connection *conn,
	const struct mg_request_info *request_info)
{
	struct evbuffer * evb = NULL;
	const char * acttype = NULL;
	char * response = NULL;
	https_req_t https_req;
	int bodysize;
	int query_ret;

	if (evt != MG_NEW_REQUEST)
	{
		return processed;
	}

	INIT_LIST_HEAD(&https_req.list);

	/* get all query string */
	query_ret = req_get_query_string(conn, request_info, &https_req);
	if (query_ret != 0)
	{
		return processed;
	}

	{
		struct list_head * pos;
		https_cb_t * tmp;
		int count = 0;

		list_for_each(pos, &dispatch_list.list)
		{
			const char * path = request_info->uri;
			tmp = list_entry(pos, https_cb_t, list);
			if (strncmp(tmp->paths, path, strlen(tmp->paths)) == 0)
			{
				response = tmp->cb(tmp->opaque, &https_req, &bodysize);
			}

			count++;
		}
	}

	{
		struct list_head * pos, * q;
		https_req_t * tmp;
		int ret = -1;

		list_for_each_safe(pos, q, &https_req.list)
		{
			tmp = list_entry(pos, https_req_t, list);
			list_del(pos);
			free(tmp->key);
			free(tmp->value);
			free(tmp);
		}
	}

	if (response != NULL)
	{
		mg_printf(conn,
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain\r\n\r\n"
			"%s", response);
		free(response);
	}
	else
	{
		mg_printf(conn, 
			"HTTP/1.1 401 Unauthorized\r\n"
			"Content-Type: text/plain\r\n\r\n");
	}

	return processed;
}

/*
 * init http core
 *
 * return
 *    0 if no errors
 */
int https_init(int listenport, int timeout)
{
	INIT_LIST_HEAD(&dispatch_list.list);

	https_inited = 1;

	return 0;
}

/*
 * http add callbacks for specific paths
 */
void https_add_cb(const char * paths, pfn_https_cb cb, void * opaque)
{
	https_cb_t * newcb;

	newcb = (https_cb_t *)malloc(sizeof(https_cb_t));
	newcb->paths = strdup(paths);
	newcb->cb = cb;
	newcb->opaque = opaque;

	list_add_tail(&(*newcb).list, &dispatch_list.list);
}

/*
 * get full uri root, eg: http://127.0.0.1:80/
 */
const char * https_get_uri_root()
{
	return https_uri_root;
}

/*
 * get how many key have in this request
 */
int https_get_key_count(const https_req_t * req)
{
	struct list_head * pos;
	https_req_t * tmp;
	int count = 0;

	list_for_each(pos, &(*req).list)
	{
		tmp = list_entry(pos, https_req_t, list);
		count++;
	}

	return count;
}

/*
 * get user key value by index
 *
 * @return
 *     NULL if no data here
 */
const char * https_req_index(const https_req_t * req, int index)
{
	struct list_head * pos;
	https_req_t * tmp;
	int count = 0;

	list_for_each(pos, &(*req).list)
	{
		tmp = list_entry(pos, https_req_t, list);
		if (count == index)
		{
			return tmp->key;
		}
		count++;
	}

	return NULL;
}

/*
 * get http query value such as http://xxx.req?name=?&keyvalue=?
 *
 * @return
 *     the value string or NULL if not exist/cannot be read 
 */
const char * https_req_get(const https_req_t * req, const char * key)
{
	struct list_head * pos;
	https_req_t * tmp;

	list_for_each(pos, &(*req).list)
	{
		tmp = list_entry(pos, https_req_t, list);
		if (strcmp(tmp->key, key) == 0)
		{
			return tmp->value;
		}
	}

	return NULL;
}

/*
 * start http core
 */
int https_start()
{
	const char *options[] = {
		"listening_ports", "8580",
		"enable_directory_listing", "no",
		"num_threads", "20",
		NULL};

	ctx = mg_start(&req_dispath, NULL, options);

	return 0;
}

/*
 * destroy http
 */
void https_destroy()
{
	if (https_inited)
	{
		struct list_head * pos, * q;
		https_cb_t * tmp;
		int ret = -1;

		list_for_each_safe(pos, q, &dispatch_list.list)
		{
			tmp = list_entry(pos, https_cb_t, list);
			list_del(pos);
			free(tmp->paths);
			free(tmp);
		}

		if (ctx != NULL)
		{
			mg_stop(ctx);
			ctx = NULL;
		}

		https_inited = 0;
	}
}
