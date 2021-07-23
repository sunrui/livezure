#include "db_account.h"

#include "db_util.h"
#include "db_provider.h"
#include "log4c_extend.h"
#include "hashkey.h"
#include "xmalloc.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define t_account_license          "t_account_license"
#define t_account_basic            "t_account_basic"
#define t_account_admin            "t_account_admin"
#define t_account_child            "t_account_child"
#define t_account_child_permission "t_account_child_permission"
#define t_account_device           "t_account_device"
#define t_device_basic             "t_device_basic"

#if defined(_DEBUG) || defined(DEBUG)
#define account_gen_hash_key gen_hash_key_do_nothing

void gen_hash_key_do_nothing(char * hashpwd, const char * pwd, int length)
{
	assert(hashpwd != NULL && pwd != NULL);
	if (length <= 0)
	{
		length = strlen(pwd);
	}

	memcpy(hashpwd, pwd, length + 1);
}
#else
#define account_gen_hash_key gen_hash_key
#endif

void world_tolower(char * p)
{
	assert(p != NULL);

	do
	{
		*p = tolower(*p);
	} while (*p++);
}

void db_account_ensure_table_exist()
{
	const char * query_sql;
	e_sql_query_result query_r;

	/* ensure t_account_license_type exist */
	{
		query_sql = "DROP TABLE IF EXISTS "
			t_account_license
			;
		query_r = db_exec(query_sql, NULL);
		assert(query_r == e_query_done);

		query_sql =
			"CREATE TABLE IF NOT EXISTS "
			t_account_license
			" (license_type INTEGER PRIMARY KEY,"
			"comment TEXT"
			")";
		query_r = db_exec(query_sql, NULL);
		assert(query_r == e_query_done);

		query_sql =
			"INSERT INTO "
			t_account_license
			" (license_type,comment) VALUES ("
			"0,"
			"'this license is now must be checking by service root.')";
		query_r = db_exec(query_sql, NULL);
		assert(query_r == e_query_done);

		query_sql =
			"INSERT INTO "
			t_account_license
			" (license_type,comment) VALUES ("
			"1,"
			"'this license have expired time limited.')";
		query_r = db_exec(query_sql, NULL);
		assert(query_r == e_query_done);

		query_sql =
			"INSERT INTO "
			t_account_license
			" (license_type,comment) VALUES ("
			"2,"
			"'this license is for ever for use')";
		query_r = db_exec(query_sql, NULL);
		assert(query_r == e_query_done);
	}

	/* ensure t_account_basic exist */
	{
		query_sql =
			"CREATE TABLE IF NOT EXISTS "
			t_account_basic
			" (uuid TEXT PRIMARY KEY,"
			"pwd TEXT,"
			"nickname TEXT,"
			"comment TEXT,"
			"createtime TIMESTAMPE NOT NULL DEFAULT (datetime('now','localtime'))"
			")";

		query_r = db_exec(query_sql, NULL);
		assert(query_r == e_query_done);
	}

	/* ensure t_account_admin exist */
	{
		query_sql =
			"CREATE TABLE IF NOT EXISTS "
			t_account_admin
			" (uuid TEXT PRIMARY KEY,"
			"license_type INTEGER,"
			"licensed_time TEXT,"
			"expired_time TEXT,"
			"create_time TIMESTAMPE NOT NULL DEFAULT (datetime('now','localtime'))"
			")";

		query_r = db_exec(query_sql, NULL);
		assert(query_r == e_query_done);
	}

	/* ensure t_account_child exist */
	{
		query_sql =
			"CREATE TABLE IF NOT EXISTS "
			t_account_child
			" (uuid TEXT PRIMARY KEY,"
			"ref_admin_uuid,"
			"create_time TIMESTAMPE NOT NULL DEFAULT (datetime('now','localtime'))"
			")";

		query_r = db_exec(query_sql, NULL);
		assert(query_r == e_query_done);
	}

	/* ensure t_account_child_permission exist */
	{
		query_sql =
			"CREATE TABLE IF NOT EXISTS "
			t_account_child_permission
			" (child_uuid TEXT PRIMARY KEY,"
			"can_view_ba INTEGER,"
			"can_view_record INTEGER,"
			"can_ptz_ba INTEGER,"
			"can_task_record INTEGER"
			")";

		query_r = db_exec(query_sql, NULL);
		assert(query_r == e_query_done);
	}

	/* ensure t_device_basic exist */
	{
		query_sql =
			"CREATE TABLE IF NOT EXISTS "
			t_device_basic
			" (uuid TEXT PRIMARY KEY,"
			"plugin_name TEXT,"
			"ip TEXT,"
			"port INTEGER,"
			"user TEXT,"
			"pwd TEXT,"
			"channel INTEGER,"
			"seg_folder TEXT,"
			"seg_in_count INTEGER,"
			"seg_per_sec INTEGER,"
			"create_time TIMESTAMPE NOT NULL DEFAULT (datetime('now','localtime'))"
			")";

		query_r = db_exec(query_sql, NULL);
		assert(query_r == e_query_done);
	}

	/* ensure t_account_device exist */
	{
		query_sql =
			"CREATE TABLE IF NOT EXISTS "
			t_account_device
			" (uuid TEXT PRIMARY KEY,"
			"ref_admin_uuid TEXT,"
			"nickname TEXT,"
			"comment TEXT,"
			"is_top_level INTEGER,"
			"ref_parent_uuid TEXT,"
			"createtime TIMESTAMPE NOT NULL DEFAULT (datetime('now','localtime'))"
			")";

		query_r = db_exec(query_sql, NULL);
		assert(query_r == e_query_done);
	}
}

void db_account_reset()
{
	const char * query_sql;
	e_sql_query_result query_r;

	/* delete t_account_license */
	{
		query_sql =
			"DROP TABLE IF EXISTS "
			t_account_license
			;

		query_r = db_exec(query_sql, NULL);
		assert(query_r == e_query_done);
	}

	/* delete t_account_basic */
	{
		query_sql =
			"DROP TABLE IF EXISTS "
			t_account_admin
			;

		query_r = db_exec(query_sql, NULL);
		assert(query_r == e_query_done);
	}

	/* delete t_account_admin */
	{
		query_sql =
			"DROP TABLE IF EXISTS "
			t_account_admin
			;

		query_r = db_exec(query_sql, NULL);
		assert(query_r == e_query_done);
	}

	/* delete t_account_child */
	{
		query_sql =
			"DROP TABLE IF EXISTS "
			t_account_child
			;

		query_r = db_exec(query_sql, NULL);
		assert(query_r == e_query_done);
	}

	/* delete t_account_child_permission */
	{
		query_sql =
			"DROP TABLE IF EXISTS "
			t_account_child_permission
			;

		query_r = db_exec(query_sql, NULL);
		assert(query_r == e_query_done);
	}

	/* delete t_device_basic */
	{
		query_sql =
			"DROP TABLE IF EXISTS "
			t_device_basic
			;

		query_r = db_exec(query_sql, NULL);
		assert(query_r == e_query_done);
	}

	/* delete t_account_device */
	{
		query_sql =
			"DROP TABLE IF EXISTS "
			t_account_device
			;

		query_r = db_exec(query_sql, NULL);
		assert(query_r == e_query_done);
	}
}

bool db_account_admin_add(const db_account_basic_t * account)
{
	e_sql_query_result query_r;
	bool r;

	char sqlbuf[4096];
	const char * sql;
	char hashpwd[33];
	char lower_uuid[17];

	assert(account != NULL);

	/* check whether user already exist */
	{
		db_account_license_t license;
		r = db_account_license_get(account->uuid, &license);
		if (r)
		{
			LOGW("add admin '%s' already exist.", account->uuid);
			return false;
		}
	}

	hashpwd[32] = 0;
	account_gen_hash_key(hashpwd, account->pwd, -1);

	assert(strlen(account->uuid) <= 16);
	strncpy(lower_uuid, account->uuid, sizeof(lower_uuid) - 1);
	world_tolower(lower_uuid);

	sql = "INSERT INTO "
		t_account_basic
		"(uuid,pwd,nickname,comment) VALUES ('%s','%s','%s','%s')";
	sprintf(sqlbuf, sql, lower_uuid, hashpwd, account->nickname, account->comment);

	query_r = db_exec(sqlbuf, NULL);
	r = (query_r == e_query_done);
	assert(r);

	/* add default checking license */
	{
		db_account_license_t license;
		license.license_type = e_license_checking;
		license.licensed_time[0] = 0;
		license.expired_time[0] = 0;
		db_account_license_set(lower_uuid, &license);
	}

	LOGI("[DB] add admin '%s'.", account->uuid, r ? "ok" : "failed");

	return r;
}

/*
 * NOTE: this operation will delete all child which ref to admin
 */
bool db_account_admin_del(const char * uuid)
{
	e_sql_query_result query_r;
	bool r;

	char sqlbuf[4096];
	const char * sql;

	sql = "DELETE FROM "
		t_account_basic
		" WHERE uuid='%s'";
	sprintf(sqlbuf, sql, uuid);
	query_r = db_exec(sqlbuf, NULL);
	r = (query_r == e_query_done);
	assert(r);

	sql = "DELETE FROM "
		t_account_admin
		" WHERE uuid='%s'";
	sprintf(sqlbuf, sql, uuid);
	query_r = db_exec(sqlbuf, NULL);
	r = (query_r == e_query_done);
	assert(r);

	if (r)
	{
		bool r0;

		r0 = db_account_license_del(uuid);
		assert(r0);

		r0 = db_account_child_del(uuid, NULL);
		assert(r0);

		r0 = db_account_device_del(uuid, NULL);
		assert(0);
	}

	LOGI("[DB] delete admin '%s'.", uuid, r ? "ok" : "failed");

	return r;
}

bool db_account_child_add(const char * ref_admin_uuid, db_account_basic_t * account, char err[64])
{
	e_sql_query_result query_r;
	bool r;

	char sqlbuf[4096];
	const char * sql;
	char hashpwd[33];
	char lower_uuid[17];
	char lower_ref_admin_uuid[17];

	assert(ref_admin_uuid != NULL && account != NULL);

	assert(strlen(ref_admin_uuid) <= 16);
	strncpy(lower_ref_admin_uuid, ref_admin_uuid, sizeof(lower_ref_admin_uuid) - 1);
	world_tolower(lower_ref_admin_uuid);

	/* first check whether ref_admin_uuid is valid uuid */
	if (!db_account_check_user_exist(lower_ref_admin_uuid, NULL, NULL))
	{
		if (err)
		{
			strncpy(err, "admin uuid not valid", 63);
		}

		return false;
	}

	assert(strlen(account->uuid) <= 16);
	strncpy(lower_uuid, account->uuid, sizeof(lower_uuid) - 1);
	world_tolower(lower_uuid);

	assert(account != NULL);
	hashpwd[32] = 0;
	account_gen_hash_key(hashpwd, account->pwd, -1);

	sql = "INSERT INTO "
		t_account_basic
		" (uuid,pwd,nickname,comment) VALUES ('%s','%s','%s','%s')";
	sprintf(sqlbuf, sql, lower_uuid, hashpwd, account->nickname, account->comment);
	query_r = db_exec(sqlbuf, err);
	r = (query_r == e_query_done);
	if (!r)
	{
		return r;
	}

	sql = "INSERT INTO "
		t_account_child
		" (uuid,ref_admin_uuid) VALUES ('%s','%s')";
	sprintf(sqlbuf, sql, lower_uuid, lower_ref_admin_uuid);
	query_r = db_exec(sqlbuf, err);
	r = (query_r == e_query_done);

	LOGI("[DB] add child '%s' ref to admin '%s'.", lower_uuid, lower_ref_admin_uuid, r ? "ok" : "failed");

	return r;
}

bool db_account_child_del(const char * ref_admin_uuid, const char * specified_uuid)
{
	e_sql_query_result query_r;
	bool r;

	char sqlbuf[4096];
	const char * sql;

	assert(ref_admin_uuid != NULL);

	if (specified_uuid != NULL)
	{
		sql = "DELETE FROM "
			t_account_child
			" WHERE ref_admin_uuid='%s' AND uuid='%s'";
		sprintf(sqlbuf, sql, ref_admin_uuid, specified_uuid);
	}
	else
	{
		sql = "DELETE FROM "
			t_account_child
			" WHERE ref_admin_uuid='%s'";
		sprintf(sqlbuf, sql, ref_admin_uuid);
	}

	query_r = db_exec(sqlbuf, NULL);
	r = (query_r == e_query_done);
	assert(r);

	if (specified_uuid != NULL)
	{
		sql = "DELETE FROM "
			t_account_basic
			" WHERE uuid='%s'";
		sprintf(sqlbuf, sql, specified_uuid);
		query_r = db_exec(sqlbuf, NULL);
		r = (query_r == e_query_done);
		assert(r);
	}
	else
	{		
		sql = "DELETE FROM "
			t_account_basic
			" WHERE uuid IN (SELECT uuid FROM "
			t_account_admin
			" WHERE uuid='%s')";
		sprintf(sqlbuf, sql, ref_admin_uuid);
		query_r = db_exec(sqlbuf, NULL);
		r = (query_r == e_query_done);
		assert(r);
	}

	LOGI("[DB] delete child '%s' ref to admin '%s'.", specified_uuid != NULL ? specified_uuid : "<all>", ref_admin_uuid, r ? "ok" : "failed");

	return r;
}

bool db_account_check_user_exist(const char * uuid, bool * b_is_admin, /*out*/char ref_admin_uuid[16])
{
	e_sql_query_result query_r;
	bool r;

	char sqlbuf[4096];
	const char * sql;

	assert(uuid != NULL);

	if (b_is_admin != NULL) *b_is_admin = false;
	if (ref_admin_uuid != NULL) *ref_admin_uuid = 0;

	sql = "SELECT * FROM "
		t_account_basic
		" WHERE uuid='%s'";
	sprintf(sqlbuf, sql, uuid);

	query_r = db_exec(sqlbuf, NULL);
	r = (query_r == e_query_row);

	if (!r) return r;

	/* check current user whether have a admin license */
	if (b_is_admin != NULL)
	{
		db_account_license_t license;
		bool root_r;

		root_r = db_account_license_get(uuid, &license);
		if (root_r)
		{
			*b_is_admin = true;
			return r;
		}

		*b_is_admin = false;
	}

	if (ref_admin_uuid != NULL)
		/* account exist not be a admin so must be a child */
	{
		e_sql_query_result query_r;

		const char * query_sql;
		char sqlbuf[4096];
		sql_t sql;

		query_sql = "SELECT * FROM "
			t_account_child
			" WHERE uuid='%s'";
		sprintf(sqlbuf, query_sql, uuid);
		sql = sql_create_new(def_provider, NULL);
		if (sql == NULL)
		{
			assert(0);
			return r;
		}

		query_r = sql_query(sql, sqlbuf);
		assert(query_r != e_query_error);
		if (query_r != e_query_row)
		{
			sql_free(&sql);
			return r;
		}

		{
			sql_row_t * row;
			int fields;
			bool find_r;

			fields = sql_num_fields(sql);
			if (fields != 3)
			{
				assert(0);
				sql_free(&sql);
				return r;
			}

			find_r = false;
			while (!find_r && sql_fetch_row(sql, &row))
			{
				strncpy(ref_admin_uuid, row[1].data, 16);
				sql_fetch_row_free(sql, row);
				find_r = true;
				break;
			}

			sql_free(&sql);
		}
	}

	return r;
}

bool db_account_check_user_is_ok(const char * uuid, const char * pwd, bool * b_is_admin, /*out*/char ref_admin_uuid[16])
{
	e_sql_query_result query_r;
	bool r;

	char sqlbuf[4096];
	const char * sql;
	char hashpwd[33];

	assert(uuid != NULL && pwd != NULL);

	hashpwd[32] = 0;
	account_gen_hash_key(hashpwd, pwd, -1);

	sql = "SELECT * FROM "
		t_account_basic
		" WHERE uuid='%s' AND pwd='%s'";
	sprintf(sqlbuf, sql, uuid, hashpwd);

	query_r = db_exec(sqlbuf, NULL);
	r = (query_r == e_query_row);

	if (!r) return r;

	r = db_account_check_user_exist(uuid, b_is_admin, ref_admin_uuid);
	assert(r);

	return r;
}

bool db_account_update(const char * uuid, db_account_basic_t * new_account, char err[64])
{
	e_sql_query_result query_r;
	bool r;

	char sqlbuf[4096];
	const char * sql;
	char hashpwd[33];
	db_account_basic_t old_account;

	assert(uuid != NULL && new_account != NULL);
	assert(strcmp(uuid, new_account->uuid) == 0);

	/* only for log record */
	{
		char ref_admin_uuid[16 + 1];
		bool b_is_admin;
		bool r0;

		r0 = db_account_check_user_exist(uuid, &b_is_admin, ref_admin_uuid);
		if (!r0)
		{
			strcpy(err, "user not exist");
			LOGI("[DB] update user '%s' not exist.", uuid);
			return false;
		}

		{
			db_account_basic_t ** acc;
			int count;

			if (b_is_admin)
			{
				r0 = db_account_list_admin_get(uuid, &acc, &count);
				assert(r0);
			}
			else
			{
				r0 = db_account_list_child_get(ref_admin_uuid, uuid, &acc, &count);
				assert(r0);
			}

			if (!r0 || count != 1)
			{
				return false;
			}

			memcpy(&old_account, acc[0], sizeof(old_account));
			db_account_list_free(&acc, count);
		}
	}

	hashpwd[32] = 0;
	account_gen_hash_key(hashpwd, new_account->pwd, -1);

	sql = "UPDATE "
		t_account_basic
		" SET "
		"pwd='%s',nickname='%s',comment='%s'"
		" WHERE uuid='%s'";
	sprintf(sqlbuf, sql, hashpwd, new_account->nickname, new_account->comment, uuid);

	query_r = db_exec(sqlbuf, err);
	r = (query_r == e_query_done);

	if (!r) return r;
	assert(r);

	if (r)
	{
		LOGI("[DB] update account from ['%s','%s','%s','%s'] to ['%s','%s','%s','%s'].",
			old_account.uuid, old_account.pwd, old_account.nickname, old_account.comment,
			new_account->uuid, hashpwd, new_account->nickname, new_account->comment);
	}

	return r;
}

bool db_account_permission_set(const char * ref_admin_uuid, db_account_permission_t * conf)
{
	return false;
}

bool db_account_permission_get(const char * ref_admin_uuid, db_account_permission_t * conf)
{
	return false;
}

bool db_account_license_set(const char * ref_admin_uuid, db_account_license_t * license)
{
	e_sql_query_result query_r;
	bool r;

	const char * sql;
	char sqlbuf[4096];
	bool b_exist_setting;

	/* check if already exist */
	{
		db_account_license_t lic;

		b_exist_setting = db_account_license_get(ref_admin_uuid, &lic);
		if (b_exist_setting)
		{
			r = db_account_license_del(ref_admin_uuid);
			assert(r);
		}
	}

	sql = "INSERT INTO "
		t_account_admin
		" (uuid,license_type,licensed_time,expired_time) "
		"VALUES ('%s',%d,'%s','%s')";
	sprintf(sqlbuf, sql, 
		ref_admin_uuid,
		license->license_type,
		license->licensed_time,
		license->expired_time);

	query_r = db_exec(sqlbuf, NULL);
	r = (query_r == e_query_done);
	assert(r);

	return r;
}

bool db_account_license_get(const char * ref_admin_uuid, db_account_license_t * license)
{
	e_sql_query_result query_r;

	const char * query_sql;
	char sqlbuf[4096];
	sql_t sql;

	assert(ref_admin_uuid != NULL && license != NULL);

	query_sql = "SELECT * FROM "
		t_account_admin
		" WHERE uuid='%s'";

	memset(license, 0, sizeof(*license));
	sprintf(sqlbuf, query_sql, ref_admin_uuid);
	sql = sql_create_new(def_provider, NULL);
	if (sql == NULL)
	{
		assert(0);
		return false;
	}

	query_r = sql_query(sql, sqlbuf);
	assert(query_r != e_query_error);
	if (query_r != e_query_row)
	{
		sql_free(&sql);
		return false;
	}

	{
		sql_row_t * row;
		int fields;
		bool find_r;

		fields = sql_num_fields(sql);
		if (fields != 5)
		{
			assert(0);
			sql_free(&sql);
			return false;
		}

		find_r = false;
		while (!find_r && sql_fetch_row(sql, &row))
		{
			license->license_type = (e_account_license_type)atoi(row[1].data);
			strncpy(license->licensed_time, row[2].data, sizeof(license->licensed_time) - 1);
			strncpy(license->expired_time, row[3].data, sizeof(license->expired_time) - 1);
			sql_fetch_row_free(sql, row);
			find_r = true;
		}

		sql_free(&sql);

		return find_r;
	}

	return false;
}

bool db_account_license_del(const char * ref_admin_uuid)
{
	e_sql_query_result query_r;
	bool r;

	const char * sql;
	char sqlbuf[4096];

	if (!db_account_check_user_exist(ref_admin_uuid, NULL, NULL))
	{
		assert(0);
		return false;
	}

	sql = "DELETE FROM "
		t_account_admin
		" WHERE uuid='%s'";
	sprintf(sqlbuf, sql, 
		ref_admin_uuid);

	query_r = db_exec(sqlbuf, NULL);
	r = (query_r == e_query_done);
	assert(r);

	return r;
}

bool db_account_list_admin_get(const char * specified_uuid, db_account_basic_t *** account, int * count)
{
	e_sql_query_result query_r;
	sql_t sql;
	int ret_cnt;

	const char * query_sql;
	char sqlbuf[4096];

	if (account != NULL) *account = NULL;
	if (count != NULL) *count = 0;

	if (specified_uuid == NULL)
	{
		query_sql = "SELECT * FROM "
			t_account_basic
			" WHERE uuid IN (SELECT uuid FROM %s)";
		sprintf(sqlbuf, query_sql, t_account_admin);
	}
	else
	{
		query_sql = "SELECT * FROM "
			t_account_basic
			" WHERE uuid IN (SELECT uuid FROM %s WHERE uuid='%s')";
		sprintf(sqlbuf, query_sql, t_account_admin, specified_uuid);
	}

	sql = sql_create_new(def_provider, NULL);
	if (sql == NULL)
	{
		assert(0);
		return false;
	}

	query_r = sql_query(sql, sqlbuf);
	assert(query_r != e_query_error);
	if (query_r != e_query_row)
	{
		sql_free(&sql);
		return false;
	}

	ret_cnt = 0;
	while (sql_fetch_row(sql, NULL)) ret_cnt++;
	sql_free(&sql);

	if (count != NULL) *count = ret_cnt;
	if (ret_cnt == 0)
	{
		return false;
	}

	if (account != NULL)
	{
		db_account_basic_t ** acc;
		int index;
		sql_row_t * row;

		sql = sql_create_new(def_provider, NULL);
		if (sql == NULL)
		{
			assert(0);
			return false;
		}

		acc = (db_account_basic_t **)xcalloc(1, ret_cnt * sizeof(db_account_basic_t *));

		query_r = sql_query(sql, sqlbuf);
		assert(query_r != e_query_error);
		index = 0;
		while (sql_fetch_row(sql, &row))
		{
			db_account_basic_t * one;

			one = (db_account_basic_t *)xcalloc(1, sizeof(db_account_basic_t));
			acc[index] = one;

			strncpy(one->uuid, row[0].data, sizeof(one->uuid) - 1);
			strncpy(one->pwd, row[1].data, sizeof(one->pwd) - 1);
			strncpy(one->nickname, row[2].data, sizeof(one->nickname) - 1);
			strncpy(one->comment, row[3].data, sizeof(one->comment) - 1);
			strncpy(one->created_time, row[4].data, sizeof(one->created_time) - 1);

			sql_fetch_row_free(sql, row);
			index++;
		}

		*account = acc;
		sql_free(&sql);
	}

	return true;
}

bool db_account_list_child_get(const char * ref_admin_uuid, const char * specified_uuid, db_account_basic_t *** account, int * count)
{
	e_sql_query_result query_r;
	sql_t sql;
	int ret_cnt;

	const char * query_sql;
	char sqlbuf[4096];

	assert(ref_admin_uuid != NULL);

	if (account != NULL) *account = NULL;
	if (count != NULL) *count = 0;

	if (specified_uuid == NULL)
	{
		query_sql = "SELECT * FROM "
			t_account_basic
			" WHERE uuid IN (SELECT uuid FROM "
			t_account_child
			" WHERE ref_admin_uuid='%s')";
		sprintf(sqlbuf, query_sql, ref_admin_uuid);
	}
	else
	{
		query_sql = "SELECT * FROM "
			t_account_basic
			" WHERE uuid IN (SELECT uuid FROM "
			t_account_child
			" WHERE ref_admin_uuid='%s' AND uuid='%s')";
		sprintf(sqlbuf, query_sql, ref_admin_uuid, specified_uuid);
	}

	sql = sql_create_new(def_provider, NULL);
	if (sql == NULL)
	{
		assert(0);
		return false;
	}

	query_r = sql_query(sql, sqlbuf);
	assert(query_r != e_query_error);
	if (query_r != e_query_row)
	{
		sql_free(&sql);
		return false;
	}

	ret_cnt = 0;
	while (sql_fetch_row(sql, NULL)) ret_cnt++;
	sql_free(&sql);

	if (count != NULL) *count = ret_cnt;
	if (ret_cnt == 0)
	{
		return false;
	}

	if (account != NULL)
	{
		db_account_basic_t ** acc;
		int index;
		sql_row_t * row;

		sql = sql_create_new(def_provider, NULL);
		if (sql == NULL)
		{
			assert(0);
			return false;
		}

		acc = (db_account_basic_t **)xcalloc(1, ret_cnt * sizeof(db_account_basic_t *));
		query_r = sql_query(sql, sqlbuf);
		assert(query_r != e_query_error);
		index = 0;
		while (sql_fetch_row(sql, &row))
		{
			db_account_basic_t * one;

			one = (db_account_basic_t *)xcalloc(1, sizeof(db_account_basic_t));
			acc[index] = one;

			strncpy(one->uuid, row[0].data, sizeof(one->uuid) - 1);
			strncpy(one->pwd, row[1].data, sizeof(one->pwd) - 1);
			strncpy(one->nickname, row[2].data, sizeof(one->nickname) - 1);
			strncpy(one->comment, row[3].data, sizeof(one->comment) - 1);
			strncpy(one->created_time, row[4].data, sizeof(one->created_time) - 1);

			sql_fetch_row_free(sql, row);
			index++;
		}

		*account = acc;
		sql_free(&sql);
	}

	return true;
}

void db_account_list_free(db_account_basic_t *** account, int count)
{
	db_account_basic_t ** acc;
	int index;

	if (account == NULL || *account == NULL)
	{
		assert(0);
		return;
	}

	if (account != NULL) acc = *account;

	if (acc != NULL)
	{
		for (index = 0; index < count; index++)
		{
			xfree(acc[index]);
		}

		xfree(acc);
		*account = NULL;
	}
}

bool db_account_device_add(const char * ref_admin_uuid, db_account_device_t * device, char err[64])
{
	e_sql_query_result query_r;
	db_device_basic_t * basic;
	bool r;

	const char * sql;
	char sqlbuf[4096];

	/* check whether admin is valid */
	{
		bool b_is_admin;

		r = db_account_check_user_exist(ref_admin_uuid, &b_is_admin, NULL);
		if (!r || !b_is_admin)
		{
			if (err)
			{
				strcpy(err, "admin uuid not valid");
			}

			return false;
		}
	}

	{
		bool b_is_exsit;

		b_is_exsit = db_account_device_check_exist(ref_admin_uuid, device->basic.ip, device->basic.port, device->basic.port);
		if (b_is_exsit)
		{
			if (err)
			{
				strcpy(err, "device already exist");
			}

			return false;
		}
	}

	basic = &device->basic;
	sql = "INSERT INTO "
		t_device_basic
		" (uuid,plugin_name,ip,port,user,pwd,channel,seg_folder,seg_in_count,seg_per_sec) "
		"VALUES ('%s','%s','%s',%d,'%s','%s',%d,'%s',%d,%d)";
	sprintf(sqlbuf, sql, 
		basic->uuid,
		basic->plugin_name,
		basic->ip,
		basic->port,
		basic->user,
		basic->pwd,
		basic->channel,
		basic->seg_folder,
		basic->seg_in_count,
		basic->seg_per_sec);

	query_r = db_exec(sqlbuf, err);
	r = (query_r == e_query_done);
	if (!r)
	{
		return false;
	}

	sql = "INSERT INTO "
		t_account_device
		" (uuid,ref_admin_uuid,nickname,comment,is_top_level,ref_parent_uuid) "
		"VALUES ('%s','%s','%s','%s',%d,'%s')";
	sprintf(sqlbuf, sql, 
		device->basic.uuid,
		ref_admin_uuid,
		device->nickname,
		device->comment,
		device->is_top_level ? 1 : 0,
		device->ref_parent_uuid);

	query_r = db_exec(sqlbuf, err);
	r = (query_r == e_query_done);
	assert(r);

	return r;
}

bool db_account_device_del(const char * ref_admin_uuid, const char * spcified_uuid)
{
	e_sql_query_result query_r;
	bool r;

	const char * sql;
	char sqlbuf[4096];

	assert(ref_admin_uuid != NULL);

	if (spcified_uuid != NULL)
	{
		sql = "DELETE FROM "
			t_device_basic
			" WHERE uuid IN (SELECT uuid FROM "
			t_account_device
			" WHERE uuid='%s' AND ref_admin_uuid='%s')";
		sprintf(sqlbuf, sql, spcified_uuid, ref_admin_uuid);
		query_r = db_exec(sqlbuf, NULL);
		r = (query_r == e_query_done);
		assert(r);

		sql = "DELETE FROM "
			t_account_device
			" WHERE uuid='%s' AND ref_admin_uuid='%s'";
		sprintf(sqlbuf, sql, spcified_uuid, ref_admin_uuid);
		query_r = db_exec(sqlbuf, NULL);
		r = (query_r == e_query_done);
		assert(r);
	}
	else
	{
		sql = "DELETE FROM "
			t_device_basic
			" WHERE uuid IN (SELECT uuid FROM "
			t_account_device
			" WHERE ref_admin_uuid='%s')";
		sprintf(sqlbuf, sql, ref_admin_uuid);
		query_r = db_exec(sqlbuf, NULL);
		r = (query_r == e_query_done);
		assert(r);

		sql = "DELETE FROM "
			t_account_device
			" WHERE ref_admin_uuid='%s'";
		sprintf(sqlbuf, sql, ref_admin_uuid);
		query_r = db_exec(sqlbuf, NULL);
		r = (query_r == e_query_done);
		assert(r);
	}

	return r;
}

bool db_account_device_check_exist(const char * ref_admin_uuid, const char * ip, int port, int channel)
{
	e_sql_query_result query_r;
	bool r;

	const char * sql;
	char sqlbuf[4096];

	assert(ref_admin_uuid != NULL);

	sql = "SELECT * FROM "
		t_device_basic
		" WHERE ip='%s' AND port=%d AND channel=%d AND uuid IN "
		"(SELECT uuid FROM "
		t_account_device
		" WHERE ref_admin_uuid='%s')";

	sprintf(sqlbuf, sql, ip, port, channel, ref_admin_uuid);
	query_r = db_exec(sqlbuf, NULL);
	r = (query_r == e_query_row);

	return r;
}

bool db_account_device_workpath_valid(const char * workpath)
{
	e_sql_query_result query_r;
	bool r;

	const char * sql;
	char sqlbuf[4096];

	assert(workpath != NULL);

	sql = "SELECT * FROM "
		t_device_basic
		" WHERE seg_folder='%s'";

	sprintf(sqlbuf, sql, workpath);
	query_r = db_exec(sqlbuf, NULL);
	r = (query_r == e_query_row);

	return r;
}

bool db_account_device_get(const char * ref_admin_uuid, const char * spcified_uuid, db_account_device_t *** device, int * count)
{
	e_sql_query_result query_r;
	sql_t sql;
	int ret_cnt;

	const char * query_sql;
	db_account_device_t ** dev;
	char sqlbuf[1024];

	assert(ref_admin_uuid != NULL);
	if (device != NULL) *device = NULL;
	if (count != NULL) *count = 0;

	if (spcified_uuid == NULL)
	{
		query_sql = "SELECT "
			t_device_basic ".uuid,"
			t_device_basic ".plugin_name,"
			t_device_basic ".ip,"
			t_device_basic ".port,"
			t_device_basic ".user,"
			t_device_basic ".pwd,"
			t_device_basic ".channel,"
			t_device_basic ".seg_folder,"
			t_device_basic ".seg_in_count,"
			t_device_basic ".seg_per_sec,"
			t_device_basic ".create_time,"
			t_account_device ".nickname,"
			t_account_device ".comment,"
			t_account_device ".is_top_level,"
			t_account_device ".ref_parent_uuid"
			" FROM " t_device_basic
			" JOIN " t_account_device
			" ON "
			t_account_device ".uuid=="
			t_device_basic ".uuid"
			" WHERE "
			t_account_device
			".ref_admin_uuid='%s'";
		sprintf(sqlbuf, query_sql, ref_admin_uuid);
	}
	else
	{
		query_sql = "SELECT "
			t_device_basic ".uuid,"
			t_device_basic ".plugin_name,"
			t_device_basic ".ip,"
			t_device_basic ".port,"
			t_device_basic ".user,"
			t_device_basic ".pwd,"
			t_device_basic ".channel,"
			t_device_basic ".seg_folder,"
			t_device_basic ".seg_in_count,"
			t_device_basic ".seg_per_sec,"
			t_device_basic ".create_time,"
			t_account_device ".nickname,"
			t_account_device ".comment,"
			t_account_device ".is_top_level,"
			t_account_device ".ref_parent_uuid"
			" FROM " t_device_basic
			" JOIN " t_account_device
			" ON "
			t_account_device ".uuid=="
			t_device_basic ".uuid"
			" WHERE "
			t_account_device
			".ref_admin_uuid='%s' AND "
			t_account_device
			".uuid='%s'";
		sprintf(sqlbuf, query_sql, ref_admin_uuid, spcified_uuid);
	}
	ret_cnt = 0;

	sql = sql_create_new(def_provider, NULL);
	if (sql == NULL)
	{
		assert(0);
		return false;
	}

	query_r = sql_query(sql, sqlbuf);
	assert(query_r != e_query_error);
	if (query_r != e_query_row)
	{
		sql_free(&sql);
		return false;
	}

	while (sql_fetch_row(sql, NULL)) ret_cnt++;
	sql_free(&sql);

	if (ret_cnt == 0)
	{
		sql_free(&sql);
		return false;
	}

	if (count != NULL) *count = ret_cnt;

	if (device != NULL)
	{
		sql_row_t * row;
		int index;

		sql = sql_create_new(def_provider, NULL);
		if (sql == NULL)
		{
			assert(0);
			return false;
		}

		dev = (db_account_device_t **)xcalloc(1, ret_cnt * sizeof(db_account_device_t *));
		query_r = sql_query(sql, sqlbuf);
		assert(query_r != e_query_error);
		index = 0;
		while (sql_fetch_row(sql, &row))
		{
			db_account_device_t * one;

			one = (db_account_device_t *)xcalloc(1, sizeof(db_account_device_t));
			dev[index] = one;

			strncpy(one->basic.uuid, row[0].data, sizeof(one->basic.uuid) - 1);
			strncpy(one->basic.plugin_name, row[1].data, sizeof(one->basic.plugin_name) - 1);
			strncpy(one->basic.ip, row[2].data, sizeof(one->basic.ip) - 1);
			one->basic.port = atoi(row[3].data);
			strncpy(one->basic.user, row[4].data, sizeof(one->basic.user) - 1);
			strncpy(one->basic.pwd, row[5].data, sizeof(one->basic.pwd) - 1);
			one->basic.channel = atoi(row[6].data);
			strncpy(one->basic.seg_folder, row[7].data, sizeof(one->basic.seg_folder) - 1);
			one->basic.seg_in_count = atoi(row[8].data);
			one->basic.seg_per_sec = atoi(row[9].data);
			strncpy(one->basic.created_time, row[10].data, sizeof(one->basic.created_time) - 1);
			strncpy(one->nickname, row[11].data, sizeof(one->nickname) - 1);
			strncpy(one->comment, row[12].data, sizeof(one->comment) - 1);
			one->is_top_level = atoi(row[13].data);
			strncpy(one->ref_parent_uuid, row[14].data, sizeof(one->ref_parent_uuid) - 1);

			sql_fetch_row_free(sql, row);
			index++;
		}

		*device = dev;
		sql_free(&sql);
	}

	return true;
}

void db_account_device_free(db_account_device_t *** device, int count)
{
	db_account_device_t ** dev;
	int index;

	assert(device != NULL && *device != NULL);

	if (device != NULL) dev = *device;

	if (dev != NULL)
	{
		for (index = 0; index < count; index++)
		{
			xfree(dev[index]);
		}

		xfree(dev);
		*device = NULL;
	}
}

#ifdef _DEBUG

#include "guid.h"
void test_acount_all()
{
	bool r;

	db_account_ensure_table_exist();
//	db_account_reset();

	// add admin
	{
		db_account_basic_t account;
		strcpy(account.uuid, "admin_0");
		strcpy(account.pwd, "123");
		strcpy(account.nickname, "nick_0");
		strcpy(account.comment, "comment_0");

		r = db_account_admin_add(&account);

		{
			char ref_admin_uuid[17];
			bool is_admin;

			r = db_account_check_user_is_ok("admin_0", "123", &is_admin, ref_admin_uuid);
			assert(r);
		}

		{
			db_account_license_t license;

			r = db_account_license_get("admin_0", &license);
			db_account_license_set("admin_0", &license);
			r = db_account_license_get("admin_0", &license);
		}
	}
	// admin child
	{
		db_account_basic_t account;
		strcpy(account.uuid, "child_0");
		strcpy(account.pwd, "123");
		strcpy(account.nickname, "nick_0");
		strcpy(account.comment, "comment_0");

		r = db_account_child_add("admin_error", &account, NULL);
		r = db_account_child_add("admin_0", &account, NULL);
	}
	// list admin
	{
		db_account_basic_t ** basic;
		int count;
		bool r;

		r = db_account_list_admin_get(NULL, &basic, &count);
		if (r) db_account_list_free(&basic, count);

		r = db_account_list_admin_get("admin_0", &basic, &count);
		if (r) db_account_list_free(&basic, count);

		r = db_account_list_admin_get("admin_error", &basic, &count);
		if (r) db_account_list_free(&basic, count);

		r = db_account_list_child_get("admin_0", NULL, &basic, &count);
		if (r) db_account_list_free(&basic, count);

		r = db_account_list_child_get("admin_0", "child_0", &basic, &count);
		if (r) db_account_list_free(&basic, count);
	}

	{
		char first_conf_uuid[39];
		first_conf_uuid[38] = 0;

		// test add
		{
			db_account_device_t device;

			guid_generate((char *)device.basic.uuid);

			strcpy(first_conf_uuid, device.basic.uuid);

			strcpy(device.basic.plugin_name, "hb7000");
			strcpy(device.basic.ip, "192.168.1.110");
			device.basic.port = 8101;
			strcpy(device.basic.user, "admin");
			strcpy(device.basic.pwd, "888888");
			device.basic.channel = 0;
			strcpy(device.basic.seg_folder, "live_folder");
			device.basic.seg_in_count = 3;
			device.basic.seg_per_sec = 10;

			strcpy(device.comment, "my device comment");
			device.is_top_level = true;
			strcpy(device.nickname, "my device nick name");
			device.ref_parent_uuid[0] = 0;
			r = db_account_device_add("admin_error", &device, NULL);
			r = db_account_device_add("admin_0", &device, NULL);
		}
		// test add
		{
			db_account_device_t device;

			guid_generate((char *)device.basic.uuid);

			strcpy(first_conf_uuid, device.basic.uuid);

			strcpy(device.basic.plugin_name, "hb7001");
			strcpy(device.basic.ip, "192.168.1.111");
			device.basic.port = 8102;
			strcpy(device.basic.user, "admin1");
			strcpy(device.basic.pwd, "888888111");
			device.basic.channel = 0;
			strcpy(device.basic.seg_folder, "live_folder111");
			device.basic.seg_in_count = 3;
			device.basic.seg_per_sec = 10;

			strcpy(device.comment, "my device comment1111");
			device.is_top_level = true;
			strcpy(device.nickname, "my device nick name111");
			device.ref_parent_uuid[0] = 0;
			r = db_account_device_add("admin_0", &device, NULL);
		}

		// test get
		{
			db_account_device_t ** device;
			int count;

			r = db_account_device_get("admin_0", NULL, &device, &count);
			if (r) db_account_device_free(&device, count);
		}

		// remove
		db_account_device_del("admin_0", first_conf_uuid);
	}
}

#endif