/*
 * memory pool provider
 *
 * @rui.sun 2013_1_2 17:38
 */
#ifndef XMALLOC_H
#define XMALLOC_H

/* whether use xmalloc */
#ifdef NDEBUG
#define USE_XMALLOC
#endif

#ifdef USE_XMALLOC
#ifdef __cplusplus
extern "C" {
#endif
void xmalloc_init();
void xmalloc_destroy();
void * xmalloc(int size);
void * xcalloc(int count, int size);
void xfree(void * ptr);
#ifdef __cplusplus
}
#endif
#else
#define xmalloc_init()
#define xmalloc_destroy()
#define xmalloc malloc
#define xcalloc calloc
#define xfree free
#endif

#endif