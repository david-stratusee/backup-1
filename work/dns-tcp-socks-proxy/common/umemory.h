/************************************************
 *       Filename: umemory.h
 *    Description: userspace memory define
 *
 *        Created:
 *         Author: dengwei
 ************************************************/
#ifndef _UMEMORY_H
#define _UMEMORY_H

#include <stdlib.h>
#include <string.h>
#ifdef USE_JEMALLOC
#include <jemalloc/jemalloc.h>
#endif

#include "common/types.h"
#include "common/optimize.h"

/********************************************
 * memory check here
 ********************************************/
#define PTR_NULL(ptr)       (unlikely((ptr) == NULL))
#define PTR_NOT_NULL(ptr)   (likely((ptr) != NULL))

/********************************************
 * FUNCTION
 ********************************************/
static inline void *umalloc(size_t size)
{
    return (size > 0 ? malloc((size_t)(size)) : NULL);
}
#define urealloc(ptr, new_size)  realloc((void *)(ptr), (size_t)(new_size))
#define ufree(ptr)     \
    do {               \
        if (ptr) {     \
            free(ptr); \
        }              \
    } while (0)

#define ufree_setnull(ptr) \
    do {                   \
        ufree(ptr);        \
        ptr = NULL;        \
    } while (0)

static inline void *ucalloc(size_t nmemb, size_t size)
{
    return ((nmemb > 0 && size > 0) ? calloc(nmemb, size) : NULL);
}

static inline void *__umalloc_empty(size_t size)
{
    void *ptr = umalloc(size);
    if (PTR_NOT_NULL(ptr)) {
        memset(ptr, 0, size);
    }

    return ptr;
}
#define umalloc_empty(alloc_size)   __umalloc_empty((size_t)(alloc_size))

#define umalloc_type(count, type)    \
    (type *)umalloc((count) * sizeof(type))
#define ucalloc_type(count, type)    \
    (type *)ucalloc((count), sizeof(type))

#endif    // _UMEMORY_H

