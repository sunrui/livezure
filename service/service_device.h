/*
 * device device
 * rui.sun 2012_12_21 4:19
 */
#ifndef SERVICE_DEVICE_H
#define SERVICE_DEVICE_H

#include "session_device_declare.h"

/*
 * device device init
 */
void session_device_init();

/*
 * device device clean
 */
void session_device_destroy();

/*
 * device host type
 */
typedef enum device_host_type
{
	device_stopped,
} device_host_type;

typedef void (* pfn_device_host)(void * opaque, device_host_type type, const char * uuid);

/*
 * set device host callback
 */
void session_device_host(pfn_device_host host, void * opaque);

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
int session_device_ref_work(const char * uuid);

/*
 * stop current device and dref, device will be stop when no ref longer.
 *
 * @return
 *   ref return current ref
 *    -1 device uuid not valid
 */
int session_device_dref(const char * uuid);

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
int session_device_touch(const char * uuid, device_status_t * status);

/*
 * number of device working current
 */
int session_device_count();

#endif