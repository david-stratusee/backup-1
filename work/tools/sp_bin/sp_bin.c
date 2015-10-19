/************************************************
 *    Description:
 *        Created: 2015-10-19 15:29
 *         Author: dengwei
 ************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>

#define fp_read_len(fp, strings, __maxlen) \
    fread((void *)(strings), 1, (size_t)(__maxlen), fp)

#define fp_write_len(fp, strings, __len)    \
    fwrite((void *)(strings), 1, (size_t)(__len), fp)

#define BUF_LEN 4096

/************************************************
 *         Name: main
 *  Description: main function
 *     Argument:
 *       Return:
 ************************************************/
int main (int argc, char *argv[])
{
    if (argc < 3) {
        printf("Usage: %s filename [-]split_num\n\tminus split_num means merge file", argv[0]);
        return EXIT_FAILURE;
    }

    char *input_filename = argv[1];
    int32_t sp_num = atoi(argv[2]);

    if (sp_num > 0) {
        FILE *fp = fopen(input_filename, "rb");
        if (!fp) {
            return EXIT_FAILURE;
        }

        struct stat st;
        memset(&st, 0, sizeof(struct stat));
        fstat(fileno(fp), &st);
        int32_t file_len = (int32_t)(st.st_size);
        int32_t each_part_len = (file_len / sp_num);
        int32_t last_part_len = file_len - (each_part_len * (sp_num - 1));

        printf("the file(%s) length is: %d, split into %d parts, each part length: %d, last_part_len: %d\n",
                input_filename, file_len, each_part_len, last_part_len);
        int32_t new_filename_len = strlen(input_filename) + 3;
        char new_filename[new_filename_len];
        memset(new_filename, 0, new_filename_len);
        FILE *new_fp = NULL;

        char buf[BUF_LEN] = { 0 };
        uint32_t readlen = 0;
        uint32_t left_len = 0;

        int32_t idx = 0;
        for (idx = 0; idx < sp_num; ++idx) {
            sprintf(new_filename, "%s%02u", input_filename, idx);
            new_fp = fopen(new_filename, "wb");
            if (!new_fp) {
                fprintf(stderr, "error when create file: %s\n", new_filename);
                goto err;
            }

            printf("create file: %s\n", new_filename);

            if (idx < sp_num - 1) {
                left_len = each_part_len;
            } else {
                left_len = last_part_len;
            }

            while ((readlen = (left_len >= BUF_LEN ? BUF_LEN : left_len)) > 0
                    && (readlen = fp_read_len(fp, buf, readlen)) != 0) {
                left_len -= readlen;
                if (fp_write_len(new_fp, buf, readlen) < readlen) {
                    printf("error when output %s\n", new_filename);
                    goto err;
                }
            }

            fclose(new_fp);
        }

err:
        fclose(fp);
    } else {
        FILE *fp = fopen(input_filename, "wb");
        if (!fp) {
            return EXIT_FAILURE;
        }

        int32_t new_filename_len = strlen(input_filename) + 3;
        char new_filename[new_filename_len];
        memset(new_filename, 0, new_filename_len);
        FILE *new_fp = NULL;

        char buf[BUF_LEN] = {0};
        uint32_t readlen = 0;

        sp_num = 0 - sp_num;

        int32_t idx = 0;
        for (idx = 0; idx < sp_num; ++idx) {
            sprintf(new_filename, "%s%02u", input_filename, idx);
            new_fp = fopen(new_filename, "rb");
            if (!new_fp) {
                fprintf(stderr, "error when create file: %s\n", new_filename);
                goto errs;
            }

            printf("read from file: %s\n", new_filename);

            while ((readlen = fp_read_len(new_fp, buf, BUF_LEN)) != 0) {
                if (fp_write_len(fp, buf, readlen) < readlen) {
                    printf("error when read from %s\n", new_filename);
                    goto errs;
                }
            }

            fclose(new_fp);
        }

errs:
        fclose(fp);
    }

    return EXIT_SUCCESS;
}       /* -- end of function main -- */

