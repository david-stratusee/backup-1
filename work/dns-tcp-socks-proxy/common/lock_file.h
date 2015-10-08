/*
 * =====================================================================================
 *       Filename:  lock_file.h
 *    Description:  lock file interface
 *
 *        Version:  1.0
 *        Created:
 * =====================================================================================
 */
#ifndef _LOCK_FILE_H
#define _LOCK_FILE_H
#include "common/cplus_define.h"
CPLUS_BEGIN

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

static inline void _lock_init(struct flock *lock, int16_t type, int16_t whence, off_t start, off_t len)
{
    lock->l_type = type;
    lock->l_whence = whence;
    lock->l_start = start;
    lock->l_len = len;
}

#define INIT_LOCK(lock_arg, op_arg, start_arg)  \
    do {	\
        switch (op_arg) {	\
            case READ_LOCK:	\
                _lock_init(&lock_arg, F_RDLCK, SEEK_SET, start_arg, 1);	\
                break;	\
            case WRITE_LOCK:	\
                _lock_init(&lock_arg, F_WRLCK, SEEK_SET, start_arg, 1);	\
                break;	\
            default:	\
                return -1;	\
        }	\
    } while (0)


/********************************************
 * interface
 ********************************************/

typedef enum _FILE_LOCK_TYPE_EN {
    READ_LOCK,
    WRITE_LOCK,
    FLTE_MAX
} FILE_LOCK_TYPE_EN;   /* -- end of FILE_LOCK_TYPE_EN -- */
static inline int file_lock(int fd, FILE_LOCK_TYPE_EN op, off_t start)
{
    struct flock lock;
    INIT_LOCK(lock, op, start);

    if (fd < 0 || fcntl(fd, F_SETLKW, &lock) != 0) {
        return -1;
    }

    return 0;
}

static inline int file_trylock(int fd, FILE_LOCK_TYPE_EN op, off_t start)
{
    struct flock lock;
    INIT_LOCK(lock, op, start);

    if (fd < 0 || fcntl(fd, F_SETLK, &lock) != 0) {
        if (errno == EACCES || errno == EAGAIN) {
            return (0 - errno);
        } else {
            return -1;
        }
    }

    return 0;
}

static inline int file_unlock(int fd, off_t start)
{
    if (fd < 0) {
        return -1;
    }

    struct flock lock;
    _lock_init(&lock, F_UNLCK, SEEK_SET, start, 1);

    if (fcntl(fd, F_SETLKW, &lock) != 0) {
        return -1;
    }

    return 0;
}

static inline pid_t lock_test(int fd, int16_t type, int16_t whence, off_t start)
{
    struct flock lock;
    _lock_init(&lock, type, whence, start, 1);

    if (fcntl(fd, F_GETLK, &lock) == -1) {
        return -1;
    }

    if (lock.l_type == F_UNLCK) {
        return 0;
    }

    return lock.l_pid;
}

CPLUS_END
#endif    // _LOCK_FILE_H
