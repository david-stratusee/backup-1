#ifndef _TYPES_H
#define _TYPES_H

#if defined(__KERNEL__)

#include "linux/stddef.h"
#include "linux/types.h"
#include "linux/kernel.h"

#else

#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <inttypes.h>
#include <stdbool.h>

#endif

#ifndef false
    #define bool    _Bool       /* still 8 bits wide */
    #define false   0
    #define true    1
#endif
#define BOOL_DESC(bvalue)   (bvalue ? "true" : "false")

#ifndef NULL
    #define NULL ((void*)0)
#endif

typedef struct _fix_str_size_t {
    char *fix_str;
    size_t str_len;    
} fix_str_size_t;   /* -- end of fix_str_size_t -- */
#define FIX_STR_SIZE_PAIR(fixstr)  {fixstr, sizeof(fixstr) - 1}
#define NULL_STR_SIZE_PAIR         {NULL, 0}

#define EMPTY_STATE \
    do {} while (0)

/* 1000 = 1024 - 16 - 8 */
#define PLUS1000(value)   \
    (((value) << 10) - ((value) << 4) - ((value) << 3))

#define PLUS16(value)    \
    ((value) << 4)

#define PLUS8(value)    \
    ((value) << 3)

/* 10 = 8 + 2 */
#define PLUS10(value)   \
    (PLUS8(value) + ((value) << 1))

#endif    /* _TYPES_H */
