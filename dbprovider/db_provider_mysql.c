/*
 * db provider for mysql
 * rui.sun 2012_12_12 13:48
 */
#include "db_provider_type.h"

#ifdef DB_PROVIDER_HAVE_MYSQL

#ifdef WIN32
#pragma comment(lib, "libmysql.lib")
#endif

#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mysql.h>

static char sql_ip[32 + 1];
static int sql_port;
static char sql_user[32 + 1];
static char sql_pwd[32 + 1];
static char sql_db[33 + 1];
const char * unix_socket = NULL; // only used for Linux
unsigned long client_flag = 0;

typedef struct mysql_param
{
	MYSQL mysql;
	MYSQL * sock;

	MYSQL_RES * result;
	MYSQL_ROW row;

	char err[64 + 1];
} mysql_param_t;

extern const sql_provider_type_t mysql_provider;

void sql_init_mysql(const char * ip, int port, const char * user, const char * pwd, const char * db)
{
	assert(ip != NULL && port != 0 && user != NULL && pwd != NULL && db != NULL);

	strncpy(sql_ip, ip, sizeof(sql_ip) - 1);
	sql_port = port;
	strncpy(sql_user, user, sizeof(sql_user) - 1);
	strncpy(sql_pwd, pwd, sizeof(sql_pwd) - 1);
	strncpy(sql_db, db, sizeof(sql_db) - 1);

	{
		/* notice this function is not thread-safe */
		MYSQL mysql;
		mysql_init(&mysql);
	}
}

void sql_clean_mysql()
{
}

sql_t sql_create_new_mysql(char err[64])
{
	mysql_param_t * param;
	sql_t sql;
	MYSQL mysql, *sock;

	mysql_init(&mysql);
	sock = mysql_real_connect(&mysql, sql_ip, sql_user, sql_pwd, sql_db, sql_port, unix_socket, client_flag);
	if (sock == NULL)
	{
		if (err != NULL)
		{
			strncpy(err, mysql_error(&mysql), 64 - 1);
		}

		fprintf(stderr, "%s\n", mysql_error(&mysql));
		return NULL;
	}

	sql = (sql_t)calloc(1, sizeof(sql_inst_t));
	param = (mysql_param_t *)calloc(1, sizeof(mysql_param_t));
	param->mysql = mysql;
	param->sock = sock;
	sql->real_handle = (int)param;
	sql->type = (sql_provider_type_t *)&mysql_provider;

	return sql;
}

bool sql_test_connect_mysql(char err[64])
{
	sql_t sql = sql_create_new_mysql(err);
	if (sql != NULL)
	{
		sql_free(&sql);
		return true;
	}

	return false;
}

e_sql_query_result sql_query_mysql(sql_t sql, const char * query_sql)
{
	mysql_param_t * param;
	int r;

	assert(sql != NULL && query_sql != NULL);

	param = (mysql_param_t *)sql->real_handle;
	r = mysql_query(&param->mysql, query_sql);
	if (r != 0)
	{
		fprintf(stderr, "%s\n", sql_error(sql));
		return e_query_error;
	}

	param->result = mysql_store_result(&param->mysql);
	if (param->result != NULL)
	{
		return e_query_row;
	}
	
	return e_query_done;
}

int sql_num_fields_mysql(sql_t sql)
{
	mysql_param_t * param;

	assert(sql != NULL);
	param = (mysql_param_t *)sql->real_handle;

	if (sql != NULL && param->result != NULL)
	{
		return mysql_num_fields(param->result);
	}

	return 0;
}

bool sql_fetch_row_mysql(sql_t sql, sql_row_t ** row)
{
	mysql_param_t * param;
	int fields;

	param = (mysql_param_t *)sql->real_handle;
	param->row = mysql_fetch_row(param->result);
	if (param->row == NULL)
	{
		return false;
	}

	fields = sql_num_fields(sql);
	assert(fields != 0);
	if (row != NULL)
	{
		unsigned long * length;
		int idx;
		sql_row_t * myrow;

		myrow = (sql_row_t *)calloc(1, fields * sizeof(sql_row_t));
		length = mysql_fetch_lengths(param->result);

		for (idx = 0; idx < fields; idx++)
		{
			myrow[idx].data = param->row[idx];
			myrow[idx].length = length[idx];
		}

		*row = myrow;
	}

	return true;
}

bool sql_fetch_row_free_mysql(sql_t sql, sql_row_t * row)
{
	assert(row != NULL);

	if (row != NULL)
	{
		free(row);
		return true;
	}

	return false;
}

const char * sql_error_mysql(sql_t sql)
{
	mysql_param_t * param;

	assert(sql != NULL);

	param = (mysql_param_t *)sql->real_handle;
	strncpy(param->err, mysql_error(&param->mysql), sizeof(param->err) - 1);

	return param->err;
}

void sql_free_mysql(sql_t * sql)
{
	assert(sql != NULL && *sql != NULL);

	if (sql != NULL && *sql != NULL)
	{
		mysql_param_t * param;
		param = (mysql_param_t *)(*sql)->real_handle;
		mysql_free_result(param->result);
		mysql_close(&param->mysql);
		free(param);
		free(*sql);
		*sql = NULL;
	}
}

const sql_provider_type_t mysql_provider = {
	provider_mysql,
	sql_init_mysql,
	sql_clean_mysql,
	sql_test_connect_mysql,
	sql_create_new_mysql,
	sql_query_mysql,
	sql_num_fields_mysql,
	sql_fetch_row_mysql,
	sql_fetch_row_free_mysql,
	sql_error_mysql,
	sql_free_mysql
};

#endif