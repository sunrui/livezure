/*
 * db util
 * rui.sun 2012_12_12 13:48
 */
#ifndef DB_UTIL_H
#define DB_UTIL_H

#include "db_provider.h"

/* default provider */
#define def_provider provider_sqlite

/*
 * db exec wrapper
 */
e_sql_query_result db_exec(const char * query_sql, char err[64]);

#endif