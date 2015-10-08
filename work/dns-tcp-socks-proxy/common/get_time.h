/************************************************
 *       Filename: get_time.h
 *    Description: 
 *
 *        Created:
 ************************************************/
#ifndef _GET_TIME_H
#define _GET_TIME_H

#if defined (__KERNEL__)
#include <linux/time.h>
extern struct timezone sys_tz;
#define get_utc_time() get_seconds()
#define get_local_time()    \
    (get_utc_time() + sys_tz.tz_minuteswest * 60)

#else

#include <time.h>
#define get_utc_time() time(NULL)

#include <sys/time.h>
/* 取本地时间(相对时间) */
static inline time_t get_local_time(void)
{
    struct timeval tv;
    struct timezone tz;

    if (gettimeofday(&tv, &tz) < 0) {
        return 0;
    }

    return (tv.tv_sec + tz.tz_minuteswest * 60);
}
#endif

#define get_cur_time() get_utc_time()

#endif    // _GET_TIME_H

