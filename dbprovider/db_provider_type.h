/*
 * db provider types defination
 * rui.sun 2012_12_12 13:48
 */
#ifndef DB_PROVIDER_TYPE_H
#define DB_PROVIDER_TYPE_H

#include "db_provider.h"
#include <stdbool.h>

typedef struct sql_provider_type
{
	e_db_provider provider;

	void (* pfn_sql_init)(const char * ip, int port, const char * user, const char * pwd, const char * db);
	void (* pfn_sql_clean)();
	bool (* pfn_sql_test_connect)(char err[64]);
	sql_t (* pfn_sql_create_new)(char err[64]);
	e_sql_query_result (* pfn_sql_query)(sql_t sql, const char * query_sql);
	int (* pfn_sql_num_fields)(sql_t sql);
	bool (* pfn_sql_fetch_row)(sql_t sql, sql_row_t ** row);
	bool (* pfn_sql_fetch_row_free)(sql_t sql, sql_row_t * row);
	const char * (* pfn_sql_error)(sql_t sql);
	void (* pfn_sql_free)(sql_t * sql);
} sql_provider_type_t;

typedef struct sql_inst
{
	sql_provider_type_t * type;
	int real_handle;
} sql_inst_t;

#endif