/************************************************
 *       Filename: vs_file_op.h
 *    Description: 
 *        Created:
 *         Author: dengwei
 ************************************************/
#ifndef _FILE_OP_H
#define _FILE_OP_H
#include "common/cplus_define.h"
CPLUS_BEGIN

#include "common/types.h"

int file_length(const char *path);
int create_path(char *path, int32_t path_len, const int mode);
bool isdir(const char *path);
bool isfile(const char *path);
int check_space(const char *dir, uint32_t *free_space);
int make_tmpfile(const char *dir, const char *prefix, char *path, const int maxlen);
int make_tmpdir(const char *dir, const char *prefix, char *path, const int maxlen);
int check_file_size(char *file_path, int max_size);
int copy_file_mode(char *src, char *dst, char *mode);
#define copy_file(src, dst) \
    copy_file_mode(src, dst, "w")
#define append_file(src, dst) \
    copy_file_mode(src, dst, "a")
int delete_file(const char *path);

#define fd_write_fix(fd, fix_str)   \
    write(fd, (const void *)(fix_str), sizeof(fix_str) - 1)

#define fd_write(fd, strings)   \
    write(fd, (const void *)(strings), strlen(strings))

#define fp_write_len(fp, strings, __len)    \
    fwrite((void *)(strings), 1, (size_t)(__len), fp)

#define fp_read_len(fp, strings, __maxlen)    \
    fread((void *)(strings), 1, (size_t)(__maxlen), fp)

#define fp_write(fp, strings)   \
    fp_write_len(fp, strings, strlen(strings))

#define fp_read(fp, strings)   \
    fp_read_len(fp, strings, (sizeof(strings)/sizeof(strings[0])))

#define fp_write_fix(fp, fix_str)   \
    fp_write_len(fp, fix_str, (sizeof(fix_str) - 1))

CPLUS_END
#endif    // _FILE_OP_H

