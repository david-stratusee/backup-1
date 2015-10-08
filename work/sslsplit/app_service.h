/************************************************
 *       Filename: app_service.h
 *    Description: 
 *        Created: 2015-05-01 04:08
 *         Author: dengwei
 ************************************************/
#ifndef _APP_SERVICE_H
#define _APP_SERVICE_H

#include "common/types.h"
#include "app_service_interface.h"

/* Description: so handle saved info */
typedef struct _app_service_opts_t {
	void *srv_so_handle;
	app_service_t *srv_handle;
} app_service_opts_t;   /* -- end of app_service_opts_t -- */

int32_t load_all_service(app_service_opts_t *app_service_list);
void unload_all_service(app_service_opts_t *app_service_list);

#define FUNC_RET_ALL_SERVICE(__app_service_list, __do_func)                                                                 \
({                                                                                                                          \
    int32_t idx = 0;                                                                                                        \
    int32_t __do_func##_ret_val = 0;                                                                                        \
    for (idx = 0; idx < ASI_MAX; ++idx) {                                                                                   \
        if ((__app_service_list)[idx].srv_handle && (__app_service_list)[idx].srv_handle->__do_func) {                      \
            __do_func##_ret_val = (__app_service_list)[idx].srv_handle->__do_func();                                        \
            if (unlikely(__do_func##_ret_val < 0)) {                                                                        \
                fprintf(stderr, "Error when %s service: %s\n", #__do_func, (__app_service_list)[idx].srv_handle->mod_name); \
                break;                                                                                                      \
            }                                                                                                               \
        }                                                                                                                   \
    }                                                                                                                       \
    __do_func##_ret_val;                                                                                                    \
})

#define FUNC_ALL_SERVICE(__app_service_list, __do_func)                                                \
do {                                                                                                   \
    int32_t idx = 0;                                                                                   \
    for (idx = 0; idx < ASI_MAX; ++idx) {                                                              \
        if ((__app_service_list)[idx].srv_handle && (__app_service_list)[idx].srv_handle->__do_func) { \
            (__app_service_list)[idx].srv_handle->__do_func();                                         \
        }                                                                                              \
    }                                                                                                  \
} while (0)

#endif   /* -- #ifndef _APP_SERVICE_H -- */

