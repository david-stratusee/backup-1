/************************************************
 *       Filename: file_op.c
 *    Description: file operation
 *
 *        Created:
 *         Author: david dengwei
 ************************************************/
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

#ifdef MACOS
#include <sys/param.h>
#include <sys/mount.h>
#else
#include <sys/vfs.h>
#endif

#include <time.h>
#include <fcntl.h>

#include "common/types.h"
#include "common/misc.h"
#include "common/optimize.h"
#include "common/umemory.h"
#include "common/string_s.h"
#include "common/file_op.h"

#define fileop_error(fmt, args...) fprintf(stderr, fmt, ##args)
#define fileop_debug(fmt, args...)

/*
 * get regular file length
 */
int file_length(const char *path)
{
    struct stat stats;

    if (unlikely(NULL == path)) {
        fileop_error("file_length: arg error\n");
        return -1;
    }

    ZERO_STRUCT(stats);
    if (stat(path, &stats) != 0) {
        fileop_error("stat %s: %s", path, strerror(errno));
        return -2;
    }

    if (!S_ISREG(stats.st_mode)) {
        fileop_error("%s isn`t regular file\n", path);
        return -3;
    }

    return stats.st_size;
}

int create_path(char *path, int32_t path_len, const int mode)
{
    split_node_t split_list[32] = {SPLIT_NODE_INIT_VALUE};
    int32_t split_num = split_pos(path, path_len, '/', split_list, 32, false);
    char full_path[path_len + 1];
    uint32_t fullpath_len = 0, max_fullpathlen = sizeof(full_path);

    int32_t idx = 0;
    int32_t mkdir_ret = 0;
    for (idx = 0; idx < split_num ; ++idx) {
        append_fmtmsg(full_path, max_fullpathlen, fullpath_len, "/%.*s", split_list[idx].part_len, path + split_list[idx].begin_offset);
        if (isdir(full_path)) {
            continue;
        }

        mkdir_ret = mkdir(full_path, mode);
        if (unlikely(mkdir_ret < 0)) {
            return mkdir_ret;
        }
    }

    return 0;
}

/* If PATH is an existing directory or symbolic link to a directory,
   return nonzero, else 0.  */
bool isdir(const char *path)
{
    if (PTR_NULL(path)) {
        fileop_error("isdir: arg error\n");
        return false;
    }

    struct stat stats;
    return (stat(path, &stats) == 0 && S_ISDIR(stats.st_mode));
}

bool isfile(const char *path)
{
    if (PTR_NULL(path)) {
        fileop_error("isdir: arg error\n");
        return false;
    }

    struct stat stats;
    return (stat(path, &stats) == 0 && S_ISREG(stats.st_mode));
}

int check_space(const char *dir, uint32_t *free_space)
{
    struct statfs sfs;

    if (unlikely(NULL == dir || NULL == free_space)) {
        fileop_error("check_space: arg error\n");
        return -1;
    }

    ZERO_STRUCT(sfs);
    if (statfs(dir, &sfs) != 0) {
        fileop_error("statfs: %s", strerror(errno));
        return -1;
    }

    *free_space = sfs.f_bfree * sfs.f_bsize;
    return 0;
}

int make_tmpfile(const char *dir, const char *prefix, char *path, const int maxlen)
{
    if (unlikely(access(dir, W_OK) < 0)) {
        mkdir(dir, 0777);
    }

    snprintf(path, maxlen, "%s/.%stmpXXXXXX", dir, prefix);

    int fd = mkstemp(path);

    if (fd >= 0) {
        close(fd);
    } else {
        fileop_error("mkstemp error");
        srandom(time(NULL));
        snprintf(path, maxlen, "%s/.%stmp%lX", dir, prefix, (random() & 0xFFFFFF));
        fd = open(path, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
        if (fd < 0) {
            fileop_error("open path %s error", path);
            goto err;
        }
    }

    return (strlen(path));

      err:
    return -1;
}

int make_tmpdir(const char *dir, const char *prefix, char *path, const int maxlen)
{
    if (unlikely(access(dir, W_OK) < 0)) {
        mkdir(dir, 0700);
    }

    srandom(time(NULL));
    snprintf(path, maxlen, "%s/.%stmp%lX/", dir, prefix, (random() & 0xFFFFFF));
    if (mkdir(path, 0700) < 0) {
        return -1;
    } else {
        return (strlen(path));
    }
}

int check_file_size(char *file_path, int max_size)
{
    if (unlikely(NULL == file_path || '\0' == file_path[0] || max_size <= 0)) {
        fileop_error("argument invalid !!\n");
        goto err;
    }

    struct stat file_stat;
    ZERO_STRUCT(file_stat);

    if (stat(file_path, &file_stat) < 0) {
        fileop_error("stat file %s error,errno:%d(%s)\n", file_path, errno, strerror(errno));
        goto err;
    }

    /* file size not out of bound */
    if (file_stat.st_size < max_size) {
        goto lt;
    }

    /* truncate the file to zero */
    if (-1 == truncate(file_path, 0)) {
        fileop_error("truncate file %s error,errno:%d(%s)\n", file_path, errno, strerror(errno));
        goto err;
    }
    // truncate file
    return 1;
lt:
    return 0;
err:
    return -1;
}

/*
 * copy regular file
 */
int copy_file_mode(char *src, char *dst, char *mode)
{
    FILE *rfp = NULL, *wfp = NULL;

    if (unlikely(NULL == src || NULL == dst)) {
        return -1;
    }

    if (unlikely(strcasecmp(src, dst) == 0)) {
        return 0;
    }

    struct stat stats;
    ZERO_STRUCT(stats);
    if (unlikely(stat(src, &stats) != 0)) {
        fileop_error("stat %s: %s", src, strerror(errno));
        goto err;
    }
    if (!S_ISREG(stats.st_mode)) {
        fileop_error("%s isn`t regular file\n", src);
        goto err;
    }
    char *parent_dir_end = strrchr(dst, (int)('/'));
    if (parent_dir_end) {
        if (create_path(dst, (int)(parent_dir_end - dst), 0755) < 0) {
            fileop_error("copy_file: creat_path failed\n");
            goto err;
        }
    }

    rfp = fopen(src, "r");
    if (PTR_NULL(rfp)) {
        fileop_error("fopen %s: %s\n", src, strerror(errno));
        goto err;
    }

    wfp = fopen(dst, mode);
    if (PTR_NULL(wfp)) {
        fileop_error("fopen %s: %s\n", dst, strerror(errno));
        goto err;
    }

#define BUF_LEN 4096
    char buf[BUF_LEN] = { 0 };
    uint32_t readlen = 0;

    while ((readlen = fp_read_len(rfp, buf, BUF_LEN)) != 0) {
        if (fp_write_len(wfp, buf, readlen) < readlen) {
            goto err;
        }
    }

    if (!feof(rfp)) {
        fileop_error("fread %s error\n", src);
        goto err;
    }

    fclose(rfp);
    rfp = NULL;
    // dst file closing must be successful.
    if (fclose(wfp) != 0) {
        fileop_error("fclose %s error\n", dst);
        wfp = NULL;
        goto err;
    }

    if (chmod(dst, stats.st_mode) != 0) {
        fileop_error("chmod %s: %s\n", dst, strerror(errno));
    }

    return 0;

err:
    if (rfp != NULL) {
        fclose(rfp);
    }

    if (wfp != NULL) {
        fclose(wfp);
    }
    // leave nothing if copy failed.
    unlink(dst);
    return -1;
}

/*
 * delete regular file
 */
int delete_file(const char *path)
{
    struct stat stats;

    if (unlikely(NULL == path)) {
        fileop_error("delete_file: arg error\n");
        return -1;
    }

    ZERO_STRUCT(stats);
    if (stat(path, &stats) != 0) {
        fileop_error("stat %s: %s", path, strerror(errno));
        return -1;
    }

    if (S_ISREG(stats.st_mode)) {
        if (unlink(path) < 0) {
            fileop_error("unlink %s: %s", path, strerror(errno));
            return -1;
        }
    } else if (S_ISDIR(stats.st_mode)) {
        if (rmdir(path) < 0) {
            fileop_error("rmdir %s: %s", path, strerror(errno));
            return -1;
        }
    } else {
        fileop_error("%s isn`t regular file or directory\n", path);
    }

    return 0;
}
