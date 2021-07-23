/*
 * service api
 *
 * update 2012-6-15 rui.sun <smallrui@live.com>
 */
#include "posixinc.h"
#include "service_api.h"
#include "log4c_extend.h"

#include "service_req_declare.h"
#include "service_session.h"
#include "service_device.h"
#include "timer.h"
#include "rpc_service.h"
#include "db_account.h"

/*
 * register all request handler
 */
int register_all_reqeust_handler()
{
	/* platform category */
	rpc_service_register(rpc_id_platform, rpc_id_version_check, req_version_check, NULL);
	rpc_service_register(rpc_id_platform, rpc_id_heartbeat, req_heartbeat, NULL);

	/* account category */
	rpc_service_register(rpc_id_account, rpc_id_account_login, req_account_login, NULL);
	rpc_service_register(rpc_id_account, rpc_id_account_logout, req_account_logout, NULL);
	rpc_service_register(rpc_id_account, rpc_id_account_create, req_account_create, NULL);
	rpc_service_register(rpc_id_account, rpc_id_account_modify, req_account_modify, NULL);
	rpc_service_register(rpc_id_account, rpc_id_account_delete, req_account_delete, NULL);
	rpc_service_register(rpc_id_account, rpc_id_account_list, req_account_list, NULL);

	/* device category */
	rpc_service_register(rpc_id_device, rpc_id_device_create, req_device_create, NULL);
	rpc_service_register(rpc_id_device, rpc_id_device_delete, req_device_delete, NULL);
	rpc_service_register(rpc_id_device, rpc_id_device_list, req_device_list, NULL);

	/* play category */
	rpc_service_register(rpc_id_play, rpc_id_play_device, req_play_device, NULL);
	rpc_service_register(rpc_id_play, rpc_id_play_stop, req_play_stop, NULL);
	rpc_service_register(rpc_id_play, rpc_id_play_touch, req_play_touch, NULL);

	return 0;
}

/*
 * all server pre-init module will be start here 
 */
void service_bootstrap()
{
	/* tick frequency in 100 ms */
	timer_init(100);

	online_session_init();
	session_device_init();

	/* init all request handler */
	register_all_reqeust_handler();

	/* init all db table */
	db_account_ensure_table_exist();
}

/*
 * server destroy module when no longer used
 */
void service_clean()
{
	online_session_destroy();
	session_device_destroy();
	timer_destroy();
}

static int service_started = 0;

/*
 * start rpc engine
 *
 * @keepalive	>0 if need keepalive in seconds
 * @nprocessors how many threads for processors
 *
 * @return
 *     0 start ok
 *    -1 socket error
 *    -2 bind error
 *    -3 listen error
 */
int service_start_async(int listenport, int keepalive, int nprocessors)
{
	if (!service_started)
	{
		int ret;

		ret = rpc_service_start(listenport, keepalive, nprocessors);
		if (ret == 0)
		{
			service_bootstrap();
			service_started = 1;
		}

		return ret;
	}

	return 0;
}

/*
 * @return stop status 
 *    0 stop ok
 *	 -1 service start maybe with issues
 */
int service_stop()
{
	if (service_started)
	{
		rpc_service_stop();
		service_clean();
		LOGI("service stop.");
		service_started = 0;
	}

	return 0;
}
