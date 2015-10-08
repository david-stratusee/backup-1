/************************************************
 *       Filename: lock_def.h
 *    Description: 
 *        Created: 2015-01-20 16:39
 *         Author: dengwei david@stratusee.com
 ************************************************/

#ifndef _LOCK_DEF_H
#define _LOCK_DEF_H

#define mutexlock_init(lock)    pthread_mutex_init(&(lock), NULL)
#define mutex_trylock(lock)     pthread_mutex_trylock(&(lock))
#define mutex_lock(lock)        pthread_mutex_lock(&(lock))
#define mutex_unlock(lock)      pthread_mutex_unlock(&(lock))
#define mutexlock_destroy(lock) pthread_mutex_destroy(&(lock))

#endif   /* -- #ifndef _LOCK_DEF_H -- */
