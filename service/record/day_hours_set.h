/*
 * day hours set provider
 * (c) livezure.com
 * 2013-2-26 17:26 rui.sun
 */
#ifndef DAY_HOURS_SET_H
#define DAY_HOURS_SET_H

typedef struct day_hours day_hours_t;

#ifdef __cplusplus
extern "C" {
#endif

day_hours_t * day_hours_new(int b_set_all);
day_hours_t * day_hours_from(char szdh[7 * 24]);

int day_hours_set(day_hours_t * dh, int weekday, int hour, int b_set);
int day_hours_get(day_hours_t * dh, int weekday, int hour, int * b_set);

int day_hours_dump(day_hours_t * dh, char szdh[7 * 24]);

void day_hours_free(day_hours_t ** dh);

#ifdef __cplusplus
}
#endif

#endif