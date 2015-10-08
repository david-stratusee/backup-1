#ifndef _STRING_S_H
#define _STRING_S_H

#include "common/types.h"
#include "common/str_def.h"
#include "common/misc.h"
#include "common/optimize.h"
#include "common/umemory.h"

static inline int32_t _s_strncpy(char *dst, int32_t size, char *src, int32_t len)
{
    int32_t l;

    l = MIN(size - 1, len);
    memcpy(dst, src, l);
    dst[l] = 0;

    return l;
}

#define MIN_VALID_PTR_VALUE 100UL
static inline int32_t _strncpy_s(char *dst, int32_t size, char *src, int32_t len)
{
    if (unlikely(size < 1 || len < 1 || (unsigned long)src <= MIN_VALID_PTR_VALUE)) {
        dst[0] = '\0';
        return 0;
    }

    return _s_strncpy(dst, size, src, len);
}

static inline int32_t _strcpy_s(char *dst, int32_t size, char *src)
{
    int32_t len = (((unsigned long)src > MIN_VALID_PTR_VALUE) ? strlen(src) : 0);

    if (unlikely(size < 1 || len < 1)) {
        dst[0] = '\0';
        return 0;
    }

    return _s_strncpy(dst, size, src, len);
}


#define strncpy_s(dst, size, src, len)    \
    _strncpy_s((char *)(dst), (int)(size), (char *)(src), (int)(len))

#define append_strncpy_s(buffer, buf_maxlen, appendstr, str_len)   \
    strncpy_s((buffer) + strlen(buffer), buf_maxlen - strlen(buffer), appendstr, str_len)

#define fix_strncpy_s(dst, src, len)   \
    _strncpy_s((char *)(dst), (int)(sizeof(dst)), (char *)(src), (int)(len))

#define strcpy_s(dst, size, src)    \
    _strcpy_s((char *)(dst), (int)(size), (char *)(src))

#define fix_strcpy_s(dst, src)    \
    _strcpy_s((char *)(dst), (int)(sizeof(dst)), (char *)(src))

/* 
 *    Find the first occurrence of the substring.
 */
char *_memstr_s(char *haystack_start, int32_t haystack_len, char *needle_start, int32_t needle_len);

#define memstr_s(subject, length, pattern, pat_len)    \
    _memstr_s((char *)(subject), (size_t)(length), (char *)(pattern), (size_t)(pat_len))

#define fix_memstr_s(subject, length, fix_pat)  \
    memstr_s(subject, length, fix_pat, FLEN(fix_pat))

char *_strcasestr_s(char *haystack_start, int32_t haystack_len, char *needle_start, int32_t needle_len);

#define strcasestr_s(subject, length, pattern, pat_len)    \
    (char *)_strcasestr_s((char *)(subject), (size_t)(length), (char *)(pattern), (size_t)(pat_len))

#define strstr_case_s(haystack, range, needle, needle_len, case_need)    \
    (case_need ? strcasestr_s(haystack, range, needle, needle_len) : memstr_s(haystack, range, needle, needle_len))

/* 
 *    Find the last occurrence of the substring.
 */
char *_memrstr_s(char *haystack, int32_t range, char *needle, int32_t needle_len);

#define memrstr(haystack_start, haystack_len, needle_start, needle_len)    \
    (char *)_memrstr_s((char *)(haystack_start), (size_t)(haystack_len), (char *)(needle_start), (size_t)(needle_len))

char *_strrstr_case_s(char *haystack, int32_t range, char *needle, int32_t needle_len, int32_t case_need);

#define strrstr_case(haystack, range, needle, needle_len, case_need)    \
    (char *)_strrstr_case_s((char *)(haystack), (int)(range), (char *)(needle), (int)(needle_len), case_need)

static inline int32_t memcpy_s(void *dst, int32_t size, void *src, int32_t n)
{
    int32_t l;

    if (unlikely(size < 1 || n < 1))
        return 0;

    l = MIN(size, n);
    memcpy(dst, src, l);

    return l;
}

void *memrchr(void *s, char c, int32_t n);

size_t strlcpy_s(char *dst, const char *src, size_t siz);

static inline int32_t memchr_pos(char *subject, int32_t len, char ch, int32_t *pos, int32_t max_pos_num)
{
    char *ptr = NULL;
    char *begin = subject, *end = subject + len;
    int32_t pos_num = 0;

    while (begin < end && (ptr = memchr(begin, ch, end - begin)) != NULL) {
        if (pos_num < max_pos_num) {
            pos[pos_num++] = ptr - subject;
            while (++ptr < end && *ptr == ch) {}
        } else {
            break;
        }

        begin = ptr;
    }

    return pos_num;
}

typedef struct _split_node_t {
    int32_t begin_offset;
    int32_t part_len;
} split_node_t;   /* -- end of split_node_t -- */
#define SPLIT_NODE_INIT_VALUE ((split_node_t){.part_len = 0})
int32_t split_pos(char *subject, int32_t len, char ch, split_node_t *split_list, int32_t max_split_num, bool need_strim);
int32_t split_sp_pos(char *subject, int32_t len, split_node_t *split_list, int32_t max_split_num);

unsigned long strtoul_s(const char *nptr, int ptr_len, char **endptr, int base);
#define strtoul_sn(nptr, ptr_len, base) \
    strtoul_s((const char *)(nptr), (int)(ptr_len), NULL, (int)(base))

#endif  // _STRING_S_H
