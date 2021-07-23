/*
 * online session mgr
 * rui.sun 2012_12_21 4:19
 */
#ifndef ONLINE_SESSION_DECLARE_H
#define ONLINE_SESSION_DECLARE_H

#include "db_account_declare.h"
#include <stdbool.h>

typedef struct online_net_basic
{
	int fd; /* user socket id */
	char ip[16 + 1]; /* session ip address */
	int port; /* session port */

	bool is_heartbeated; /* whether session has been heartbeat here */
} online_net_basic_t;

typedef struct online_session
{
	db_account_basic_t basic;

	char ref_admin_uuid[16 + 1];
	bool is_admin;

	bool is_online;
	online_net_basic_t net;
} online_session_t;

#endif