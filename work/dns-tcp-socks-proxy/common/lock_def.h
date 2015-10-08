/************************************************
 *       Filename: lock_def.h
 *    Description: lock typedef
 *
 *        Created:
 *         Author: dengwei
 ************************************************/
#ifndef _LOCK_DEF_H
#define _LOCK_DEF_H

#ifdef USE_THREAD_LOCK

#include <pthread.h>
typedef pthread_mutex_t         mutexlock_t;
typedef pthread_rwlock_t        rwlock_t;
#else

#include "common/mutex_lock_def.h"
typedef raw_spinlock_t          mutexlock_t;
typedef raw_rwlock_t            rwlock_t;
#endif

#endif    // _LOCK_DEF_H

