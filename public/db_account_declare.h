/*
 * db account declare
 * update 2012-6-15 rui.sun
 */
#ifndef DB_ACCOUNT_DECLARE_H
#define DB_ACCOUNT_DECLARE_H

#include <stdbool.h>

typedef enum e_platfrom_id
{
	platfrom_pc_win            = 100,
	platfrom_android_phone     = 200,
	platfrom_android_pad       = 201,
	platfrom_ios_phone         = 300,
	platfrom_ios_pad           = 301
} e_platfrom_id;

#define valid_platform(pid) (pid == platfrom_pc_win || \
	pid == platfrom_android_phone || \
	pid == platfrom_android_pad || \
	pid == platfrom_ios_phone || \
	pid == platfrom_ios_pad)

#define valid_uuid(uuid) (uuid != NULL && strlen(uuid) <= 16)
#define valid_pwd(pwd) (pwd != NULL && strlen(pwd) <= 16)
#define valid_hashpwd(hashpwd) (hashpwd != NULL && strlen(hashpwd) <= 32)
#define valid_nickname(nickname) (nickname == NULL || (nickname != NULL && strlen(nickname) <= 64))
#define valid_comment(comment) (comment == NULL || comment != NULL && strlen(comment) <= 256)

typedef struct db_account_basic
{
	char uuid[16 + 1];
	char pwd[32 + 1]; /* hash pwd is 32 or user inputed is short than 16 */

	char nickname[64 + 1]; /* utf8-ed */
	char comment[256 + 1]; /* utf8-ed */

	char created_time[64 + 1]; /* ignore this when add */
} db_account_basic_t;

typedef enum e_account_license_type
{
	e_license_checking,
	e_license_time_user,
	e_license_forever
} e_account_license_type;

typedef struct db_account_license
{
	e_account_license_type license_type;
	char licensed_time[64 + 1];
	char expired_time[64 + 1];
} db_account_license_t;

typedef struct db_account_detail
{
	db_account_basic_t account;
	db_account_license_t license;
} db_account_detail;

typedef struct db_account_permission
{
	bool can_view_bridge_agent;
	bool can_view_record;
	bool can_ptz_ba;
	bool can_task_record;
} db_account_permission_t;

typedef struct db_device_basic
{
	char uuid[38 + 1]; /* unique id of device */
	char plugin_name[16 + 1]; /* registered plugin name */
	char ip[16 + 1]; /* device ip */
	int port; /* device port */
	char user[32 + 1]; /* device login user */
	char pwd[32 + 1]; /* device login pwd */
	int channel; /* open channel number */
	char seg_folder[32 + 1]; /* saved segmenter folder */
	int seg_in_count; /* how many segmenter files in one seg_file */
	int seg_per_sec; /* segmenter per-second */
	char created_time[64]; /* automate generate created time */
} db_device_basic_t;

typedef struct db_account_device
{
	db_device_basic_t basic;

	char nickname[64 + 1];
	char comment[256 + 1];

	bool is_top_level;
	char ref_parent_uuid[16 + 1];
} db_account_device_t;

#endif