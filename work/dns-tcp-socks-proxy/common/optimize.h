
#ifndef _OPTIMIZE_H
#define _OPTIMIZE_H
#include "common/cplus_define.h"
CPLUS_BEGIN

#ifndef likely
    #define likely(x)    __builtin_expect((long)(!!(x)), (long)1)
    #define unlikely(x)    __builtin_expect((long)(!!(x)), (long)0)
#endif

#if defined(__KERNEL__)
#include <linux/kernel.h>
#else
#include <stdio.h>
#define assert_expr(expr)    \
    do {    \
        if (unlikely(!(expr))) {    \
            printf("[%s-%u](%s) is not correct, result is %u\n", __FILE__, __LINE__, #expr, expr);    \
        }    \
    } while (0)

#define assert_goto_label(expr, label)    \
    do {    \
        if (unlikely(!(expr))) {    \
            printf("[%s-%u](%s) is not correct, result is %u\n", __FILE__, __LINE__, #expr, expr);    \
            goto label;        \
        }    \
    } while (0)

#define assert_goto_err(expr)    \
    assert_goto_label(expr, err)

#endif 

CPLUS_END
#endif    // _OPTIMIZE_H

