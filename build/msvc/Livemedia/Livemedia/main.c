#include "media_api.h"

#include "log4c_extend.h"

#include <winsock.h>
#include <crtdbg.h>

#include "vld.h"
#include <stdio.h>
#include <assert.h>
#include "xmalloc.h"
#include "posixtime.h"
#include <tchar.h>
#include "encrypt.h"


typedef struct mc_status_lang
{
	mc_status status;
	char * string;
} mc_status_lang_t;

mc_status_lang_t status_lang_en[] =
{
	{ mc_ok, "ok" },
	{ mc_server_not_response, "mc_server_not_response" },
	{ mc_syntax_error, "mc_syntax_error" },
	{ mc_internal_error, "mc_internal_error" },
	{ mc_no_permission, "mc_no_permission" },
	{ mc_session_expired, "mc_session_expired" },
	{ mc_invalid_uuid_or_pwd, "mc_invalid_uuid_or_pwd" },
	{ mc_old_pwd_not_correct, "mc_old_pwd_not_correct" },
	{ mc_uuid_exist, "mc_uuid_exist" },
	{ mc_delete_cannot_be_owner, "mc_delete_cannot_be_root" },
	{ mc_uuid_not_exist, "mc_uuid_not_exist" },
};

/*
 * media module status to string, default language: us-en
 *
 * @return
 *     utf8-ed status string or NULL if no exist
 */
//const char * mc_status_to_string(mc_status status, const char * lang);

const char * mc_status_to_string(mc_status status)
{
	int index;

	/* default language: us-en */
	for (index = 0; index < _countof(status_lang_en); index++)
	{
		if (status_lang_en[index].status == status)
		{
			return status_lang_en[index].string;
		}
	}

	return "unknown";
}

void test_platform()
{
	mc_status r;

	r = mc_connect_to_server("127.0.0.1", 13404, 0);
	
	{
		version_status_t ver;

		r = mc_check_version(platfrom_pc_win, &ver);
		
	}

	mc_disconnect();
}

void test_account()
{
	mc_session_t * session;
	mc_status r;

	r = mc_connect_to_server("127.0.0.1", 13404, 0);
	assert(r == mc_ok);

	// ����һ�������ڵ� uuid ��½
	r = mc_account_login("admin", "admin", platfrom_pc_win, &session);
	assert(r == mc_invalid_uuid_or_pwd);
	

	// ����һ�����ڵ� uuid ��½
	{
		r = mc_account_login("root", "root", platfrom_pc_win, &session);
		assert(r == mc_ok);
		
		r = mc_account_logout(&session);
		assert(r == mc_ok);
	}

	// ������Ӳ���
	{
		r = mc_account_login("root", "root", platfrom_pc_win, &session);
		assert(r == mc_ok);
		
		// ����û�
		{
			db_account_basic_t basic;

			strcpy(basic.uuid, "root");
			strcpy(basic.pwd, "root");
			strcpy(basic.nickname, "nickname");
			strcpy(basic.comment, "comment");

			/* �û��Ѿ������� */
			r = mc_account_create(session, &basic);
			assert(r == mc_uuid_exist);
			
			/* �û���ӳɹ� */
			strcpy(basic.uuid, "refroot_child0");
			r = mc_account_create(session, &basic);
			assert(r == mc_ok);
		}

		r = mc_account_logout(&session);
		assert(r == mc_ok);
	}

	// ���Էǹ���Ա�޸�
	{
		r = mc_account_login("refroot_child0", "root", platfrom_pc_win, &session);
		assert(r == mc_ok);

		{
			db_account_basic_t basic;
			strcpy(basic.uuid, "refroot_child0");
			strcpy(basic.pwd, "1234");
			strcpy(basic.nickname, "child_nick");
			strcpy(basic.comment, "child_comment");

			// ���������
			r = mc_account_modify(session, "root_err", &basic);
			assert(r == mc_old_pwd_not_correct);

			// ���³ɹ�
			r = mc_account_modify(session, "root", &basic);
			assert(r == mc_ok);
		}

		r = mc_account_logout(&session);
		assert(r == mc_ok);
	}

	// ����ʹ���¾������½
	{
		// ��½�ɹ�
		r = mc_account_login("refroot_child0", "1234", platfrom_pc_win, &session);
		assert(r == mc_ok);

		// �˳��ɹ�
		r = mc_account_logout(&session);
		assert(r == mc_ok);

		// ��½ʧ��
		r = mc_account_login("refroot_child0", "root", platfrom_pc_win, &session);
		assert(r == mc_invalid_uuid_or_pwd);
	}

	// ���Էǹ���Աɾ��
	{
		r = mc_account_login("refroot_child0", "1234", platfrom_pc_win, &session);
		assert(r == mc_ok);

		// ɾ���û�
		{
			/* ֱ���ڱ��ؾ��ж��ˣ���û��Ȩ�޵ġ� */
			r = mc_account_delete(session, "root_pwd_err", "root");
			assert(r == mc_no_permission);

			/* �û�û��Ȩ�޵� */
			r = mc_account_delete(session, "1234", "root");
			assert(r == mc_no_permission);
		}

		r = mc_account_logout(&session);
		assert(r == mc_ok);
	}

	// ���Թ���Աɾ��
	{
		r = mc_account_login("root", "root", platfrom_pc_win, &session);
		assert(r == mc_ok);

		// ɾ���û�
		{
			/* ��������� */
			r = mc_account_delete(session, "root_pwd_err", "root");
			assert(r == mc_old_pwd_not_correct);

			/* ����ɾ���û��Լ�������Ա�� */
			r = mc_account_delete(session, "root", "root");
			assert(r == mc_delete_cannot_be_owner);

			/* �û������ڣ���Ϊ��ʹ�û������ݿ⣬ҲҪ�ж� ref �� admin �� */
			r = mc_account_delete(session, "root", "root0");
			assert(r == mc_uuid_not_exist);


			/* ɾ���ɹ� */
			r = mc_account_delete(session, "root", "refroot_child0");
			assert(r == mc_ok);
		}

		r = mc_account_logout(&session);
		assert(r == mc_ok);
	}

	// �����г��������û�
	{
		r = mc_account_login("root", "root", platfrom_pc_win, &session);
		assert(r == mc_ok);

		{
			db_account_basic_t basic;
			int index;

			for (index = 0; index < 2; index++)
			{
				sprintf(basic.uuid, "child_%d", index);
				strcpy(basic.pwd, "1234");
				sprintf(basic.nickname, "child_nick_%d", index);
				sprintf(basic.comment, "child_comment_%d", index);

				r = mc_account_create(session, &basic);
				assert(r == mc_ok);
			}

			// list
			{
				online_session_t ** online;
				int count;

				r = mc_account_list(session, &online, &count);
				assert(r == mc_ok);

				r = mc_account_list_free(online, count);
				assert(r == mc_ok);
			}
		}

		r = mc_account_logout(&session);
		assert(r == mc_ok);
	}
	
	//// �����������
	//{
	//	r = mc_account_login("root", "root", platfrom_pc_win, &session);
	//	assert_r(r);
	//}

	mc_disconnect();

	// ���� device ���
	{
		mc_session_t * session;
		mc_status r;
		db_account_device_t device;

		r = mc_connect_to_server("127.0.0.1", 13404, 0);
		assert(r == mc_ok);

		memset(&device, 0, sizeof(db_account_device_t));
		strcpy(device.basic.plugin_name, "hb7000");
		strcpy(device.basic.ip, "192.168.1.110");
		device.basic.port = 3306;
		strcpy(device.basic.user, "admin");
		strcpy(device.basic.pwd, "888888");
		device.basic.channel = 0;
		strcpy(device.basic.seg_folder, "folder0");
		device.basic.seg_in_count = 3;
		device.basic.seg_per_sec = 10;

		strcpy(device.nickname, "my_device_nickname");
		strcpy(device.comment, "my_device_comment");
		device.is_top_level = 1;

		// ���Էǹ���Ա���
		{
			char uuid[39];

			// ��½�ɹ�
			r = mc_account_login("child_0", "1234", platfrom_pc_win, &session);
			assert(r == mc_ok);

			// ���û��Ȩ��
			r = mc_device_add(session, &device, uuid);
			assert(r == mc_no_permission);

			// ɾ������
			uuid[0] = 'h';
			uuid[1] = '\0';

			r = mc_device_del(session, uuid);
			assert(r == mc_no_permission);

			r = mc_account_logout(&session);
			assert(r == mc_ok);
		}

		// ���Թ���Ա���
		{
			char uuid[39];
			int try_time;

			for (try_time = 0; try_time < 2; try_time++)
			{
				// ��½�ɹ�
				r = mc_account_login("root", "root", platfrom_pc_win, &session);
				assert(r == mc_ok);

				r = mc_device_add(session, &device, uuid);
				//assert(r == mc_ok);

				// ɾ������
				//r = mc_device_del(session, uuid);
				//assert(r == mc_ok);

				r = mc_account_logout(&session);
				assert(r == mc_ok);
			}
		}
	}

	mc_disconnect();

	LOGD("test account finished.");
}

void test_operate()
{
	// �����б�
	db_account_device_t ** dev;
	mc_session_t * session;
	int cnt;
	const char * uuid;
	mc_status r;

	r = mc_connect_to_server("127.0.0.1", 13404, 0);
	assert(r == mc_ok);

	// ��½�ɹ�
	r = mc_account_login("root", "root", platfrom_pc_win, &session);
	assert(r == mc_ok);

	r = mc_device_list(session, &dev, &cnt);
	assert(r == mc_ok);

	// ���� play ָ���� device uuid
	uuid = dev[0]->basic.uuid;
	{
		int rid;
		int i;

		r = mc_play_device(session, uuid, &rid);

		for (i = 0; i < 2; i++)
		{
			device_status_t sta;

			r = mc_play_touch(session, uuid, &rid, &sta);
			usleep(2000);
		}

		r = mc_play_stop(session, uuid, &rid);
	}

	r = mc_device_list_free(dev, cnt);
	assert(r == mc_ok);

	r = mc_account_logout(&session);
	assert(r == mc_ok);

	mc_disconnect();
}


int main()
{
	xmalloc_init();
	logc_init(NULL, NULL);

	{
		int i = 0;
		for (i; i < 500; i++)
		{
			version_status_t ver;

 			mc_connect_to_server("127.0.0.1", 13404, 0);

			mc_check_version(platfrom_pc_win, &ver);

			LOGD("get version ok.");


			mc_disconnect();
		}
	}


	test_platform();
	test_account();
	test_operate();


	logc_fini();
	xmalloc_destroy();

	return 0;
}
