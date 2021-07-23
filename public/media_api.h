/*
 * media module api 
 *
 * @author rui.sun 2012-7-3 Tue 11:12
 */
#ifndef MC_API_H
#define MC_API_H

#include "db_account_declare.h"
#include "session_online_declare.h"
#include "session_device_declare.h"

#ifdef  __cplusplus
# define __MC_BEGIN_DECLS  extern "C" {
# define __MC_END_DECLS    }
#else
# define __MC_BEGIN_DECLS
# define __MC_END_DECLS
#endif

__MC_BEGIN_DECLS

typedef struct st_timer timer_t;

typedef struct mc_session 
{
	char uuid[16 + 1];

	bool b_is_admin;
	char ref_admin_uuid[16 + 1];

	timer_t * auto_heartbeat;
} mc_session_t;

/* 以后要可以设置 */
#define RPC_REQUEST_DEF_TIMEOUT 5 * 1000 * 1000 // 5s

//typedef struct mc_event_t
//{
////	event_cb_type type;
//	mc_client_t client;
//} mc_event_t;

typedef enum mc_status
{
	mc_ok = 0,
	mc_server_not_response,
	mc_syntax_error,
	mc_internal_error,
	mc_no_permission,
	mc_session_expired,
	mc_invalid_uuid_or_pwd,
	mc_old_pwd_not_correct,
	mc_uuid_exist,
	mc_delete_cannot_be_owner,
	mc_uuid_not_exist,
	mc_workfolder_exist,
} mc_status;

typedef struct version_status
{
	char cur_ver[64];
	char new_ver[64];
	bool b_force_update;
	char update_logs[4096];
} version_status_t;

/*
 * fd close callback
 */
typedef void (* pfn_mc_close_cb)(void * opaque);

/*
 * connect to server
 *
 * @return
 *     mc_ok                   ok
 *     mc_server_not_response  server not response
 */
mc_status mc_connect_to_server(const char * ip, int port, int keepalive);

/*
 * disconnect from server
 */
void mc_disconnect();

/*
 * notify when fd close
 */
void mc_close_cb(pfn_mc_close_cb close_cb, void * opaque);

/*
 * get local version
 */
void mc_current_version(char version[64]);

/*
 * check newest version from server
 *
 * @return
 *     mc_ok                         check ok
 *     mc_server_not_response        server not response
 *     mc_syntax_error               syntax error
 */
mc_status mc_check_version(e_platfrom_id pid, version_status_t * version);

/*
 * login and get session instance
 *
 * @param [out]session
 *      return the malloc session if mc_ok, you need free it by
 *       mc_logout when it no longer be used
 *
 * @return
 *     mc_ok                         login ok
 *     mc_server_not_response        server not response
 *     mc_invalid_user_or_password   not valid user or password
 *     mc_syntax_error               syntax error
 *     mc_session_expired            session expired
 */
mc_status mc_account_login(const char * uuid, const char * pwd, e_platfrom_id pid, mc_session_t ** session);

/*
 * check the recently changed status when login success.
 *
 * @param[status]
 *   evcb will be ignore if status is not mc_ok, status maybe in blew.
 *     ok
 *     session expired
 *     syntax error
 */
//typedef void (* mc_event_cb)(void * userdata, mc_status status, const mc_event_t ** evcb, int evsize, int * abort);

/*
 * register server event callback
 */
//void mc_register_cb(const mc_session_t * session, mc_event_cb cb, void * userdata);

/*
 * create new user
 *
 * @param [in]session
 *      user session which got by mc_login, use this operate must be 
 *       have admin rights
 * @return
 *     mc_ok                         create ok
 *     mc_no_permission              no permission
 *     mc_uuid_exist                 uuid exist
 *     mc_syntax_error               syntax error
 *     mc_internal_error             internal error
 *     mc_session_expired            session expired
 */
mc_status mc_account_create(const mc_session_t * session, db_account_basic_t * child);

/*
 * modify user
 *
 * @return
 *     mc_ok                         login ok
 *     mc_server_not_response        server not response
 *     mc_invalid_user_or_password   not valid user or password
 *     mc_syntax_error               syntax error
 *     mc_internal_error             internal error
 *     mc_session_expired            session expired
 */
mc_status mc_account_modify(const mc_session_t * session, const char * old_pwd, db_account_basic_t * basic);

/*
 * delete user
 *
 * @param [out]session
 *      return the malloc session if mc_ok, you need free it by
 *       mc_logout when it no longer be used
 *
 * @return
 *     mc_ok                         ok
 *     mc_server_not_response        server not response
 *     mc_invalid_user_or_password   not valid user or password
 *     mc_no_permission              no permission
 *     mc_delete_cannot_be_root      cannot be root
 *     mc_syntax_error               syntax error
 *     mc_internal_error             internal error
 *     mc_session_expired            session expired
 */
mc_status mc_account_delete(const mc_session_t * session, const char * old_pwd, const char * del_uuid);

/*
 * check login power id
 *
 * @return
 *     mc_ok                         get power ok
 *     mc_syntax_error               syntax error
 */
mc_status mc_login_check_power(const mc_session_t * session, bool * b_is_admin);

/*
 * get all registered users
 *
 * @return
 *     mc_ok                         ok
 *     mc_server_not_response        server not response
 *     mc_no_permission              no permission
 *     mc_syntax_error               syntax error
 *     mc_internal_error             internal error
 *     mc_session_expired            session expired
 */
mc_status mc_account_list(const mc_session_t * session, online_session_t *** basic, int * count);

/*
 * free all users
 *
 * @return
 *     mc_ok                         free ok
 *     mc_syntax_error               syntax error
 */
mc_status mc_account_list_free(online_session_t ** basic, int count);

/*
 * logout from server, session will be set to NULL if mc_ok
 *
 * @return
 *     mc_ok                         logout ok
 *     mc_server_not_response        server no response
 *     mc_session_expired            session expired
 */
mc_status mc_account_logout(mc_session_t ** session);

/*
 * add a new device, user must be a admin.
 *
 * NOTE:
 *     device.basic.uuid will be ignore and automate generate by internal.
 *
 * @uuid
 *     uuid will be automate generate by internal
 *
 * @return
 *     mc_ok                         logout ok
 *     mc_server_not_response        server no response
 *     mc_no_permission              no permission
 *     mc_uuid_exist                 uuid exist
 *     mc_workfolder_exist           work folder exist
 *     mc_syntax_error               syntax error
 *     mc_internal_error             internal error
 *     mc_session_expired            session expired
 */
mc_status mc_device_add(const mc_session_t * session, const db_account_device_t * device, /*out*/char uuid[38]);

/*
 * delete device, user must be a admin.
 *
 * @return
 *     mc_ok                         logout ok
 *     mc_no_permission              no permission
 *     mc_uuid_not_exist             uuid not exist
 *     mc_syntax_error               syntax error
 *     mc_internal_error             internal error
 *     mc_session_expired            session expired
 */
mc_status mc_device_del(const mc_session_t * session, const char * uuid);

/*
 * list device, user must be a admin.
 *
 * @return
 *     mc_ok                         logout ok
 *     mc_server_not_response        server no response
 *     mc_no_permission              no permission
 *     mc_syntax_error               syntax error
 *     mc_internal_error             internal error
 *     mc_session_expired            session expired
 */
mc_status mc_device_list(const mc_session_t * session, db_account_device_t *** device, int * count);

/*
 * free all devices
 *
 * @return
 *     mc_ok                         free ok
 *     mc_syntax_error               syntax error
 */
mc_status mc_device_list_free(db_account_device_t ** device, int count);

/*
 * add a new device and try to work
 *
 * @rid
 *    ref success start ok and returned current ref
 *     -1 plugin not support
 *     -2 could not created working folder
 *     -3 login to device failed.
 *     -4 play device failed
 *     -5 internal error[check segmenter_api.c]
 *
 * @return
 *     mc_ok                         logout ok
 *     mc_server_not_response        server no response
 *     mc_syntax_error               syntax error
 *     mc_internal_error             internal error
 *     mc_session_expired            session expired
 */
mc_status mc_play_device(const mc_session_t * session, const char * uuid, int * rid);

/*
 * stop current device and dref, device will be stop when no ref longer.
 *
 * @rid
 *   ref return current ref, 0 is device stopped
 *    -1 device uuid not valid
 *
 * @return
 *     mc_ok                         logout ok
 *     mc_server_not_response        server no response
 *     mc_syntax_error               syntax error
 *     mc_internal_error             internal error
 *     mc_session_expired            session expired
 */
mc_status mc_play_stop(const mc_session_t * session, const char * uuid, int * rid);

/*
 * touch a device and check whether already work
 *
 * @rid
 *   ref device is current work status
 *     0 device is not work
 *    -1 device not exist
 *
 * @status
 *    just valid when in work status
 *
 * @return
 *     mc_ok                         logout ok
 *     mc_server_not_response        server no response
 *     mc_syntax_error               syntax error
 *     mc_internal_error             internal error
 *     mc_session_expired            session expired
 */
mc_status mc_play_touch(const mc_session_t * session, const char * uuid, int * rid, device_status_t * device);

__MC_END_DECLS

#endif