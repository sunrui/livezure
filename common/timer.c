/*
 * timer provider
 *
 * @author rui.sun Fri 2012-7-13 2:08
 */
#include "timer.h"
#include "posixinc.h"

#include <malloc.h>
#include <pthread.h>

struct st_timer
{
	pfn_timer_cb timer_cb; /* handle timeout callback here */
	void * userdata; /* timeout callback user data */
	int milliseconds; /* set timeout in milliseconds */
	int remaining; /* time remaining in last trigger time */
	int b_auto_delete; /* whether auto delete this when time triggered */
	int b_sign_as_deleted; /* sign this timer as deleted */
};

typedef struct timer_list
{
	timer_t timer;
	struct list_head list;
} timer_list_t;

timer_list_t timer_list; /* all sessions that registered to server */
int timer_list_inited = 0; /* whether sessions list inited */
int timer_stop_sign = 0; /* set to 1 if want to end loop */

pthread_t timer_thrd; /* timer thread handle */
pthread_cond_t timer_sync_cond; /* use for sleep exact time */
pthread_mutex_t timer_sync_mutex; /* use for sleep exact time */
struct timeval last_timeval; /* save the last timeval between every tick */
int timer_tickfps = 0; /* tick frequency every time */

/*
 * timer tick in millisesecond
 *
 * @return
 *     the last timeval
 */
struct timeval timer_tick(int millisesecond)
{
	struct timespec ts;
	struct timeval now;
	int ret_status;

	/* get the current time */
	gettimeofday(&now, NULL); /* add the offset to get timeout value */ 
	ts.tv_nsec = now.tv_usec * 1000 + (millisesecond % 1000) * 1000000; 
	ts.tv_sec = now.tv_sec + millisesecond / 1000;

	pthread_mutex_lock(&timer_sync_mutex);
	ret_status = pthread_cond_timedwait(&timer_sync_cond, &timer_sync_mutex, &ts);
	pthread_mutex_unlock(&timer_sync_mutex);

	return now;
}

/*
 * count timer sub in milliseconds
 */
int my_timer_sub(struct timeval * now, struct timeval * last)
{
	int subtv;

	if ((now->tv_usec -= last->tv_usec) < 0)
	{    
		--now->tv_sec;
		now->tv_usec += 1000000;
	}

	now->tv_sec -= last->tv_sec;
	subtv = now->tv_sec * 1000 + now->tv_usec / 1000;

	return subtv;
}

/*
 * timer synchronize
 *
 * @return
 *     0 stopped ok
 *    -1 received a abort message
 */
int timer_synchronize(int milliseconds)
{
	struct timeval now;
	int elapsed_sec;

	last_timeval = timer_tick(milliseconds);
	gettimeofday(&now, NULL);

	/* check how long times elapsed during last tick */
	elapsed_sec = my_timer_sub(&now, &last_timeval);

	/* synchronize all timer clock here */
	{
		struct list_head * pos, * q;
		timer_list_t * tmp;

		list_for_each_safe(pos, q, &timer_list.list)
		{
			tmp = list_entry(pos, timer_list_t, list);
			if (tmp->timer.b_sign_as_deleted || timer_stop_sign)
			{
				list_del(pos);
				free(tmp);
				continue;
			}

			if (tmp->timer.remaining != 0)
			{
				tmp->timer.remaining -= elapsed_sec;
				if (tmp->timer.remaining < 0)
				{
					tmp->timer.remaining = 0;
				}
			}
		}
	}

	if (timer_stop_sign)
	{
		assert(list_empty(&timer_list.list));
		return -1;
	}

	return 0;
}

/*
 * timer step tick in every milliseconds
 *
 * @return
 *     0 stopped ok
 *    -1 received a abort message
 */
int timer_step(int milliseconds)
{
	struct list_head * pos;
	timer_list_t * tmp;
	int ret;

	ret = timer_synchronize(milliseconds);

	/* if we received a abort message tell timer_clock_proc at once */
	if (ret == -1) return ret;

	/* check which timer cb is timeout */
	list_for_each(pos, &timer_list.list)
	{
		tmp = list_entry(pos, timer_list_t, list);
		if (tmp->timer.remaining == 0)
		{
			int b_abort_clock = 0;

			tmp->timer.timer_cb(tmp->timer.userdata, &b_abort_clock);
			if (b_abort_clock)
			{
				tmp->timer.b_sign_as_deleted = 1;
				continue;
			}

			/* recheck the time remaining again */
			tmp->timer.remaining = tmp->timer.milliseconds;
		}
	}

	return 0;
}

/*
 * start timer clock here, set timer_stop_sign 1 if you
 *  want to stop this timer process
 */
void * timer_clock_proc(void * param)
{
	/* we just need only exact in timer_tickfps ms per step */
	while (timer_step(timer_tickfps) != -1);

	return NULL;
}

/*
 * init timer environment, this function must be called
 *  before any timer_add can be used.
 *
 * @tickfps	tick frequency every time
 */
void timer_init(int tickfps)
{
	if (timer_list_inited)
	{
		return;
	}

	INIT_LIST_HEAD(&timer_list.list);

	timer_stop_sign = 0;
	timer_tickfps = tickfps;
	gettimeofday(&last_timeval, NULL);

	pthread_cond_init(&timer_sync_cond, NULL);
	pthread_mutex_init(&timer_sync_mutex, NULL);

	pthread_create(&timer_thrd, NULL, timer_clock_proc, NULL);

	timer_list_inited = 1;
}

/*
 * add a new timer to timer clock
 *
 * @time_cb			trigger function when timeout reached.
 *
 * @userdata		time_cb user data
 *
 * @milliseconds	set trigger timeout milliseconds
 *
 * @b_auto_delete	if wanted automate delete timer when triggered
 *
 * @return
 *    timer_t instance which can be use for timer_del
 */
timer_t * timer_create(pfn_timer_cb time_cb, void * userdata, int milliseconds, int b_auto_delete)
{
	timer_list_t * item;

	item = (timer_list_t *)malloc(sizeof(timer_list_t));
	item->timer.b_auto_delete = b_auto_delete;
	item->timer.milliseconds = milliseconds;
	item->timer.remaining = milliseconds;
	item->timer.timer_cb = time_cb;
	item->timer.userdata = userdata;
	item->timer.b_sign_as_deleted = 0;

	list_add_tail(&item->list, &timer_list.list);

	return &item->timer;
}

/*
 * delete an exist timer
 *
 * @return
 *     0 delete ok
 *    -1 invalid timer
 */
int timer_del(timer_t * timer)
{
	struct list_head * pos;
	timer_list_t * tmp;

	if (!timer_list_inited || timer == NULL)
	{
		assert(0);
		return -1;
	}

	list_for_each(pos, &timer_list.list)
	{
		tmp = list_entry(pos, timer_list_t, list);
		if (&tmp->timer == timer)
		{
			tmp->timer.b_sign_as_deleted = 1;
			return 0;
		}
	}

	return -1;
}

/*
 * destroy timer when more used, it will also delete
 *  all registered timers.
 */
void timer_destroy()
{
	if (!timer_list_inited)
	{
		return;
	}

	/* stop timer thread first */
	timer_stop_sign = 1;
	pthread_join(timer_thrd, NULL);
	assert(list_empty(&timer_list.list));

	timer_list_inited = 0;
}
