
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
#include "common/umemory.h"
#include "common/file_op.h"
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

int execute_cmd(char *command)
{
    int pid = fork();
    if (pid < 0) {
        return -1;
    }

    if (pid == 0) {
        daemon(0, 0);

        int32_t command_len = strlen(command);
        char *new_command = umalloc(command_len + 1);
        if (PTR_NOT_NULL(new_command)) {
            split_node_t pos[16] = {{0, 0}};

            strncpy_s(new_command, command_len + 1, command, command_len);
            int32_t pos_num = split_pos(new_command, command_len, ' ', pos, 16, true);

            int32_t idx = 0;
            for (idx = 0; idx < pos_num; ++idx) {
                new_command[pos[idx].begin_offset + pos[idx].part_len] = '\0';
            }

            char *argv[16] = {NULL};
            for (idx = 0; idx < pos_num; ++idx) {
                argv[idx] = new_command + pos[idx].begin_offset;
                printf("get argv[%u]: %s\n", idx, argv[idx]);
            }
            char *envp[]={0};
            execve(argv[0], argv, envp);

            ufree(new_command);
        }

        exit(EXIT_SUCCESS);
    } else {
        wait(NULL);
    }

    return 0;
}

// get path from cmd
static int get_current_dir(char *cmd, char *path, int max_len)
{
    char *pidx = NULL;
    int copy_len = 0;

    if ((pidx = strrchr(cmd, '/')) != NULL) {
        copy_len = pidx - cmd;
        if ((copy_len == 1) && (cmd[0] == '.')) {
            return 0;
        }

        if (copy_len >= max_len) {
            return -1;
        }

        memcpy(path, cmd, copy_len);
        path[copy_len] = '\0';
    }

    return copy_len;
}

void change_workdir(char *cmd_path)
{
    if (cmd_path == NULL || cmd_path[0] == '\0') {
        return;
    }

    char path[1024] = { 0 };
    if (get_current_dir(cmd_path, path, 1024) > 0) {
#ifdef MISC_DEBUG
        print("path: %s\n", path);
#endif
        chdir(path);
    }
}

int fread_oneline(char *filename, char *readline, int max_len)
{
    char *pline = readline;
    FILE *fp_oneline = fopen((const char *)filename, "r");
    if (fp_oneline == NULL) {
        fprintf (stderr, "couldn't open file '%s'; %s\n", filename, strerror(errno));
        return -1;
    }

    int32_t buf_len = 0;
    while (memset(readline, 0, max_len) && fgets(readline, max_len, fp_oneline)) {
        buf_len = strlen(pline);
        pline = strim(readline, &buf_len);

        if (unlikely(buf_len == 0 || pline == NULL || pline[0] == '#')) {
            buf_len = 0;
            continue;
        }

        /***************************************************
         * TODO: use pline here, filtered # and empty lines
         ***************************************************/
        buf_len = strlen(pline);
        if (pline != readline) {
            memmove(readline, pline, buf_len);
            readline[buf_len] = '\0';
        }
        break;
    }

    fclose(fp_oneline);
    return buf_len;
}

#define PROC_STR_LEN 128
bool check_proc_name(char *proc_comm_name, char *check_name)
{
	FILE *fp = NULL;
    bool ret_val = false;

	if (NULL == (fp = fopen(proc_comm_name, "r"))) {
		goto RET;
	}

	char buf[PROC_STR_LEN] = {0};
    int buf_len = 0;
	if (fgets(buf, PROC_STR_LEN, fp) != NULL) {
        buf_len = strlen(buf);
        rstrim(buf, &buf_len);

        if (case_strequal(buf, buf_len, check_name, strlen(check_name))) {
            ret_val = true;
        }
	}

RET:
	if (NULL != fp) {
		fclose(fp);
	}

	return ret_val;
}

// get total mem, unit k
int get_total_mem(void)
{
    int total_mem = -1;

    FILE *meminfo = NULL;    /* input-file pointer */
    char *meminfo_file_name = "/proc/meminfo";    /* input-file name    */

    meminfo = fopen((const char *) meminfo_file_name, "r");
    if (meminfo == NULL) {
        //CMLIB_ERR(ELSE, "couldn't open file '%s'; %s\n", meminfo_file_name, strerror(errno));
        goto err;
    }

    char strline[1024] = { 0 };
    char *pline = NULL;

#define TOTAL_MEM_HEAD "MemTotal:"
    int head_len = strlen(TOTAL_MEM_HEAD);

    while (memset(strline, 0, 1024)
           && fgets(strline, 1024, meminfo) != NULL) {
        pline = search_sense(strline, strlen(strline));
        if (pline == NULL) {
            continue;
        }

        if (strncasecmp(pline, TOTAL_MEM_HEAD, head_len) == 0) {
            pline += head_len;
            pline = search_sense(pline, strlen(pline));
            if (pline != NULL) {
                total_mem = atoi(pline);
                break;
            }
        }
    }

    if (fclose(meminfo) == EOF) {
        //CMLIB_ERR(ELSE, "couldn't close file '%s'; %s\n", meminfo_file_name, strerror(errno));
        goto err;
    }

    return total_mem;

err:
    return -1;
}
