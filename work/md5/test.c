/************************************************
 *       Filename: test.c
 *    Description:
 *        Created: 2015-02-07 16:13
 *         Author: dengwei david@stratusee.com
 ************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "md5.h"

#define md5_error printf

static const char transtbl_i2a[16] = "0123456789abcdef";
static int32_t print_buf_hex_local(uint8_t *in_str, int32_t in_len, int32_t max_len)
{
    int32_t idx = 0;
    int32_t out_len = (in_len << 1), out_idx;

    if (out_len >= max_len) {
        in_len = ((max_len - 1) >> 1);
        out_len = (in_len << 1);
    }

    out_idx = out_len - 2;

    for (idx = in_len - 1, out_idx = out_len - 2;
            idx >= 0;
            --idx, out_idx -= 2) {
        in_str[out_idx + 1] = transtbl_i2a[in_str[idx] & 0xf];
        in_str[out_idx] = transtbl_i2a[(in_str[idx] >> 4) & 0xf];
    }

    in_str[out_len] = '\0';
    return out_len;
}

/************************************************
 *         Name: main
 *  Description: main function
 *     Argument:
 *       Return:
 ************************************************/
int main (int argc, char *argv[])
{
    if (argc == 0) {
        fprintf(stderr, "string is needed as one argument\n");
        return -1;
    }

    MD5_CONTEXT context;
    uint8_t checksum[33] = {'\0'};
    MD5Init(&context);
    char *input = argv[1];

    MD5Update(&context, (uint8_t *)input, strlen(input));
    MD5Final(checksum, &context);

    print_buf_hex_local(checksum, 16, 33);
    printf("%s\n", checksum);
    return EXIT_SUCCESS;
}       /* -- end of function main -- */
