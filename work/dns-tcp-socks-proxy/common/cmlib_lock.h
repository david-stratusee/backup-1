/************************************************
 *       Filename: cmlib_lock.h
 *    Description: 
 *        Created: 2014-11-12 23:49
 *         Author: dengwei david@stratusee.com
 ************************************************/

#ifndef _CMLIB_LOCK_H
#define _CMLIB_LOCK_H

#include "common/lock_def.h"

#ifdef USE_THREAD_LOCK

#include <pthread.h>
static inline int _mutex_lock_init(pthread_mutex_t *lock)
{
    pthread_mutexattr_t lock_attr;
    pthread_mutexattr_init(&lock_attr);
    pthread_mutexattr_setpshared(&lock_attr, PTHREAD_PROCESS_SHARED);

    if (pthread_mutex_init(lock, &lock_attr) != 0) {
        pthread_mutexattr_destroy(&lock_attr);
        return -1;
    }

    pthread_mutexattr_destroy(&lock_attr);
    return 0;
}

static inline int _rwlock_init(pthread_rwlock_t *lock)
{
    pthread_rwlockattr_t lock_attr;
    pthread_rwlockattr_init(&lock_attr);
    pthread_rwlockattr_setpshared(&lock_attr, PTHREAD_PROCESS_SHARED);
    if (pthread_rwlock_init(lock, &lock_attr)) {
        pthread_rwlockattr_destroy(&lock_attr);
        return -1;
    }
    pthread_rwlockattr_destroy(&lock_attr);

    return 0;
}

#define mutexlock_init(lock)    _mutex_lock_init(&(lock))
#define mutex_trylock(lock)     pthread_mutex_trylock(&(lock))
#define mutex_lock(lock)        pthread_mutex_lock(&(lock))
#define mutex_unlock(lock)      pthread_mutex_unlock(&(lock))
#define mutexlock_destroy(lock) pthread_mutex_destroy(&(lock))

#define rwlock_init(lock)       _rwlock_init(&(lock))
#define rw_read_lock(lock)      pthread_rwlock_rdlock(&(lock))
#define rw_read_unlock(lock)    pthread_rwlock_unlock(&(lock))
#define rw_write_lock(lock)     pthread_rwlock_wrlock(&(lock))
#define rw_write_unlock(lock)   pthread_rwlock_unlock(&(lock))
#define rwlock_destroy(lock)    pthread_rwlock_destroy(&(lock))

#else

#include "mutex_lock.h"
#define mutexlock_init(lock)    _spinlock_init(&(lock))
#define mutex_trylock(lock)     _spinlock_trylock(&(lock))
#define mutex_lock(lock)        _spinlock_lock(&(lock))
#define mutex_unlock(lock)      _spinlock_unlock(&(lock))
#define mutexlock_destroy(lock) mutexlock_init(lock)

#define rwlock_init(lock)       _rw_lock_init(&(lock))
#define rw_read_lock(lock)      _rw_read_lock(&(lock))
#define rw_read_unlock(lock)    _rw_read_unlock(&(lock))
#define rw_write_lock(lock)     _rw_write_lock(&(lock))
#define rw_write_unlock(lock)   _rw_write_unlock(&(lock))
#define rwlock_destroy(lock)    rwlock_init(lock)

#endif

#endif   /* -- #ifndef _CMLIB_LOCK_H -- */

