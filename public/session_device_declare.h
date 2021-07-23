/*
 * device status declare
 * rui.sun 2012_1_9 15:01
 */
 #ifndef SESSION_DEVICE_DECLARE
 #define SESSION_DEVICE_DECLARE
 
typedef struct device_status
{
	int elapsed; /* time in minute */
	char fullpath[260]; /* full access hls address */
} device_status_t;

#endif