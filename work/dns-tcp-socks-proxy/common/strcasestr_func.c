
#include "common/types.h"
#include "common/optimize.h"
#include "common/str_def.h"

#define TOLOWER(Ch) (isupper (Ch) ? tolower (Ch) : (Ch))

static inline char *strcasechr(char *s, char c, int n)
{
    char *end = s + n;
    do {
        if (TOLOWER(*s) == TOLOWER(c)) {
            goto find_it;
        }
    } while (++s < end);

    return NULL;

find_it:
    return s;
}

/* Two-Way algorithm.  */
#define CANON_ELEMENT(c) TOLOWER (c)
#define CMP_FUNC(p1, p2, l)                \
  strncasecmp ((const char *) (p1), (const char *) (p2), l)
#include "common/str_two_way.h"

/* Find the first occurrence of NEEDLE in HAYSTACK, using
   case-insensitive comparison.  This function gives unspecified
   results in multibyte locales.  */

char *_strcasestr_s(char *haystack_start, size_t haystack_len, char *needle_start, size_t needle_len)
{
    const char *haystack = haystack_start;

    if (unlikely(needle_len > haystack_len)) {
        return NULL;
    }

    if (unlikely(strncasecmp(haystack_start, needle_start, needle_len) == 0)) {
        return haystack_start;
    }

    /* Reduce the size of haystack using strchr, since it has a smaller
       linear coefficient than the Two-Way algorithm.  */
    haystack = strcasechr(haystack_start + 1, *needle_start, haystack_len - 1);
    if (!haystack || unlikely(needle_len == 1)) {
        return (char *) haystack;
    }

    haystack_len -= (haystack - haystack_start);

    /* Perform the search.  Abstract memory is considered to be an array
       of 'unsigned char' values, not an array of 'char' values.  See
       ISO C 99 section 6.2.6.1.  */
    if (needle_len < LONG_NEEDLE_THRESHOLD) {
        return two_way_short_needle((const unsigned char *) (haystack), haystack_len,
                        (const unsigned char *) needle_start, needle_len);
    } else {
        return two_way_long_needle((const unsigned char *) (haystack), haystack_len,
                        (const unsigned char *) needle_start, needle_len);
    }
}

#undef LONG_NEEDLE_THRESHOLD

