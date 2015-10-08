/************************************************
 *       Filename: misc.h
 *    Description: 
 *        Created:
 *         Author: dengwei
 ************************************************/
#ifndef _MISC_H
#define _MISC_H
#include "common/cplus_define.h"
CPLUS_BEGIN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common/types.h"
#include "common/optimize.h"

#define PRINTF_FORMAT_ARG(x,y) __attribute__ ((__format__ (__printf__, x, y)))

#define ARG_UNUSED __attribute__ ((__unused__))
#define VAR_UNUSED(x) (void)(x)

/* #include <string.h> */
/* structures for str_split */
struct split_var {
    void *addr;
    int maxlen;
};

typedef int(*split_fp)(char *str, void *addr, int maxlen);
int sf_s2i(char *str, void *addr, int maxlen);
int sf_scp(char *str, void *addr, int maxlen);
int sf_s2ul(char *str, void *addr, int maxlen);
int sf_s2us(char *str, void *addr, int maxlen);
int str_split(char *orig_str, int delim, split_fp func, struct split_var split_var_table[], int split_var_max);

void _print_buf(FILE *fp, uint8_t *buf, int nbuf, const char *funcname, const int line_no, bool relative_addr, bool only_ascii, const char* fmt, ...)  \
                PRINTF_FORMAT_ARG(8, 9);

#define print_buf(buf, nbuf, fmt, args...)    \
    _print_buf(stdout, (uint8_t *)(buf), (int)(nbuf), __func__, __LINE__, true, false, fmt, ##args)

#define print_buf_tofile(fp, buf, nbuf, fmt, args...)    \
    _print_buf(fp, (uint8_t *)(buf), (int)(nbuf), __func__, __LINE__, true, false, fmt, ##args)

#define print_buf_detail(buf, nbuf, relative, only_ascii, fmt, args...)    \
    _print_buf(stdout, (uint8_t *)(buf), (int)(nbuf), __func__, __LINE__, relative, only_ascii, fmt, ##args)

#define print_buf_tofile_detail(fp, buf, nbuf, relative, only_ascii, fmt, args...)    \
    _print_buf(fp, (uint8_t *)(buf), (int)(nbuf), __func__, __LINE__, relative, only_ascii, fmt, ##args)

char *strchr_set(char *string, char *delim);

#define SENSE_VALUE 0
#define SP_VALUE    1
#define CRLF_VALUE  2
extern unsigned char __check_ascii[0xFF];
#define IS_SPACE(ch) (__check_ascii[(uint8_t)(ch)] == SP_VALUE)
#define IS_CRLF(ch) (__check_ascii[(uint8_t)(ch)] == CRLF_VALUE)
#define IS_SENSE(ch) (__check_ascii[(uint8_t)(ch)] == SENSE_VALUE)

#define SEARCH_FORWARD  1             /* not allow to change */
#define SEARCH_BACKWARD -1
/*
 * search_nosense
 * search not sense characters, include ' ', '\t', '\n', '\r' and '\0' 
 * NULL: not found
 * step: >=0 = forward search
 *     <0 = backward search
 */
static inline char *_search_nosense(char *begin, size_t nsearch, int step)
{
    char *p;
    char *pret = NULL;
    size_t i;

    if (unlikely(!begin || nsearch <= 0)) {
        return NULL;
    }

    p = begin;
    for (i = 0; i < nsearch; i++) {
        if (!IS_SENSE(p[0])) {
            pret = p;
            break;
        }

        p = (step >= 0) ? (p + 1) : (p - 1);
    }

    return pret;
}
#define search_nosense(begin, nsearch)  \
    _search_nosense((char *)(begin), (size_t)(nsearch), SEARCH_FORWARD)
#define search_nosense_r(begin, nsearch)    \
    _search_nosense((char *)(begin), (size_t)(nsearch), SEARCH_BACKWARD)

/*
 * search_sense
 * search sense characters, exclude ' ', '\t', '\n', '\r' and '\0' 
 * NULL: not found
 * step: >=0 = forward search
 *     <0 = backward search
 */
static inline char *_search_sense(char *begin, size_t nsearch, int step)
{
    char *p;
    char *pret = NULL;
    size_t i;

    if (unlikely(!begin || nsearch <= 0)) {
        return NULL;
    }

    p = begin;
    for (i = 0; i < nsearch; i++) {
        if (IS_SENSE(p[0])) {
            pret = p;
            break;
        }

        p = (step >= 0) ? (p + 1) : (p - 1);
    }

    return pret;
}
#define search_sense(begin, nsearch)  \
    _search_sense((char *)(begin), (size_t)(nsearch), SEARCH_FORWARD)
#define search_sense_r(begin, nsearch)    \
    _search_sense((char *)(begin), (size_t)(nsearch), SEARCH_BACKWARD)

static inline char *ltrim(char *str, int *str_len)
{
    char *end = str + *str_len;
    str = search_sense(str, *str_len);
    if (str) {
        *str_len = (int)(end - str);
    } else {
        *str_len = 0;
    }

    return str;
}

static inline char *strim(char *str, int *str_len)
{
    char *end = str + *str_len;

    char *pline = search_sense(str, *str_len);
    if (pline == NULL) {
        *str_len = 0;
        return NULL;
    }

    char *p = search_sense_r(end - 1, end - pline);
    if (p != NULL) {
        *str_len = (int)(p + 1 - pline);
    } else {
        *str_len = 0;
    }

    return pline;
}

static inline void rstrim(char *str, int *str_len)
{
    char *end = str + *str_len;

    char *p = search_sense_r(end - 1, end - str);
    if (p != NULL) {
        *str_len = (int)(p + 1 - str);
    } else {
        *str_len = 0;
    }

    return;
}

#ifndef MIN
#define MIN(a, b)    ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a, b)    ((a) > (b) ? (a) : (b))
#endif

#ifndef SAFE_MIN
#define SAFE_MIN(x, y)     \
({                 \
    typeof(x) _x = (x);    \
    typeof(y) _y = (y);    \
    _x < _y ? _x : _y;     \
 })

// (void) (&_x == &_y);
#endif

#ifndef SAFE_MAX
#define SAFE_MAX(x, y)     \
({                 \
    typeof(x) _x = (x);    \
    typeof(y) _y = (y);    \
    _x > _y ? _x : _y;     \
 })

//    (void) (&_x == &_y);
#endif

#ifndef MIN_TYPE
#define MIN_TYPE(type, x, y)     \
({                 \
     type __x = (x);     \
     type __y = (y);     \
     __x < __y ? __x: __y;     \
 })
#endif

#ifndef MAX_TYPE
#define MAX_TYPE(type, x, y)     \
({                 \
     type __x = (x);     \
     type __y = (y);     \
     __x > __y ? __x: __y;     \
 })
#endif

#ifndef offset_of
#define offset_of(type, member) __builtin_offsetof(type, member)
/* #define offset_of(type, member)  ((char *)(&((type *)0)->member) - (char *)0) */
#endif

#ifndef MEM_ALIGN_SIZE
#define MEM_ALIGN_SIZE(size, align) (((size) + (align) - 1) & ~((align) - 1))
#define MEM_ADDR_ALIGN(size) MEM_ALIGN_SIZE(size, 8)
#endif

#define _ITOA(data) #data
#define ITOA(data) _ITOA(data)

#define ZERO_STRUCT(stru)    \
    memset(&(stru), 0, sizeof(stru))

#if 0
/********************************************
 * tolower and toupper is under "ctype.h"
 ********************************************/
static inline uint8_t tolower(uint8_t c)
{
    return ((c >= 'A' && c <= 'Z') ? (c | 0x20) : c)
}

static inline uint8_t toupper(uint8_t c)
{
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 'A';
    return c;
}
#else
#include <ctype.h>
#endif

int execute(char *command, int max_cmdlen, char *buf, int bufmax);
int execute_cmd(char *command);
void change_workdir(char *cmd_path);
int fread_oneline(char *filename, char *readline, int max_len);
bool check_proc_name(char *proc_comm_name, char *check_name);
bool pid_is_valid(pid_t check_pid, char *proc_name);
bool check_procecss_exist(char *pid_file_dir, char *proc_name);
int get_total_mem(void);

#define UINT_GT(x, y)     ((uint32_t)((y) - (x)) > 0x7fffffffU)
#define UINT_LT(x, y)     ((uint32_t)((x) - (y)) > 0x7fffffffU)
#define UINT_EQ(x, y)     ((uint32_t)(x) == (y))
#define UINT_GE(x, y)     ((uint32_t)((x) - (y)) < 0x7fffffffU)
#define UINT_LE(x, y)     ((uint32_t)((y) - (x)) < 0x7fffffffU)

#define UINT16_GT(x, y)     ((uint16_t)((y) - (x)) > (uint16_t)0x7fffU)
#define UINT16_LT(x, y)     ((uint16_t)((x) - (y)) > (uint16_t)0x7fffU)
#define UINT16_EQ(x, y)     ((uint16_t)(x) == (y))
#define UINT16_GE(x, y)     ((uint16_t)((x) - (y)) < (uint16_t)0x7fffU)
#define UINT16_LE(x, y)     ((uint16_t)((y) - (x)) < (uint16_t)0x7fffU)

#if 0
/********************************************
 * maybe get some wrong, need check
 ********************************************/
#define UBIT_GT(x, y, utype)     (utype)((y) - (x)) > (utype)((1U << ((uint32_t)((sizeof(utype)) << 3) - 1U)) - 1U)
#define UBIT_LT(x, y, utype)     (utype)((x) - (y)) > (utype)((1U << ((uint32_t)((sizeof(utype)) << 3) - 1U)) - 1U)
#define UBIT_EQ(x, y, utype)     ((utype)(x) == (utype)(y))
#define UBIT_GE(x, y, utype)     (utype)((x) - (y)) < (utype)((1U << ((uint32_t)((sizeof(utype)) << 3) - 1U)) - 1U)
#define UBIT_LE(x, y, utype)     (utype)((y) - (x)) < (utype)((1U << ((uint32_t)((sizeof(utype)) << 3) - 1U)) - 1U)
#endif

/********************************************
 * from the code of "include/linux/kernel.h"
 ********************************************/
#define k_container_of(ptr, type, member)              \
({                                                     \
    const typeof(((type *)0)->member) *__mptr = (ptr); \
    (type *)((char *)__mptr - offsetof(type,member));  \
})

CPLUS_END
#endif    // _MISC_H

