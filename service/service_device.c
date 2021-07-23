/*
 * device device
 * rui.sun 2012_12_21 4:19
 */
#include "service_device.h"

#include "db_account.h"
#include "timer.h"
#include "pref.h"
#include "xmalloc.h"

#include "list.h"
#include "sock.h"
#include "log4c_extend.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct device_list
{
	char device_uuid[38 + 1];

	pref_t * ref;
	struct list_head list;
} device_list_t;

device_list_t device_list; /* all sessions that registered to server */
int device_list_inited = 0; /* whether sessions list inited */

pfn_device_host device_host;
void * device_opaque;

/*
 * device device init
 */
void session_device_init()
{
	if (device_list_inited)
	{
		return;
	}

	INIT_LIST_HEAD(&device_list.list);
	device_list_inited = 1;
}

/*
 * device device clean
 */
void session_device_destroy()
{
	struct list_head * pos, * q;
	device_list_t * tmp;

	if (!device_list_inited)
	{
		return;
	}

	list_for_each_safe(pos, q, &device_list.list)
	{
		tmp = list_entry(pos, device_list_t, list);
		list_del(pos);
		pref_release(&tmp->ref);
		xfree(tmp);
	}

	device_list_inited = 0;
}

/*
 * set device host callback
 */
void session_device_host(pfn_device_host host, void * opaque)
{
	assert(host != NULL && opaque != NULL);
	device_host = host;
	device_opaque = opaque;
}

/*
 * add a new device and try to work
 *
 * @return
 *    ref success start ok and returned current ref
 *     -1 plugin not support
 *     -2 could not created working folder
 *     -3 login to device failed.
 *     -4 play device failed
 *     -5 internal error[check segmenter_api.c]
 */
int session_device_ref_work(const char * uuid)
{
	device_list_t * tmp;

	assert(uuid != NULL);

	{
		struct list_head * pos;
		device_list_t * tmp;

		list_for_each(pos, &device_list.list)
		{
			tmp = list_entry(pos, device_list_t, list);
			/* if device already worked then just add ref */
			if (strcmp(tmp->device_uuid, uuid) == 0)
			{
				pref_get(tmp->ref);
				return pref_count(tmp->ref);
			}
		}
	}

	tmp = (device_list_t *)xcalloc(1, sizeof(device_list_t));
	strncpy(tmp->device_uuid, uuid, sizeof(tmp->device_uuid) - 1);
	pref_init(&tmp->ref);
	pref_get(tmp->ref);
	list_add_tail(&tmp->list, &device_list.list);

	LOGI("[DEVICE] add device work '%s'.\n", uuid);

	return true;
}

/*
 * stop current device and dref, device will be stop when no ref longer.
 *
 * @return
 *   ref return current ref
 *    -1 device uuid not valid
 */
int session_device_dref(const char * uuid)
{
	struct list_head * pos, * q;
	device_list_t * tmp;

	list_for_each_safe(pos, q, &device_list.list)
	{
		tmp = list_entry(pos, device_list_t, list);
		if (strcmp(tmp->device_uuid, uuid) == 0)
		{
			int c_ref;

			c_ref = pref_put(tmp->ref);

			if (c_ref == 0)
			{
				LOGI("[DEVICE] remove device work '%s'.\n", uuid);
				list_del(pos);
				pref_release(&tmp->ref);
				xfree(tmp);
			}

			return c_ref;
		}
	}

	assert(false);
	return -1;
}

/*
 * touch a device and check whether already work
 *
 * @status	just valid when in work status
 *
 * @return
 *   ref device is current work status
 *     0 device is not work
 *    -1 device not exist
 *    -2 device is starting
 */
int session_device_touch(const char * uuid, device_status_t * status)
{
	assert(status != NULL);

	memset(status, 0, sizeof(device_status_t));
	/* check uuid whether in work status */
	{
		struct list_head * pos, * q;
		device_list_t * tmp;

		list_for_each_safe(pos, q, &device_list.list)
		{
			tmp = list_entry(pos, device_list_t, list);
			if (strcmp(tmp->device_uuid, uuid) == 0)
			{
				/* device is current work status */
				return pref_count(tmp->ref);
			}
		}
	}

	/* check uuid whether in data base */
	{
		db_account_basic_t ** admin;
		int count;
		bool r;
		int loop;

		r = db_account_list_admin_get(NULL, &admin, &count);
		assert(r && count > 1);

		for (loop = 0; loop < count; loop++)
		{
			db_account_device_t ** device;
			int d_count;
			int d_loop;

			r = db_account_device_get(admin[loop]->uuid, NULL, &device, &d_count);
			assert(r);

			for (d_loop = 0; d_loop < d_count; d_loop++)
			{
				if (strcmp(device[0]->basic.uuid, uuid) == 0)
				{
					db_account_list_free(&admin, count);
					db_account_device_free(&device, d_count);
					/* device is not work */
					return 0;
				}
			}

			db_account_device_free(&device, d_count);
		}

		db_account_list_free(&admin, count);
	}

	assert(0);
	/* device not exist */
	return -1;
}

/*
 * get number of device count
 */
int session_device_count()
{
	struct list_head * pos;
	int cnt;

	cnt = 0;
	list_for_each(pos, &device_list.list) cnt++;

	return cnt;
}