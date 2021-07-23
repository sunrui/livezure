#include "service_api.h"
#include "log4c_extend.h"

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

int init_log_module()
{
	logc_init(NULL, "d://develop//livezure_centre.log");

	return 1;
}

int init_connectin_db()
{
	const char * ip;
	int port;
	const char * user;
	const char * pwd;
	const char * db;

	ip = "127.0.0.1";
	port = 13350;
	user = "admin";
	pwd = "admin";
	db = "D:\\Develop\\livezure_default.db";

	db_init(ip, port, user, pwd, db);

	{
		char err[65] = { 0 };
		if (!db_test_connect(err))
		{
			LOGE("connect to db failed.");
			exit(0);
		}
	}

	return 1;
}

int init_server_centre()
{
	int listenport = 13404;
	int keepalive = 30;
	int start_r;

	start_r = service_start_async(listenport, keepalive, 10);
	if (start_r != 0)
	{
		LOGE("start server failed.");
		exit(0);
	}

	return 1;
}

int main(int argc, char * argv[])
{
	init_log_module();
	init_connectin_db();
	init_server_centre();

	printf("init ok.\n");

	Sleep(30 * 1000);

	service_stop();

	return 0;
}