/*
 * db provider for oracle
 * rui.sun 2012_12_12 13:48
 */
#include "db_provider_type.h"

#ifdef DB_PROVIDER_HAVE_ORACLE

#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
//#include <oracle.h>

static char sql_ip[32 + 1];
static int sql_port;
static char sql_user[32 + 1];
static char sql_pwd[32 + 1];
static char sql_db[33 + 1];

typedef struct oracle_param
{

	char err[64 + 1];
} oracle_param_t;

extern const sql_provider_type_t oracle_provider;

void sql_init_oracle(const char * ip, int port, const char * user, const char * pwd, const char * db)
{
	assert(ip != NULL && port != 0 && user != NULL && pwd != NULL && db != NULL);

	strncpy(sql_ip, ip, sizeof(sql_ip) - 1);
	sql_port = port;
	strncpy(sql_user, user, sizeof(sql_user) - 1);
	strncpy(sql_pwd, pwd, sizeof(sql_pwd) - 1);
	strncpy(sql_db, db, sizeof(sql_db) - 1);
}

void sql_clean_oracle()
{
}

sql_t sql_create_new_oracle(char err[64])
{
	assert(0 && "no implement db provider for oracle.");
	return NULL;
}

bool sql_test_connect_oracle(char err[64])
{
	sql_t sql = sql_create_new_oracle(err);
	if (sql != NULL)
	{
		sql_free(&sql);
		return true;
	}

	return false;
}

e_sql_query_result sql_query_oracle(sql_t sql, const char * query_sql)
{
	return e_query_error;
}

int sql_num_fields_oracle(sql_t sql)
{
	return 0;
}

bool sql_fetch_row_oracle(sql_t sql, sql_row_t ** row)
{
	return false;
}

bool sql_fetch_row_free_oracle(sql_t sql, sql_row_t * row)
{
	return false;
}

const char * sql_error_oracle(sql_t sql)
{
	return NULL;
}

void sql_free_oracle(sql_t * sql)
{
}

const sql_provider_type_t oracle_provider = {
	provider_oracle,
	sql_init_oracle,
	sql_clean_oracle,
	sql_test_connect_oracle,
	sql_create_new_oracle,
	sql_query_oracle,
	sql_num_fields_oracle,
	sql_fetch_row_oracle,
	sql_fetch_row_free_oracle,
	sql_error_oracle,
	sql_free_oracle
};

#endif