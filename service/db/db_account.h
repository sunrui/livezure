/*
 * login database support
 *
 * update 2012-6-15 rui.sun
 */
#ifndef DB_ACCOUNT_H
#define DB_ACCOUNT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "db_account_declare.h"

void db_account_ensure_table_exist();
void db_account_reset();

bool db_account_admin_add(const db_account_basic_t * account);
bool db_account_admin_del(const char * uuid);
bool db_account_child_add(const char * ref_admin_uuid, db_account_basic_t * account, char err[64]);
bool db_account_child_del(const char * ref_admin_uuid, const char * specified_uuid);

bool db_account_check_user_exist(const char * uuid, bool * b_is_admin, /*out*/char ref_admin_uuid[16]);
bool db_account_check_user_is_ok(const char * uuid, const char * pwd, bool * b_is_admin, /*out*/char ref_admin_uuid[16]);

bool db_account_update(const char * uuid, db_account_basic_t * new_account, char err[64]);

bool db_account_permission_set(const char * ref_admin_uuid, db_account_permission_t * conf);
bool db_account_permission_get(const char * ref_admin_uuid, db_account_permission_t * conf);

bool db_account_license_set(const char * ref_admin_uuid, db_account_license_t * license);
bool db_account_license_get(const char * ref_admin_uuid, db_account_license_t * license);
bool db_account_license_del(const char * ref_admin_uuid);

/* list account functions */
bool db_account_list_admin_get(const char * specified_uuid, db_account_basic_t *** account, int * count);
bool db_account_list_child_get(const char * ref_admin_uuid, const char * specified_uuid, db_account_basic_t *** account, int * count);
void db_account_list_free(db_account_basic_t *** account, int count);

bool db_account_device_add(const char * ref_admin_uuid, db_account_device_t * device, char err[64]);
bool db_account_device_del(const char * ref_admin_uuid, const char * spcified_uuid);
bool db_account_device_check_exist(const char * ref_admin_uuid, const char * ip, int port, int channel);
bool db_account_device_workpath_valid(const char * workpath);
bool db_account_device_get(const char * ref_admin_uuid, const char * spcified_uuid, db_account_device_t *** device, int * count);
void db_account_device_free(db_account_device_t *** device, int count);

#ifdef __cplusplus
}
#endif

#endif