/*
 * media module http support
 *
 * author rui.sun 2012-7-3 Tue 17:58
 */
#ifndef HTTPC_H
#define HTTPC_H

typedef struct httpc httpc_t;

/*
 * http new request
 *  eg: http://127.0.0.1:8580/subdir
 * 
 * @param uri
 *     full uri prefix string
 */
httpc_t * httpc_new(const char * uri, const char * subdir);

/*
 * http add query pair
 *  eg: http://localhost:8580/subdir?a_key=val
 */
void httpc_add_query(httpc_t * http, char * a_key, char * a_val);

/*
 * http add body
 */
void httpc_add_body(httpc_t * http, char *a_body, int a_len);

/*
 * this is function only used for make body pairs.
 *  eg: [size]key1: value1[size]key2: vlaue2[0]
 */
void httpc_add_query_pair(httpc_t * http, const char *a_key, const char * a_value);

/*
 * http add header
 *
 *  eg: Connection: keep-alive
 *      a_hdr: a_val
 */
void httpc_add_header(httpc_t * http, const char * a_hdr, const char * a_val);

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
int httpc_get(httpc_t * http, char ** body, int * bodylen);

/*
 * http destroy
 */
void httpc_destroy(httpc_t * http);

#endif