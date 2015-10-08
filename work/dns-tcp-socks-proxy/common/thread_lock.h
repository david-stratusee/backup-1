/************************************************
 *       Filename: thread_lock.h
 *    Description: 
 *        Created: 2014-09-04 17:39
 *         Author: dengwei david@stratusee.com
 ************************************************/

#ifndef _THREAD_LOCK_H
#define _THREAD_LOCK_H

#include "common/cplus_define.h"
CPLUS_BEGIN

#include <pthread.h>
#include <string.h>

typedef struct {
    int node_locked;
    pthread_t lock_thread;
    pthread_mutex_t node_mutex;
} lock_t;

static inline void thread_lock_init(lock_t * h)
{
#ifdef _REENTRANT
    memset(h, 0, sizeof(lock_t));
    pthread_mutex_init(&(h->node_mutex), NULL);
#endif
}

static inline void thread_lock(lock_t * h)
{
#ifdef _REENTRANT
    pthread_t my_thread = pthread_self();

    if (!h->node_locked || h->lock_thread != my_thread) {
        if (likely(pthread_mutex_lock(&h->node_mutex) == 0)) {
            h->lock_thread = my_thread;
            h->node_locked = 1;
        }
    } else {
        (h->node_locked)++;
    }
#endif
}

static inline void thread_unlock(lock_t * h)
{
#ifdef _REENTRANT
    if (--(h->node_locked) == 0) {
        pthread_mutex_unlock(&h->node_mutex);
    }
#endif
}

CPLUS_END

#endif   /* -- #ifndef _THREAD_LOCK_H -- */
