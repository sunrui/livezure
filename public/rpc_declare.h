/*
 * public protocol definition
 *
 * @author rui.sun 2012-6-28 23:33
 */
#ifndef RPC_DECLARE_H
#define RPC_DECLARE_H

/* declare current version environments */
#define rpc_id_encrypt                crypt_type_none

#define rpc_id_platform               0
#define rpc_id_version_check          0
#define rpc_id_heartbeat              1

#define rpc_id_push                   1
#define rpc_id_push_update            0

#define rpc_id_account                2
#define rpc_id_account_login          0
#define rpc_id_account_logout         1
#define rpc_id_account_create         2
#define rpc_id_account_modify         3
#define rpc_id_account_delete         4
#define rpc_id_account_list           5

#define rpc_id_device                 3
#define rpc_id_device_create          0
#define rpc_id_device_modify          1
#define rpc_id_device_delete          2
#define rpc_id_device_list            3

#define rpc_id_user_storage           4
#define rpc_id_user_storage_create    0
#define rpc_id_user_storage_modify    1
#define rpc_id_user_storage_delete    2
#define rpc_id_user_storage_list      3

#define rpc_id_play                   5
#define rpc_id_play_device            0
#define rpc_id_play_stop              1
#define rpc_id_play_touch             2

#define rpc_id_record                 6
#define rpc_id_record_play            0
#define rpc_id_record_stop            1

#endif