/*
 * guid provider
 * @2012-12-5 rui.sun
 */
#include "guid.h"

#include <stdio.h>

#ifdef WIN32
#include <objbase.h>
#else
#include <uuid/uuid.h>

typedef struct _GUID
{
	unsigned long Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char Data4[8];
} GUID, UUID;
#endif

void guid_generate(char guid[38])
{
	GUID uuid;
#ifdef WIN32
	CoCreateGuid(&uuid);
#else
	uuid_generate((unsigned char *)(&uuid));
#endif

	sprintf(
		guid,
		"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		uuid.Data1, uuid.Data2, uuid.Data3,
		uuid.Data4[0], uuid.Data4[1],
		uuid.Data4[2], uuid.Data4[3],
		uuid.Data4[4], uuid.Data4[5],
		uuid.Data4[6], uuid.Data4[7]);
}