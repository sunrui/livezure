/*
 * day hours set provider
 * (c) livezure.com
 * 2013-2-26 17:26 rui.sun
 */
#include "day_hours_set.h"

#include <string.h>
#include <stdlib.h>

typedef struct day_hours
{
	char weekday[7 * 24];
} day_hours_t;

day_hours_t * day_hours_new(int b_set_all)
{
	day_hours_t * dh;

	dh = (day_hours_t *)malloc(sizeof(day_hours_t));
	memset(dh->weekday, b_set_all ? '1' : '0', sizeof(dh->weekday));

	return dh;
}

day_hours_t * day_hours_from(char szdh[7 * 24])
{
	day_hours_t * dh;

	dh = (day_hours_t *)malloc(sizeof(day_hours_t));
	memcpy(dh->weekday, szdh, sizeof(dh->weekday));

	return dh;
}

int day_hours_set(day_hours_t * dh, int weekday, int hour, int b_set)
{
	int hour_pos;

	if (dh == NULL ||
		weekday < 0 || weekday > 7 ||
		hour < 0 || weekday > 24)
	{
		return 0;
	}

	hour_pos = (weekday - 1) * 24 + hour;
	dh->weekday[hour_pos - 1] = b_set ? '1' : '0';

	return 1;
}

int day_hours_get(day_hours_t * dh, int weekday, int hour, int * b_set)
{
	int hour_pos;

	if (dh == NULL ||
		weekday < 0 || weekday > 7 ||
		hour < 0 || weekday > 24 ||
		b_set == NULL)
	{
		return 0;
	}

	hour_pos = (weekday - 1) * 24 + hour;
	*b_set = (dh->weekday[hour_pos - 1] == '1');

	return 1;
}

int day_hours_dump(day_hours_t * dh, char szdh[7 * 24])
{
	if (dh != NULL)
	{
		memcpy(szdh, dh->weekday, sizeof(dh->weekday));
		return 1;
	}

	return 0;
}

void day_hours_free(day_hours_t ** dh)
{
	if (dh != NULL && *dh != NULL)
	{
		free(*dh);
		*dh = NULL;
	}
}