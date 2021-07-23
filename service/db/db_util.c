#include "db_util.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

void db_init(const char * ip, int port, const char * user, const char * pwd, const char * db)
{
	sql_init(def_provider, ip, port, user, pwd, db);
}

void db_clean()
{
	sql_clean(def_provider);
}

/*
 * @return
 *    1 connect ok
 *    0 failed
 */
int db_test_connect(char err[64])
{
	return sql_test_connect(def_provider, err);
}

e_sql_query_result db_exec(const char * query_sql, char err[64])
{
	sql_t sql;
	e_sql_query_result query_r;

	query_r = e_query_error;
	sql = sql_create_new(def_provider, err);
	if (sql == NULL)
	{
		assert(0);
		goto exit_func;
	}

	query_r = sql_query(sql, query_sql);
	if (query_r != e_query_done)
	{
		if (err != NULL)
		{
			strcpy(err, sql_error(sql));
		}
	}

exit_func:
	sql_free(&sql);
	return query_r;
}

#ifdef _DEBUG

extern void test_acount_all();

void test_db_all()
{
	const char * db = "D:\\Develop\\Project\\Work\\rpcmodule\\servicedb.db";
	remove(db);

	sql_init(def_provider, NULL, 0, NULL, NULL, db);

	test_acount_all();

	sql_clean(def_provider);
}

#endif
