/*
 * db provider
 * rui.sun 2012_12_12 13:48
 */
#include "db_provider.h"
#include "db_provider_type.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#ifdef DB_PROVIDER_HAVE_MYSQL
extern const sql_provider_type_t mysql_provider;
#endif

#ifdef DB_PROVIDER_HAVE_SQLITE
extern const sql_provider_type_t sqlite_provider;
#endif

#ifdef DB_PROVIDER_HAVE_ORACLE
extern const sql_provider_type_t oracle_provider;
#endif

static const sql_provider_type_t * const providers[] =
{
#ifdef DB_PROVIDER_HAVE_MYSQL
	&mysql_provider,
#endif
#ifdef DB_PROVIDER_HAVE_SQLITE
	&sqlite_provider,
#endif
#ifdef DB_PROVIDER_HAVE_ORACLE
	&oracle_provider,
#endif
};

sql_provider_type_t * sql_get_type_by_provider(e_db_provider provider)
{
	int count_provider;
	int idx;

	count_provider = sizeof(providers) / sizeof(providers[0]);

	for (idx = 0; idx < count_provider; idx++)
	{
		if (providers[idx]->provider == provider)
		{
			return (sql_provider_type_t *)providers[idx];
		}
	}

	assert(0 && "not implement specified provider");
	return NULL;
}

#define sql_provider_dispath1(provider, func, param) \
{ \
	sql_provider_type_t * type; \
	type = sql_get_type_by_provider(provider); \
	return type->pfn_##func(param); \
}

#define sql_provider_dispath0_void(provider, func) \
{ \
	sql_provider_type_t * type; \
	type = sql_get_type_by_provider(provider); \
	type->pfn_##func(); \
}

#define sql_provider_dispath5_void(provider, func, param0, param1, param2, param3, param4) \
{ \
	sql_provider_type_t * type; \
	type = sql_get_type_by_provider(provider); \
	type->pfn_##func(param0, param1, param2, param3, param4); \
}

#define sql_dispatch0(sql, func) \
{ \
	sql_provider_type_t * type; \
	assert(sql != NULL); \
	type = sql->type; \
	return type->pfn_##func(sql);\
}

#define sql_dispatch1(sql, func, param) \
{ \
	sql_provider_type_t * type; \
	assert(sql != NULL); \
	type = sql->type; \
	return type->pfn_##func(sql, param);\
}

void sql_init(e_db_provider provider, 
	const char * ip, int port, 
	const char * user, const char * pwd, 
	const char * db)
{
 	sql_provider_dispath5_void(provider, sql_init, ip, port, user, pwd, db);
}

void sql_clean(e_db_provider provider)
{
	sql_provider_dispath0_void(provider, sql_clean);
}

bool sql_test_connect(e_db_provider provider, char err[64])
{
	sql_provider_dispath1(provider, sql_test_connect, err);
}

sql_t sql_create_new(e_db_provider provider, char err[64])
{
	sql_provider_dispath1(provider, sql_create_new, err);
}

e_sql_query_result sql_query(sql_t sql, const char * query_sql)
{
	sql_dispatch1(sql, sql_query, query_sql);
}

int sql_num_fields(sql_t sql)
{
	sql_dispatch0(sql, sql_num_fields);
}

/* use this like [while (sql_fetch_row) ;] */
bool sql_fetch_row(sql_t sql, sql_row_t ** row)
{
	sql_dispatch1(sql, sql_fetch_row, row);
}

bool sql_fetch_row_free(sql_t sql, sql_row_t * row)
{
	sql_dispatch1(sql, sql_fetch_row_free, row);
}

const char * sql_error(sql_t sql)
{
	sql_dispatch0(sql, sql_error);
}

void sql_free(sql_t * sql)
{
	sql_provider_type_t * type;

	if (sql != NULL && *sql != NULL)
	{
		type = (*sql)->type;
		type->pfn_sql_free(sql);
	}
}

//#define TEST_DB_PROVIDER

#ifdef TEST_DB_PROVIDER
void test_sql_provider()
{
	e_db_provider provider;
	bool r;
	int n;

	//{
	//	provider = provider_mysql;
	//	sql_init(provider, "127.0.0.1", 3306, "root", "root", "servicedb");
	//}
	{
		provider = provider_sqlite;
		sql_init(provider, NULL, 0, NULL, NULL, "d:/test_sql_provider.sqlite3");
	}

	r = sql_test_connect(provider, NULL);

	{
		e_sql_query_result query_r;
		sql_t sql;

		sql = sql_create_new(provider, NULL);

		query_r = sql_query(sql, "CREATE TABLE IF NOT EXISTS test(uuid TEXT, age INTEGER)");

		query_r = sql_query(sql, "INSERT INTO test (uuid,age) VALUES ('zhang3',18)");
		query_r = sql_query(sql, "INSERT INTO test (uuid,age) VALUES ('li4',19)");

		query_r = sql_query(sql, "SELECT * FROM test");

		n = sql_num_fields(sql);

		{
			sql_row_t * row;

			while (sql_fetch_row(sql, &row))
			{
				int i;

				for (i = 0; i < n; i++)
				{
					fprintf(stderr, "key = %s(%d).\n", row[i].data, row[i].length);
				}

				sql_fetch_row_free(sql, row);
			}
		}

		sql_free(&sql);

		sql_clean(provider);
	}
}
#endif