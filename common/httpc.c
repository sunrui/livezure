/*
 * media module http support
 *
 * author rui.sun 2012-7-3 Tue 17:58
 */
#include "httpc.h"

#include <ghttp.h>
#include "urlcode.h"
#include "rpckeypair.h"
#include "urlbase64.h"

#include <string.h>
#include <assert.h>
#include <malloc.h>
#include <stdlib.h>

struct httpc
{
	ghttp_request * req;
	char uri[4096];
	int first_question_mark; /* whether their is first http://sample.com/dir? */

	rpcbuffer_t * rpc;
};

/*
 * http new request
 *  eg: http://127.0.0.1:8580/subdir
 * 
 * @param uri
 *     full uri prefix string
 */

httpc_t * httpc_new(const char * uri, const char * subdir)
{
	httpc_t * http;

	http = (httpc_t *)malloc(sizeof(httpc_t));

	http->req = ghttp_request_new();
	http->rpc = NULL;
	assert(uri[strlen(uri) - 1] == '/');
	strcpy(http->uri, uri);
	strcat(http->uri, subdir);
	http->first_question_mark = 1;

	return http;
}

/*
 * http add query pair
 *  eg: http://localhost:8580/subdir?a_key=val
 */
void httpc_add_query(httpc_t * http, char * a_key, char * a_val)
{
	if (http->first_question_mark)
	{
		strcat(http->uri, "?");
		http->first_question_mark = 0;
	}
	else
	{
		strcat(http->uri, "&");
	}

	strcat(http->uri, a_key);
	strcat(http->uri, "=");

	/* encode val */
	{
		char * a_encode_val = url_encode(a_val);
		strcat(http->uri, a_encode_val);
		free(a_encode_val);
	}
}

/*
 * http add body
 */
void httpc_add_body(httpc_t * http, char *a_body, int a_len)
{
	ghttp_set_body(http->req, a_body, a_len);
}

/*
 * this is function only used for make body pairs.
 *  eg: [size]key1: value1[size]key2: vlaue2[0]
 */
void httpc_add_query_pair(httpc_t * http, const char *a_key, const char * a_value)
{
	http->rpc = rpckeypair_append(http->rpc, a_key, -1, a_value, -1);
}


/*
 * http add header
 *
 *  eg: Connection: keep-alive
 *      a_hdr: a_val
 */
void httpc_add_header(httpc_t * http, const char * a_hdr, const char * a_val)
{
	ghttp_set_header(http->req, a_hdr, a_val);
}

/*
 * http get process in block method
 *
 * @param [out]body
 *     memory managed by http instance [no need to free]
 * @return
 *      0 get ok
 *     -1 syntax error
 *     -2 network error
 */
int httpc_get(httpc_t * http, char ** body, int * bodylen)
{
	ghttp_status status;
	int ret;

	assert(body != NULL && *body != NULL);
	assert(bodylen != NULL);

	ret = ghttp_set_uri(http->req, http->uri);
	if (ret == -1)
	{
		goto ret_clean;
	}

	ret = ghttp_set_type(http->req, ghttp_type_get);
	if (ret == -1)
	{
		goto ret_clean;
	}

	if (http->rpc != NULL)
	{
		char * rpcbuf;
		char * rpcbufb64;
		int rpcsize;

		/* encrypt user session */
		http->rpc = rpcbuffer_encrpyt(http->rpc);

		rpcbuf = (char *)rpcbuffer_get_buffer(http->rpc);
		rpcsize = rpcbuffer_get_size(http->rpc);

		rpcbufb64 = urlbase64_encode(rpcbuf, rpcsize, NULL);

		ghttp_set_header(http->req, "session", rpcbufb64);
		free(rpcbufb64);
	}

	/* add our agents */
	ghttp_set_header(http->req, "Connection", "Close");
	ghttp_set_header(http->req, "User-Agent", "mediacore");

	ghttp_set_sync(http->req, ghttp_sync);
	ghttp_prepare(http->req);

	status = ghttp_process(http->req);
	if(status == ghttp_error)
	{
		ret = -2;
		goto ret_clean;
	}

	*body = ghttp_get_body(http->req);
	*bodylen = ghttp_get_body_len(http->req);
	ret = 0;

ret_clean:
	return ret;
}

/*
 * http destroy
 */
void httpc_destroy(httpc_t * http)
{
	ghttp_flush_response_buffer(http->req);
	rpcbuffer_free(&http->rpc);
	ghttp_request_destroy(http->req);
	free(http);
}
