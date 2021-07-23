/*
 * instance ref manager
 *
 * @author rui.sun 2012_12_31 2:27
 */
#include "pref.h"

#include <pthread.h>
#include <assert.h>
#include <stdlib.h>

struct pref
{
	pthread_cond_t cond;
	pthread_mutex_t mutex;

	int refcount;
};

void pref_init(pref_t ** ref)
{
	pref_t * _ref;
	assert(ref != NULL);

	_ref = (pref_t *)calloc(1, sizeof(*_ref));
	pthread_mutex_init(&_ref->mutex, NULL);
	pthread_cond_init(&_ref->cond, NULL);
	_ref->refcount = 0;
	*ref = _ref;
}

int pref_get(pref_t * ref)
{
	pthread_mutex_lock(&ref->mutex);
	ref->refcount++;
	pthread_mutex_unlock(&ref->mutex);

	return ref->refcount;
}

int pref_put(pref_t * ref)
{
	pthread_mutex_lock(&ref->mutex);
	ref->refcount--;
	assert(ref->refcount >= 0);
	pthread_cond_signal(&ref->cond);
	pthread_mutex_unlock(&ref->mutex);

	return ref->refcount;
}

/**
 * pref_count - check how many reference current.
 * @pref_t: object.
 */
int pref_count(pref_t * ref)
{
	int count;
	pthread_mutex_lock(&ref->mutex);
	count = ref->refcount;
	pthread_mutex_unlock(&ref->mutex);

	return count;
}

/*
 * pref_try_release - try to release if no reference current.
 */
int pref_try_release(pref_t ** _ref)
{
	pref_t * ref;

	if (_ref == NULL || *_ref == NULL)
	{
		assert(0 && "pref_try_release ref is NULL.");
		return 0;
	}

	ref = *_ref;
	pthread_mutex_lock(&ref->mutex);
	if (ref->refcount == 0)
	{
		pthread_mutex_unlock(&ref->mutex);
		pthread_cond_destroy(&ref->cond);
		pthread_mutex_destroy(&ref->mutex);
		free(ref);
		*_ref = NULL;
		return 1;
	}
	pthread_mutex_unlock(&ref->mutex);
	
	return 0;
}

/* 0 release, return count if have ref instance */
void pref_release(pref_t ** _ref)
{	
	pref_t * ref;

	if (_ref == NULL || *_ref == NULL)
	{
		assert(0 && "pref_release ref is NULL.");
		return;
	}
	ref = *_ref;

	while (ref->refcount != 0)
	{
		pthread_mutex_lock(&ref->mutex);
		pthread_cond_wait(&ref->cond, &ref->mutex);
		pthread_mutex_unlock(&ref->mutex);
	}

	pthread_cond_destroy(&ref->cond);
	pthread_mutex_destroy(&ref->mutex);
	free(ref);
	*_ref = NULL;
}

#ifdef TEST_PREF
void test_pref()
{
	pref_t * ref;

	pref_init(&ref);
	pref_get(ref);
	pref_try_release(&ref);
	pref_put(ref);
	pref_release(&ref);
}
#endif