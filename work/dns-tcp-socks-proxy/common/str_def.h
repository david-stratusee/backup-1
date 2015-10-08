/************************************************
 *       Filename: str_def.h
 *    Description: string include platless define
 *
 *        Created:
 *         Author: dengwei
 ************************************************/
#ifndef _STR_DEF_H
#define _STR_DEF_H

/********************************************
 * string
 ********************************************/
#if defined(__KERNEL__)
#include <linux/string.h>
#include <linux/kernel.h>

#ifndef strncasecmp
    #define strncasecmp strnicmp
#endif

#ifndef strcasecmp
    #define strcasecmp(s1, s2)    \
        ((strlen(s1) == strlen(s2)) && (strncasecmp(s1, s2, strlen(s1) == 0)))
#endif

#ifndef strtol
    #define strtol     simple_strtol
    #define strtoul    simple_strtoul
    #define strtoll    simple_strtoll
    #define strtoull   simple_strtoull    
#endif

#else
    #include <string.h>
    #include <stdlib.h>
    #include <strings.h>
#endif

/********************************************
 * ctype
 ********************************************/
#if defined(__KERNEL__)
    #include <linux/ctype.h>
#else
    #ifndef _LINUX_CTYPE_H
        #include <ctype.h>
    #endif
#endif

/********************************************
 * sprintf
 ********************************************/
#if defined(__KERNEL__)
    #include <linux/kernel.h>
#else
    #include <stdio.h>
#endif

#include "common/types.h"
#include "common/optimize.h"

#define usize(str) ((uint32_t)sizeof(str))

#define FLEN(fstr) (sizeof(fstr) - 1)

#define case_strequal(buf, len, fix_str, fix_strlen) \
    ((size_t)(len) == (size_t)(fix_strlen) && strncasecmp((char *)(buf), (char *)(fix_str), (size_t)(fix_strlen)) == 0)

#define strequal(buf, len, fix_str, fix_strlen) \
    ((size_t)(len) == (size_t)(fix_strlen) && strncmp((char *)(buf), (char *)(fix_str), (size_t)(fix_strlen)) == 0)

#define fix_case_strequal(buf, len, fix_str) \
    case_strequal(buf, len, fix_str, FLEN(fix_str))

#define fix_strequal(buf, len, fix_str) \
    strequal(buf, len, fix_str, FLEN(fix_str))

#define fix_prefix(buf, len, fix_str) \
    ((size_t)(len) >= (size_t)(FLEN(fix_str)) && strncasecmp((const char *)(buf), (const char *)(fix_str), FLEN(fix_str)) == 0)

#define append_fmtmsg(buffer, buf_maxlen, buf_len, format, args...)                                      \
    do {                                                                                                 \
        if (likely((buf_len) < (buf_maxlen))) {                                                          \
            buf_len += snprintf((char *)(buffer) + (buf_len), (buf_maxlen) - (buf_len), format, ##args); \
        }                                                                                                \
    } while (0)

#define append_strmsg(buffer, buf_maxlen, buf_len, appendstr, str_len)                   \
    do {                                                                                 \
        if (likely((size_t)(buf_len) + (size_t)(str_len) < (size_t)(buf_maxlen))) {      \
            strncpy((buffer) + (buf_len), (const char *)(appendstr), (size_t)(str_len)); \
            (buf_len) += (str_len);                                                      \
            (buffer)[buf_len] = '\0';                                                    \
        }                                                                                \
    } while (0)

#define append_fixstr(buffer, buf_maxlen, buf_len, fixstr) \
    append_strmsg(buffer, buf_maxlen, buf_len, fixstr, FLEN(fixstr))

#define str_append_fmtmsg(buffer, buf_maxlen, buf_len, format, args...) \
    do {                                                                \
        size_t buf_len = strlen(buffer);                                \
        append_fmtmsg(buffer, buf_maxlen, buf_len, format, ##args);     \
    } while (0)

#define str_append_strmsg(buffer, buf_maxlen, appendstr, str_len)       \
    do {                                                                \
        size_t buf_len = strlen(buffer);                                \
        append_strmsg(buffer, buf_maxlen, buf_len, appendstr, str_len); \
    } while (0)

#define str_append_fixstr(buffer, buf_maxlen, fixstr)    \
    str_append_strmsg(buffer, buf_maxlen, fixstr, FLEN(fixstr))

#define fix_snprintf(buffer, format, args...)   \
    snprintf(buffer, sizeof(buffer), format, ##args)

#endif    // _STR_DEF_H

