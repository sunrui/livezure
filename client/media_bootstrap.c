/*
 * service rpc entry
 *
 * @author rui.sun 2012-7-22 Sun 12:52
 */
#include "media_bootstrap.h"
#include "posixinc.h"
#include "timer.h"
#include "log4c_extend.h"
#include "rpc_dispatch.h"
#include "rpc_client.h"

#include "rpc_declare.h"

/* event callback declare */
rpc_buffer_t * media_rpc_event_login_dispatcher(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);
rpc_buffer_t * media_rpc_event_logout_dispatcher(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);
rpc_buffer_t * media_rpc_event_discon_dispatcher(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);
rpc_buffer_t * media_rpc_event_update_dispatcher(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);
rpc_buffer_t * media_rpc_event_login_repeat_dispatcher(void * opaque, int fd, rpc_req_t * req, int * b_req_reclaim);

int register_all_reqeust_handler();

/*
 * all client pre-init will be start here 
 */
void media_bootstrap_start()
{
	/* tick frequency in 100 ms */
	timer_init(100);
	register_all_reqeust_handler();
}

/*
 * client destroy module when no longer used
 */
void media_bootstrap_clean()
{
	rpc_client_stop();
	timer_destroy();
}

int register_all_reqeust_handler()
{
//	rpc_client_add_cb(rpc_id_version, rpc_id_version_check, 
	//rpc_client_add_cb(category_event, category_event_sub_login, media_rpc_event_login_dispatcher, NULL);
	//rpc_client_add_cb(category_event, category_event_sub_logout, media_rpc_event_logout_dispatcher, NULL);
	//rpc_client_add_cb(category_event, category_event_sub_discon, media_rpc_event_discon_dispatcher, NULL);
	//rpc_client_add_cb(category_event, category_event_sub_update, media_rpc_event_update_dispatcher, NULL);
	//rpc_client_add_cb(category_event, category_event_sub_login_repeat, media_rpc_event_login_repeat_dispatcher, NULL);

	return 0;
}