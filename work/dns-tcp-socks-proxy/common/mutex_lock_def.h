/************************************************
 *       Filename: mutex_cmlib_lock.h
 *    Description: 
 *        Created: 2014-11-12 23:46
 *         Author: dengwei david@stratusee.com
 ************************************************/

#ifndef _MUTEX_LOCK_DEF_H
#define _MUTEX_LOCK_DEF_H

#include "common/types.h"

#define CONFIG_DEBUG_LOCK

/********************************************
 * Data Structure
 ********************************************/
#define FUNC_NAME_LEN 16
/*-------------------------------------------
   spin-lock
 -------------------------------------------*/
#define SPIN_LOCK_UNLOCKED 1
typedef struct raw_spinlock {
    volatile uint32_t slock;
#if defined(CONFIG_DEBUG_LOCK)
    uint32_t pid;
    uint16_t line_no;
    char func_name[FUNC_NAME_LEN - 2];
#elif __WORDSIZE == 64
    uint32_t resv_32;
#endif
} raw_spinlock_t;

/*-------------------------------------------
   rwlock
 -------------------------------------------*/
typedef struct {
    volatile uint32_t lock;
    volatile uint32_t read_req;

#if defined(CONFIG_DEBUG_LOCK)
    uint32_t pid;
    uint32_t line_no;
    char func_name[FUNC_NAME_LEN];
#endif
} raw_rwlock_t;
#define RW_LOCK_UNLOCKED 0x01000000UL
#define RW_LOCK_BIAS_STR "0x01000000"

#endif   /* -- #ifndef _MUTEX_LOCK_DEF_H -- */
