
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>

#include "common/types.h"
#include "common/optimize.h"
#include "common/misc.h"
#include "common/file_op.h"
#include "common/umemory.h"
#include "common/str_def.h"
#include "common/string_s.h"

unsigned char __check_ascii[0xFF] = {
    [0 ... 0xFE] = 0,
    [0x20] = SP_VALUE,
    [0x09] = SP_VALUE,
    [0x0B] = SP_VALUE,

    [0] = CRLF_VALUE,
    [0x0D] = CRLF_VALUE,
    [0x0A] = CRLF_VALUE,
};

#define    SF_SIFUNC(str, addr, maxlen, type, func) \
                                                    \
    if (!addr || maxlen < (int)sizeof(type)) {      \
        goto err;                                   \
    }                                               \
                                                    \
    if (!str) {                                     \
        *(type *) addr = 0;                         \
    } else {                                        \
        *(type *) addr = (type)func(str, NULL, 0);  \
    }                                               \
                                                    \
    return (sizeof(type));                          \
                                                    \
err:                                                \
    return -1;

/*
 * split funcs
 * return: n >= 0, result len in "addr", n < 0, error
 * sf_s2i: convert string to int
 */
int sf_s2i(char *str, void *addr, int maxlen)
{
    SF_SIFUNC(str, addr, maxlen, int, strtol);
}

int sf_s2ul(char *str, void *addr, int maxlen)
{
    SF_SIFUNC(str, addr, maxlen, unsigned long, strtoul);
}

int sf_s2us(char *str, void *addr, int maxlen)
{
    SF_SIFUNC(str, addr, maxlen, uint16_t, strtoul);
}

int sf_scp(char *str, void *addr, int maxlen)
{
    int n = 0;

    if (!addr || maxlen < 1)
        goto err;

    if (!str) {
        *(char *) addr = '\0';
        goto ret;
    }

    if (strlen(str) + 1 > (uint32_t) maxlen) {
        *(char *) addr = '\0';
        goto ret;
    }

    strcpy(addr, str);

      ret:
    return n;
      err:
    return -1;
}

/*
 * split a string to n part, every part will be a param to 'func', and fill the result
 * to the address stored in struct split_var 's addr.
 * if 'func' == NULL, call string copy(sf_scp) as default func
 */
int str_split(char *orig_str, int delim, split_fp func, struct split_var split_var_table[], int split_var_max)
{
    char *tbuf = NULL;
    char *head = NULL;
    int nvar = 0;
    char *p = NULL;

    if (orig_str == NULL || orig_str[0] == '\0') {
        return -1;
    }

    tbuf = umalloc(strlen(orig_str) + 1);
    if (!tbuf) {
        goto err;
    }
    strcpy(tbuf, orig_str);

    if (func == NULL) {
        func = sf_scp;
    }

    head = tbuf;
    while (head != NULL) {
        p = strchr(head, delim);
        if (p) {
            *p = '\0';
        }

        if (func(head, split_var_table[nvar].addr, split_var_table[nvar].maxlen) < 0) {
            fprintf(stderr, "Error: splited data convert error(%s)\n", head);
            continue;
        }

        if ((++nvar) == split_var_max) {
            break;
        }

        head = (p != NULL) ? (p + 1) : NULL;
    }

    free(tbuf);
    return nvar;

      err:
    if (tbuf) {
        free(tbuf);
        tbuf = NULL;
    }

    return -1;
}

/*
 * search any char in 'delim' which is first ocurr in 'string'
 */
char *strchr_set(char *string, char *delim)
{
    int str_i = 0;
    int str_len = strlen(string);

    if (string == NULL || delim == NULL) {
        return NULL;
    }

    for (str_i = 0; str_i < str_len; str_i++) {
        if (strchr(delim, string[str_i]) != NULL) {
            /* find char in delimiters */
            break;
        }
    }

    if (str_i == str_len) {
        return NULL;
    } else {
        return (string + str_i);
    }
}

#define REVERSE_CHAR(ch) \
    (isprint(ch) ? (ch) : '.')

const char transtbl_i2a[16] = "0123456789abcdef";
//extern const char transtbl_i2a[16];
static inline int append_char_data(char *data, int maxlen, unsigned char buf_char)
{
    if (maxlen <= 2) {
        return 0;
    }

    data[0] = transtbl_i2a[(buf_char >> 4) & 0xf];
    data[1] = transtbl_i2a[buf_char & 0xf];

    return 2;
}

#define ASCII_STRING_MAX 128
static inline void __print_line(FILE *fp, char *ascii_string, int ascii_len,
                                char *last_ascii_string, int *last_ascii_string_len,
                                int32_t prefix_len, int *continue_same_line_num)
{
    if (!strequal(ascii_string + prefix_len, ascii_len - prefix_len, last_ascii_string, *last_ascii_string_len)) {
        *last_ascii_string_len = ascii_len - prefix_len;

        ascii_string[ascii_len++] = '|';
        ascii_string[ascii_len++] = '\n';
        fp_write_len(fp, ascii_string, ascii_len);

        if ((*continue_same_line_num) > 0) {
            (*continue_same_line_num) = 0;
        }
        strncpy(last_ascii_string, ascii_string + prefix_len, *last_ascii_string_len);
    } else {
        if ((*continue_same_line_num) == 0) {
            append_fixstr(ascii_string, ASCII_STRING_MAX, prefix_len, "... ...\n");
            fp_write_len(fp, ascii_string, prefix_len);
        }
        (*continue_same_line_num)++;
    }
}

static void _various_print_buf(FILE *fp, unsigned char *buf, int nbuf, const char *funcname, const int line_no, const char *desc_buf, bool relative_addr, bool only_ascii)
{
#define DEFAULT_COLUMN 16
    int i = 0, j = 0;
    int col = DEFAULT_COLUMN;
    unsigned char ch = 0;
    bool first_line = true;
    unsigned char *begin = NULL;
    char ascii_string[ASCII_STRING_MAX] = {0};
    char last_ascii_string[ASCII_STRING_MAX] = {0};
    int ascii_len = 0, last_ascii_string_len = 0, prefix_len = 0;
    int half_col = 0;
    int left_fill = 0;
    int left_column = 0;
    int continue_same_line_num = 0;

    fprintf(fp, "--- [%s:%u]-[%s] begin: [len:%u] ---\n", funcname, line_no, desc_buf, nbuf);

    half_col = (col >> 1);
    for (i = 0; i < nbuf; i++) {
        if (first_line) {
            ascii_len = 0;

            if (relative_addr) {
                append_fmtmsg(ascii_string, ASCII_STRING_MAX, ascii_len, "%04x  ", i);
            } else {
#if __WORDSIZE == 64
                append_fmtmsg(ascii_string, ASCII_STRING_MAX, ascii_len, "%016lx  ", (unsigned long)(uintptr_t)(buf + i));
#else
                append_fmtmsg(ascii_string, ASCII_STRING_MAX, ascii_len, "%08lx  ", (unsigned long)(uintptr_t)(buf + i));
#endif
            }
            prefix_len = ascii_len;
            first_line = false;
        }

        ascii_len += append_char_data(ascii_string + ascii_len, ASCII_STRING_MAX - ascii_len, (buf[i]));
        ascii_string[ascii_len++] = ' ';

        if ((i + 1) % half_col == 0) {
            ascii_string[ascii_len++] = ' ';

            if ((i + 1) % col == 0) {
                if (!only_ascii) {
                    append_fixstr(ascii_string, ASCII_STRING_MAX, ascii_len, "|");
                    begin = buf + i - (col - 1);
                    for (j = 0; j < col; j++) {
                        ch = *(begin + j);
                        append_fmtmsg(ascii_string, ASCII_STRING_MAX, ascii_len, "%c", REVERSE_CHAR(ch));
                    }
                }

                __print_line(fp, ascii_string, ascii_len, last_ascii_string, &last_ascii_string_len, prefix_len, &continue_same_line_num);
                first_line = true;
            }
        }
    }

    if (first_line) {
        return;
    }

    if (!only_ascii) {
        left_fill = col - i % col;

        for (j = 0; j < left_fill; j++) {
            append_fixstr(ascii_string, ASCII_STRING_MAX, ascii_len, "   ");
        }
        if (left_fill > half_col) {
            ascii_string[ascii_len++] = ' ';
        }

        append_fixstr(ascii_string, ASCII_STRING_MAX, ascii_len, " |");

        left_column = i % col;
        for (j = 0; j < left_column; j++) {
            ch = *(buf + i + j - left_column);
            append_fmtmsg(ascii_string, ASCII_STRING_MAX, ascii_len, "%c", REVERSE_CHAR(ch));
        }
    }

    __print_line(fp, ascii_string, ascii_len, last_ascii_string, &last_ascii_string_len, prefix_len, &continue_same_line_num);
    fprintf(fp, "--- [%s:%u] end   ---\n", funcname, line_no);

    return;
}

void _print_buf(FILE *fp, unsigned char *buf, int nbuf, const char *funcname, const int line_no, bool relative_addr, bool only_ascii, const char* fmt, ...)
{
    char desc_buf[256] = "\0";
    va_list va;

    if (PTR_NULL(fp)) {
        return;
    }

    va_start(va, fmt);
    vsnprintf(desc_buf, 256, fmt, va);
    va_end(va);

    if (unlikely(buf == NULL || nbuf <= 0)) {
        fprintf(fp, "get buffer[%p], len[%d], for desc: %s\n", buf, nbuf, desc_buf);
        return;
    }

    _various_print_buf(fp, buf, nbuf, funcname, line_no, desc_buf, relative_addr, only_ascii);
    fflush(fp);
}

int execute(char *command, int max_cmdlen, char *buf, int bufmax)
{
    FILE *fp = NULL;

    if (buf && bufmax) {
        int cmd_len = strlen(command);
        append_fixstr(command, max_cmdlen, cmd_len, " 2>&1");
    }

    if ((fp = popen(command, "r")) == NULL) {
        return -1;
    }

    int len = 0;
    if (!buf || !bufmax) {
        goto ret;
    }

    while ((buf[len] = fgetc(fp)) != EOF && (++len) < bufmax) ;

    while (len > 0 && (buf[len - 1] == 0x0a || buf[len - 1] == 0x0d)) {
        len--;
    }

    buf[len] = '\0';

#ifdef MISC_DEBUG
    printf("len: %d\n", len);
    print_buf(buf, len, 0);
#endif

ret:
    pclose(fp);
    return len;
}
