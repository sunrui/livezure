/*
 * http cgi wrapper
 *
 * @author rui.sun 2012-7-13 Fri 7:01
 */
#ifndef HTTPS_H
#define HTTPS_H

/* http request handler */
typedef struct https_req_t https_req_t;

/*
 * the callback function that gets invoked on requesting path
 *
 * @param[in] opaque
 *     defined user data
 * @param[in] req
 *     callback that can be use for control header key
 * @param[out] bodysize
 *     tell http core what size body have
 *
 * @return
 *     return alloc-ed http body
 */
typedef char * (* pfn_https_cb)(void * opaque, const https_req_t * req, int * bodysize);

/*
 * init http core
 *
 * return
 *    0 if no errors
 */
int https_init(int listenport, int timeout);

/*
 * http add callbacks for specific paths
 */
void https_add_cb(const char * paths, pfn_https_cb cb, void * opaque);

/*
 * get full uri root, eg: http://localhost:8580/
 */
const char * https_get_uri_root();

/*
 * get how many key have in this request
 */
int https_get_key_count(const https_req_t * req);

/*
 * get user key by index
 *
 * @return
 *     NULL if no data here
 */
const char * https_req_index(const https_req_t * req, int index);

/*
 * get http query value such as http://xxx.req?name=?&keyvalue=?
 *
 * @return
 *     the value string or NULL if not exist/cannot be read 
 */
const char * https_req_get(const https_req_t * req, const char * key);

/*
 * set a callback for a specified URI
 */
void https_add_cb(const char * path, pfn_https_cb https_cb, void * userdata);

/*
 * start http core
 */
int https_start();

/*
 * destroy http
 */
void https_destroy();

#endif