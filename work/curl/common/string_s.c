
#include "common/types.h"
#include "common/str_def.h"
#include "common/misc.h"    /* for macro MIN */
#include "common/optimize.h"    /* for macro likely */
#include "common/string_s.h"
#include <errno.h>

char *_memrstr_s(char *haystack, int range, char *needle, int needle_len)
{
    char *p = haystack + range;

    if (unlikely(needle_len < 1 || !needle)) {
        return haystack;
    }

    if (unlikely(needle_len > range || !range || !haystack)) {
        return NULL;
    }

    p -= needle_len;

    while (p >= haystack) {
        if ((*p) == (*needle)) {
            if (!memcmp(p, needle, needle_len)) {
                return p;
            }
        }

        p--;
    }

    return NULL;
}

char *_strrstr_case_s(char *haystack, int range, char *needle, int needle_len, int case_need)
{
    char *p = (haystack + range);

    if (unlikely(needle_len < 1 || !needle)) {
        return haystack;
    }

    if (unlikely(needle_len > range || !range || !haystack)) {
        return NULL;
    }

    p -= needle_len;

    if (case_need == 0) {
        while (p >= haystack) {
            if ((*p | 0x20) == (*needle | 0x20)) {
                if (!strncasecmp((char *) p, (char *) needle, needle_len)) {
                    return p;
                }
            }

            p--;
        }
    } else {
        while (p >= haystack) {
            if ((*p) == (*needle)) {
                if (!memcmp(p, needle, needle_len)) {
                    return p;
                }
            }

            p--;
        }
    }

    return NULL;
}

void *memrchr_s(void *s, char c, int n)
{
    char *p = NULL;
    char *begin = NULL;

    if (unlikely(!s || n <= 0)) {
        return NULL;
    }

    begin = (char *) s;
    p = begin + n - 1;

    while ((p > begin) && ((*p) != c)) {
        p--;
    }

    if ((*p) != c) {
        return NULL;
    }

    return p;
}

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t strlcpy_s(char *dst, const char *src, size_t siz)
{
    char *d = dst;
    const char *s = src;
    size_t n = siz;

    /*  Copy as many bytes as will fit */
    if (n != 0) {
        while (--n != 0) {
            if ((*d++ = *s++) == '\0') {
                break;
            }
        }
    }

    /*  Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0) {
        if (siz != 0) {
            *d = '\0';    /*  NUL-terminate dst */
        }

        while (*s++)
            ;
    }

    return (s - src - 1);   /*  count does not include NUL */
}

static inline bool __split_insert_one_part(char *subject, char *begin, int32_t len, split_node_t *split_node, bool need_strim)
{
    bool insert_ok = false;
    /* begin, len */
    split_node->part_len = len;

    if (likely(need_strim)) {
        begin = strim(begin, &(split_node->part_len));
    }

    if (PTR_NOT_NULL(begin) && split_node->part_len > 0) {
        split_node->begin_offset = begin - subject;
        insert_ok = true;
    }

    return insert_ok;
}

static inline char *__get_ch_part_end(char *begin, char *end, char ch)
{
    char *part_end = memchr(begin, ch, end - begin);

    if (PTR_NULL(part_end)) {
        part_end = end;
    }

    return part_end;
}

int32_t split_pos(char *subject, int32_t len, char ch, split_node_t *split_list, int32_t max_split_num, bool need_strim)
{
    char *ptr = NULL;
    char *begin = subject, *end = subject + len;
    int32_t index = 0;

    if (unlikely(len <= 0 || max_split_num <= 0)) {
        return 0;
    }

    while (begin < end) {
        ptr = __get_ch_part_end(begin, end, ch);

        /* begin -> ptr */
        if (__split_insert_one_part(subject, begin, ptr - begin, split_list + index, need_strim)
                && (++index) == max_split_num) {
            break;
        }

        while (++ptr < end && *ptr == ch) {}

        begin = ptr;
    }

    return index;
}

static inline char *__get_sp_part_end(char *begin, char *end, bool *get_quotation)
{
    char *part_end = NULL;

    if (begin[0] == '"' && end - begin > 1) {
        begin++;
        part_end = memchr(begin, (int32_t)('"'), end - begin);

        if (PTR_NOT_NULL(part_end)) {
            *get_quotation = true;
            return (part_end + 1);
        }
    }

    *get_quotation = false;
    part_end = search_nosense(begin, end - begin);

    if (PTR_NULL(part_end)) {
        part_end = end;
    }

    return part_end;
}

int32_t split_sp_pos(char *subject, int32_t len, split_node_t *split_list, int32_t max_split_num)
{
    char *ptr = NULL;
    int32_t index = 0;
    char *begin = search_sense(subject, len);
    char *end = subject + len;

    if (unlikely(begin == NULL || max_split_num <= 0)) {
        return 0;
    }

    bool get_quotation = false;
    bool insert_ok;

    while (begin < end) {
        ptr = __get_sp_part_end(begin, end, &get_quotation);

        /* begin -> ptr */
        if (!get_quotation) {
            insert_ok = __split_insert_one_part(subject, begin, ptr - begin, split_list + index, false);
        } else {
            insert_ok = __split_insert_one_part(subject, begin + 1, ptr - begin - 2, split_list + index, false);
        }

        if (insert_ok && (++index) == max_split_num) {
            break;
        }

        ptr = search_sense(ptr, end - ptr);

        if (PTR_NULL(ptr)) {
            ptr = end;
        }

        begin = ptr;
    }

    return index;
}

#define STRTOL_END_SET(nptr, ptr_end, endptr, __dec)   \
do {                                                   \
    if (unlikely(nptr >= ptr_end)) {                   \
        if (PTR_NOT_NULL(endptr)) {                    \
            *endptr = (char *) nptr - (__dec ? 1 : 0); \
        }                                              \
        return 0L;                                     \
    }                                                  \
} while (0)

static unsigned char __ascii_revert_table[0xFF] = {
    [0 ... 0xFE] = 0xFF,
    ['0'] = 0,
    ['1'] = 1,
    ['2'] = 2,
    ['3'] = 3,
    ['4'] = 4,
    ['5'] = 5,
    ['6'] = 6,
    ['7'] = 7,
    ['8'] = 8,
    ['9'] = 9,

    ['a'] = 10,
    ['b'] = 11,
    ['c'] = 12,
    ['d'] = 13,
    ['e'] = 14,
    ['f'] = 15,

    ['A'] = 10,
    ['B'] = 11,
    ['C'] = 12,
    ['D'] = 13,
    ['E'] = 14,
    ['F'] = 15,
};
unsigned long strtoul_s(const char *nptr, int ptr_len, char **endptr, int base)
{
    unsigned long result = 0;
    long sign = 1;
    const char *ptr_end = nptr + ptr_len;

    while (nptr < ptr_end && (*nptr == ' ' || *nptr == '\t')) {
        ++nptr;
    }
    STRTOL_END_SET(nptr, ptr_end, endptr, false);

    if (*nptr == '-') {
        sign = -1;
        ++nptr;
        STRTOL_END_SET(nptr, ptr_end, endptr, true);
    } else if (*nptr == '+') {
        ++nptr;
        STRTOL_END_SET(nptr, ptr_end, endptr, true);
    }

    if (unlikely((*nptr < '0' || *nptr > '9') && (base != 16))) {
        if (PTR_NOT_NULL(endptr)) {
            *endptr = (char *) nptr;
        }

        return 0L;
    }

    if (base != 10 && base != 16 && base != 8) {
        base = 10;
        if (*nptr == '0' && nptr + 1 < ptr_end) {
            if (nptr[1] == 'x' || nptr[1] == 'X') {
                base = 16;
                nptr += 2;
            } else {
                base = 8;
            }
        }
    } else if (base == 16) {
        if (nptr + 1 < ptr_end && nptr[0] == '0' && (nptr[1] == 'x' || nptr[1] == 'X')) {
            nptr += 2;
        }
    }

    unsigned char max_digit = 0;
    unsigned long max_result = 0;
    unsigned long max_mode = 0;
    unsigned int base_mode = 0;
    switch (base) {
        case 10:
            max_digit = 9;
            max_result = ULONG_MAX / 10;
            max_mode = ULONG_MAX % 10;
            break;
        case 16:
            max_digit = 9;
            max_result = ULONG_MAX / 16;
            max_mode = ULONG_MAX % 16;
            base_mode = 4;
            break;
        case 8:
            max_digit = 7;
            max_result = ULONG_MAX / 8;
            max_mode = ULONG_MAX % 8;
            base_mode = 3;
            break;
        default:
            break;
    }

    unsigned long digval;
    while (nptr < ptr_end) {
        if ((*nptr >= '0' && *nptr <= '0' + max_digit)
                || ((base == 16)
                    && ((*nptr >= 'a' && *nptr <= 'f') || (*nptr >= 'A' && *nptr <= 'F')))) {
            digval = (unsigned long)__ascii_revert_table[(uint8_t)(*nptr)];
        } else {
            break;
        }

        if (unlikely(result > max_result
                    || (result == max_result && digval > max_mode))) {
            errno = ERANGE;

            if (endptr != NULL) {
                *endptr = (char *) nptr;
            }

            return ULONG_MAX;
        }

        if (base == 10) {
            result = PLUS10(result);
        } else {
            result <<= base_mode;
        }
        result += digval;
        ++nptr;
    }

    if (endptr != NULL) {
        *endptr = (char *) nptr;
    }

    return (result * sign);
}


