/*
 * timer provider
 *
 * @author rui.sun Fri 2012-7-13 2:08
 */
#ifndef TIMER_H
#define TIMER_H

/*
 * callback when timeout triggered, set b_abort_clock 1 if you
 *  want to abort this timer.
 */
typedef void (* pfn_timer_cb)(void * userdata, int * b_abort_clock);

/* declare timer handle */
typedef struct st_timer timer_t;

/*
 * init timer environment, this function must be called
 *  before any timer_add can be used.
 *
 * @tickfps	tick frequency every time
 */
void timer_init(int tickfps);

/*
 * add a new timer to timer clock
 *
 * @time_cb			trigger function when timeout reached.
 *
 * @userdata		time_cb user data
 * @milliseconds	set trigger timeout milliseconds
 * @b_auto_delete	if wanted automate delete timer when triggered
 *
 * @return
 *    timer_t instance which can be use for timer_del
 */
timer_t * timer_create(pfn_timer_cb time_cb, void * userdata, int milliseconds, int b_auto_delete);

/*
 * delete an exist timer
 *
 * @return
 *     0 delete ok
 *    -1 invalid timer
 */
int timer_del(timer_t * timer);

/*
 * destroy timer when more used, it will also delete
 *  all registered timers.
 */
void timer_destroy();

#endif
