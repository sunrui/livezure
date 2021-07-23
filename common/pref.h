/*
 * instance ref manager
 *
 * @author rui.sun 2012_12_31 2:27
 */
#ifndef PREF_H
#define PREF_H

#if 0
/**
 * kref_set - initialize object and set refcount to requested number.
 * @kref: object in question.
 * @num: initial reference counter
 */
void  kref_set(struct kref *kref, int num);
/**
 * kref_init - initialize object.
 * @kref: object in question.
 */
void  kref_init(struct kref *kref);
/**
 * kref_get - increment refcount for object.
 * @kref: object.
 */
void kref_get(struct kref *kref);
/**
 * kref_put - decrement refcount for object.
 * @kref: object.
 * @release: pointer to the function that will clean up the object when the
 *      last reference to the object is released.
 *      This pointer is required, and it is not acceptable to pass kfree
 *      in as this function.
 *
 * Decrement the refcount, and if 0, call release().
 * Return 1 if the object was removed, otherwise return 0.  Beware, if this
 * function returns 0, you still can not count on the kref from remaining in
 * memory.  Only use the return value if you want to see if the kref is now
 * gone, not present.
 */
int  kref_put(struct kref *kref, void (*release) (struct kref *kref));
/*
以上为 linux 的内核 api 记数设计接口。
这种设计 api，首先没有 kref_destroy 这样的接口，怎么释放资源？（莫非因为它只为自己使用管理，所以才没有加的？）
如果没有理解错的吧，在 release 的时候可以触发用户回调，告诉用户这个时候可以删除对象了。
如果用户需要 hang 至对象全部未使用状态，岂不是要这么设计？
void release(...)
{
	get_kref_inst(->set_sem?);
}

释放的时候 wait_sem?
------------------
为什么不设计单独一个 release 接口，wait 至无人使用？

难道说，用户的 put 都实现在退出的时候吗？ 2012_12_31

以下为改造版本，用户可以随时消减计数器，并且 release 带有 hang 功能。
 */
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pref pref_t;

/**
 * pref_init - initialize object.
 * @pref_t: object in question.
 */
void pref_init(pref_t ** ref);

/**
 * pref_get - increment refcount for object.
 * @pref_t: object.
 */
int pref_get(pref_t * ref);

/**
 * pref_put - decrement refcount for object.
 * @pref_t: object.
 */
int pref_put(pref_t * ref);

/**
 * pref_count - check how many reference current.
 * @pref_t: object.
 */
int pref_count(pref_t * ref);

/*
 * pref_try_release - try to release if no reference current.
 *
 * return 1 if release ok or 0 instance still busy.
 */
int pref_try_release(pref_t ** ref);

/*
 * pref_release - handle and release object.
 */
void pref_release(pref_t ** ref);

/* may be you need pref_force_release or pref_force_release_timewait? 
    what an ugly code you do!!! */

#ifdef __cplusplus
}
#endif

#endif