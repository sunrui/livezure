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
����Ϊ linux ���ں� api ������ƽӿڡ�
������� api������û�� kref_destroy �����Ľӿڣ���ô�ͷ���Դ����Ī����Ϊ��ֻΪ�Լ�ʹ�ù������Բ�û�мӵģ���
���û������İɣ��� release ��ʱ����Դ����û��ص��������û����ʱ�����ɾ�������ˡ�
����û���Ҫ hang ������ȫ��δʹ��״̬������Ҫ��ô��ƣ�
void release(...)
{
	get_kref_inst(->set_sem?);
}

�ͷŵ�ʱ�� wait_sem?
------------------
Ϊʲô����Ƶ���һ�� release �ӿڣ�wait ������ʹ�ã�

�ѵ�˵���û��� put ��ʵ�����˳���ʱ���� 2012_12_31

����Ϊ����汾���û�������ʱ���������������� release ���� hang ���ܡ�
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