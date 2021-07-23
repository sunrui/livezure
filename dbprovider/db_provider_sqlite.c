/*
 * db provider for sqlite3
 * rui.sun 2012_12_13 15:28
 */
#include "db_provider_type.h"

#ifdef DB_PROVIDER_HAVE_SQLITE

#ifdef WIN32
#pragma comment(lib, "sqlite3.lib")
#endif

#include <sqlite3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <io.h>
#include <direct.h>

static char sql_db[260 + 1];
sqlite3 * def_sqlite3_db;
char sql_err[64 + 1];

typedef struct sqlite_param
{	
	sqlite3_stmt * stmt;
	const char * zTail;

	int fields;
	int row;
} sqlite_param_t;

extern const sql_provider_type_t sqlite_provider;

void sql_clean_sqlite();

#ifdef WIN32
#include <windows.h>

#define mkdir _mkdir
#define strdup _strdup

#define pthread_mutex_t CRITICAL_SECTION
#define pthread_mutex_init(mutex, attr) InitializeCriticalSection(mutex)
#define pthread_mutex_lock(mutex) EnterCriticalSection(mutex)
#define pthread_mutex_unlock(mutex) LeaveCriticalSection(mutex)
#define pthread_metex_destroy(mutex) DeleteCriticalSection(mutex)
#endif

pthread_mutex_t access_lock;

void sql_init_sqlite(const char * ip, int port, const char * user, const char * pwd, const char * db)
{
	assert(db != NULL);

	strncpy(sql_db, db, sizeof(sql_db) - 1);

	sql_clean_sqlite();
	if (def_sqlite3_db == NULL)
	{
		int db_if_exists;
		int r;

		db_if_exists = 1;
		if (_access(sql_db, 0) != 0)
		{
			db_if_exists = 0;
		}

		if (!db_if_exists)
		{
			char * path;
			char * tail;

			path = strdup(sql_db);;
			r = 0;
			tail = strrchr(path, '\\');
			if (tail == NULL) tail = strrchr(path, '/');
			if (tail != NULL) *tail = 0;
			if (_access(path, 0) == -1) r = mkdir(path);
			free(path);
			if (r != 0)
			{
				const char * err_str = "failed to open current directory.";
				strncpy(sql_err, err_str, strlen(err_str) > 64 ? 64 : strlen(err_str));
				fprintf(stderr, "%s\n", err_str);
				return;
			}
		}

		sqlite3_config(SQLITE_CONFIG_SERIALIZED);
		r = sqlite3_open(sql_db, &def_sqlite3_db);
		if(r != 0)
		{
			const char * sqlite3_err;
			int sqlite3_err_size;
			sqlite3_err = (char *)sqlite3_errmsg(def_sqlite3_db);
			sqlite3_err_size = strlen(sqlite3_err);
			strncpy(sql_err, sqlite3_err, sqlite3_err_size > 64 ? 64 : sqlite3_err_size);

			sqlite3_close(def_sqlite3_db);
			def_sqlite3_db = NULL;
		}

		pthread_mutex_init(&access_lock, NULL);
	}
}

void sql_clean_sqlite()
{
	if (def_sqlite3_db != NULL)
	{
		sqlite3_close(def_sqlite3_db);
		def_sqlite3_db = NULL;

		pthread_metex_destroy(&access_lock);
	}
}

sql_t sql_create_new_sqlite(char err[64])
{
	sqlite_param_t * param;
	sql_t sql;

	if (def_sqlite3_db == NULL)
	{
		assert(0 && sql_err);
		if (err != NULL)
		{
			strncpy(err, sql_err, 64);
		}

		return NULL;
	}

	param = (sqlite_param_t *)calloc(1, sizeof(sqlite_param_t));
	sql = (sql_t)calloc(1, sizeof(sql_inst_t));
	sql->real_handle = (int)param;
	sql->type = (sql_provider_type_t *)&sqlite_provider;

	pthread_mutex_lock(&access_lock);

	return sql;
}

bool sql_test_connect_sqlite(char err[64])
{
	sql_t sql = sql_create_new_sqlite(err);
	if (sql != NULL)
	{
		sql_free(&sql);
		return true;
	}

	return false;
}

e_sql_query_result sql_query_sqlite(sql_t sql, const char * query_sql)
{
	sqlite_param_t * param;
	int r;

	assert(sql != NULL && query_sql != NULL);

	param = (sqlite_param_t *)sql->real_handle;
	r = sqlite3_reset(param->stmt);
	r = sqlite3_finalize(param->stmt);
	r = sqlite3_prepare(def_sqlite3_db, query_sql, -1,&param->stmt, &param->zTail);
	param->row = sqlite3_step(param->stmt);
	param->fields = sqlite3_data_count(param->stmt);
	
	if (param->row == SQLITE_DONE)
	{
		return e_query_done;
	}
	else if (param->row == SQLITE_ROW)
	{
		return e_query_row;
	}
	else
	{
		fprintf(stderr, "%s\n", sql_error(sql));
		return e_query_error;
	}
}

int sql_num_fields_sqlite(sql_t sql)
{
	sqlite_param_t * param;

	assert(sql != NULL);
	param = (sqlite_param_t *)sql->real_handle;

	if (sql != NULL && param->stmt != NULL)
	{
		return param->fields;
	}

	return 0;
}

bool sql_fetch_row_sqlite(sql_t sql, sql_row_t ** row)
{
	sqlite_param_t * param;
	int fields;

	param = (sqlite_param_t *)sql->real_handle;
	if (param->row != SQLITE_ROW)
	{
		return false;
	}

	fields = sql_num_fields(sql);
	assert(fields != 0);
	if (row != NULL)
	{
		int length;
		int idx;
		sql_row_t * myrow;

		myrow = (sql_row_t *)calloc(1, fields * sizeof(sql_row_t));
		for (idx = 0; idx < fields; idx++)
		{
			const char * data;

			data = (const char *)sqlite3_column_text(param->stmt, idx);
			assert(data != NULL);
			length = sqlite3_column_bytes(param->stmt, idx);
			myrow[idx].data = (char *)strdup(data);
			myrow[idx].length = length;
		}

		*row = myrow;
	}
	param->row = sqlite3_step(param->stmt);

	return true;
}

bool sql_fetch_row_free_sqlite(sql_t sql, sql_row_t * row)
{
	assert(row != NULL);

	if (row != NULL)
	{
		int fields;
		int idx;
		
		fields = sql_num_fields(sql);
		assert(fields != 0);
		for (idx = 0; idx < fields; idx++)
		{
			free(row[idx].data);
		}
		free(row);
		return true;
	}

	return false;
}

const char * sql_error_sqlite(sql_t sql)
{
	sqlite_param_t * param;

	assert(sql != NULL);

	param = (sqlite_param_t *)sql->real_handle;
	strncpy(sql_err, sqlite3_errmsg(def_sqlite3_db), sizeof(sql_err) - 1);

	return sql_err;
}

void sql_free_sqlite(sql_t * sql)
{
	assert(sql != NULL && *sql != NULL);

	if (sql != NULL && *sql != NULL)
	{
		sqlite_param_t * param;
		param = (sqlite_param_t *)(*sql)->real_handle;
		sqlite3_reset(param->stmt);
		sqlite3_finalize(param->stmt);
		free(param);
		free(*sql);
		*sql = NULL;

		pthread_mutex_unlock(&access_lock);
	}
}

const sql_provider_type_t sqlite_provider = {
	provider_sqlite,
	sql_init_sqlite,
	sql_clean_sqlite,
	sql_test_connect_sqlite,
	sql_create_new_sqlite,
	sql_query_sqlite,
	sql_num_fields_sqlite,
	sql_fetch_row_sqlite,
	sql_fetch_row_free_sqlite,
	sql_error_sqlite,
	sql_free_sqlite
};

#endif