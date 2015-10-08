/************************************************
 *       Filename: so_handle.c
 *    Description: so handle
 *
 *        Created:
 ************************************************/
#include "common/types.h"
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include "common/so_handle.h"
#include "common/optimize.h"

static inline uint32_t DJB_hash(unsigned char *str)
{
    uint32_t hash = 5381;
    int i = 0;
    int len = strlen((char *) str);

    for (i = 0; i < len; str++, i++) {
        hash = ((hash << 5) + hash) + (*str);
    }

    return hash;
}

static int search_name(uint32_t *so_name_array, int array_idx, char *so_name)
{
    uint32_t so_name_id = DJB_hash((unsigned char *) so_name);

    int idx = 0;

    for (idx = 0; idx < array_idx; idx++) {
        if (so_name_array[idx] == so_name_id) {
            break;
        }
    }

    so_name_array[array_idx] = so_name_id;
    if (idx == array_idx) {
        goto nofind;
    }

    return idx;

      nofind:
    return -1;
}

#define NAME_VALID(name) \
    (name != NULL && (name)[0] != '\0')

int common_load_so(so_func_desc * sfd)
{
    if (unlikely(sfd == NULL)) {
        return 0;
    }

    uint32_t so_name_array[64] = { 0 };
    so_func_desc *sfdp = sfd;
    int idx = 0;
    int same_idx = 0;
    char *error = NULL;

    for (idx = 0; (sfdp = sfd + idx) != NULL && NAME_VALID(sfdp->func_name); idx++) {

        sfdp->handle = NULL;
        if (!NAME_VALID(sfdp->so_name)) {
            continue;
        }

        same_idx = search_name(so_name_array, idx, sfdp->so_name);
        if (same_idx >= 0) {
            sfdp->handle = sfd[same_idx].handle;
        } else {
            sfdp->handle = dlopen(sfdp->so_name, RTLD_LAZY);
            if (sfdp->handle == NULL) {
                fprintf(stderr, "dlopen %s error, %s\n", sfdp->so_name, strerror(errno));
                goto err;
            }
            printf("dlopen %s ok\n", sfdp->so_name);
        }

        dlerror();
//              *(sfdp->func) = dlsym(sfdp->handle, sfdp->func_name);
        sfdp->func = dlsym(sfdp->handle, sfdp->func_name);
        if ((error = dlerror()) != NULL) {
            fprintf(stderr, "dlsym %s error:%s\n", sfdp->func_name, error);
            goto err;
        }
    }

    return 0;

      err:
    sfdp++;
    while (NAME_VALID(sfdp->func_name)) {
        sfdp->handle = NULL;
        sfdp++;
    }

    common_close_so(sfd);

    return (0 - (idx + 1));    // +1ÊÇÎªÁË·ÀÖ¹³öÏÖ0
}

static int search_handle(long *handle_array, int array_idx, void *handle)
{
    int idx = 0;

    for (idx = 0; idx < array_idx; idx++) {
        if (handle_array[idx] == (long) handle) {
            break;
        }
    }

    handle_array[array_idx] = (long) handle;
    if (idx == array_idx) {
        goto nofind;
    }

    return idx;

      nofind:
    return -1;
}

int common_close_so(so_func_desc * sfd)
{
    if (unlikely(sfd == NULL)) {
        return 0;
    }

    long so_handle_array[64] = { 0 };
    so_func_desc *sfdp = sfd;
    int idx = 0;

    while (NAME_VALID(sfdp->func_name)) {
        if (sfdp->handle != NULL && search_handle(so_handle_array, idx, sfdp->handle) < 0) {
            dlclose(sfdp->handle);
        }
        sfdp->handle = NULL;

        sfdp++;
        idx++;
    }

    return 0;
}

void *load_func_byname(so_func_desc * sfd, char *name)
{
    so_func_desc *sfdp = sfd;

    while (NAME_VALID(sfdp->func_name)) {
        if (strcmp(sfdp->func_name, name) == 0) {
            goto find;
        }

        sfdp++;
    }

    return NULL;
      find:
    return (sfdp->func);
}

void *load_func_byid(so_func_desc * sfd, int idx)
{
    return ((NAME_VALID(sfd[idx].func_name)) ? sfd[idx].func : NULL);
}
