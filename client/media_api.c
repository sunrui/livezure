/*
 * media module api 
 *
 * @author rui.sun 2012-7-3 Tue 11:12
 */
#include "media_api.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "rpc_declare.h"

#include "vld.h"
#include "media_bootstrap.h"
#include "rpc_client.h"
#include "rpc_request.h"
#include "log4c_extend.h"
#include "rpc_keypair.h"

static int mc_env_inited = 0;

/*
 * initialize media core environment
 */
void mc_env_init()
{
	if (!mc_env_inited)
	{
		media_bootstrap_start();
		mc_env_inited = 1;
	}
}

/*
 * destroy media core environment
 */
void mc_env_destroy()
{
	if (mc_env_inited)
	{
		media_bootstrap_clean();
		mc_env_inited = 0;
	}
}

/*
 * connect to server
 *
 * @return
 *     mc_ok                   ok
 *     mc_server_not_response  server not response
 */
mc_status mc_connect_to_server(const char * ip, int port, int keepalive)
{
	mc_status status;
	int r;

	assert(ip != NULL && port > 0);

	mc_env_init();
	if (keepalive <= 0) keepalive = 60;
	r = rpc_client_start(ip, port, keepalive);
	status = (r == 0) ? mc_ok : mc_server_not_response;

	return status;
}

/*
 * disconnect from server
 */
void mc_disconnect()
{
	mc_env_destroy();
	rpc_client_stop();
}

/*
 * notify when fd close
 */
void mc_close_cb(pfn_mc_close_cb close_cb, void * opaque)
{
	rpc_client_event(close_cb, opaque);
}