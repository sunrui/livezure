/*
 * db provider
 * rui.sun 2012_12_12 13:48
 */
#ifndef DB_PROVIDER_H
#define DB_PROVIDER_H

#ifdef  __cplusplus
# define __DB_PROVIDER_BEGIN_DECLS  extern "C" {
# define __DB_PROVIDER_END_DECLS    }
#else
# define __DB_PROVIDER_BEGIN_DECLS
# define __DB_PROVIDER_END_DECLS
#endif

#include "db_provider_config.h"

__DB_PROVIDER_BEGIN_DECLS

typedef enum e_db_provider
{
#ifdef DB_PROVIDER_HAVE_MYSQL
	provider_mysql,
#endif
#ifdef DB_PROVIDER_HAVE_ORACLE
	provider_oracle,
#endif
#ifdef DB_PROVIDER_HAVE_SQLITE
	provider_sqlite,
#endif
} e_db_provider;

#include <stdbool.h>

typedef struct sql_inst * sql_t;

typedef struct sql_row
{
	char * data;
	int length;
} sql_row_t;

void sql_init(e_db_provider provider, 
	const char * ip, int port, 
	const char * user, const char * pwd, 
	const char * db);

void sql_clean(e_db_provider provider);
bool sql_test_connect(e_db_provider provider, char err[64]);

sql_t sql_create_new(e_db_provider provider, char err[64]);

typedef enum e_sql_query_result
{
	e_query_done,
	e_query_row,
	e_query_error
} e_sql_query_result;

e_sql_query_result sql_query(sql_t sql, const char * query_sql);

int sql_num_fields(sql_t sql);

/* use this like [while (sql_fetch_row) ;] */
bool sql_fetch_row(sql_t sql, sql_row_t ** row);
bool sql_fetch_row_free(sql_t sql, sql_row_t * row);

const char * sql_error(sql_t sql);

void sql_free(sql_t * sql);

__DB_PROVIDER_END_DECLS

#endif