/************************************************
 *       Filename: vs_so_handle.h
 *    Description: so handle
 *        Created:
 *         Author: dengwei
 ************************************************/
#ifndef _SO_HANDLE_H
#define _SO_HANDLE_H
#include "common/cplus_define.h"
CPLUS_BEGIN

#include <dlfcn.h>
#define declare_so_handle(so_name) \
    void *so_name##_so_handle = NULL

static inline void *in_load_so_func(void **handle, char *so_name, char *func_name)
{
    if (*handle == NULL) {
        *handle = dlopen(so_name, RTLD_LAZY);
        if (*handle == NULL) {
            //                      printf("load %s error\n", so_name);
            return NULL;
        }
    }

    dlerror();
    //      char *error = NULL;
    void *func_point = dlsym(*handle, func_name);

    //      if ((error = dlerror()) != NULL) {
    if (dlerror() != NULL) {
        //              printf("load func_name(%s) from so_name(%s) error: %s\n", func_name, so_name, error);
        dlclose(*handle);
        *handle = NULL;
        return NULL;
    }

    return func_point;
}
#define load_so_func(so_name, func_name) \
    in_load_so_func(&so_name##_so_handle, so_name, func_name)

static inline void in_close_so_handle(void **handle)
{
    if (*handle) {
        dlclose(*handle);
        *handle = NULL;
    }

    return;
}
#define close_so_handle(so_name) \
    in_close_so_handle(&so_name##_so_handle)

typedef struct _so_func_desc {
    char *so_name;        // so name
    char *func_name;    // function name in so

    void *func;        // func
    void *handle;
} so_func_desc;

int common_load_so(so_func_desc * sfd);
int common_close_so(so_func_desc * sfd);

void *load_func_byname(so_func_desc * sfd, char *name);
void *load_func_byid(so_func_desc * sfd, int idx);

CPLUS_END
#endif                // _SO_HANDLE_H
