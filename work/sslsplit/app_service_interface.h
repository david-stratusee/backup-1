/************************************************
 *       Filename: app_service_interface.h
 *    Description: 
 *        Created: 2015-05-01 04:17
 *         Author: dengwei
 ************************************************/
#ifndef _APP_SERVICE_INTERFACE_H
#define _APP_SERVICE_INTERFACE_H

#include "common/types.h"
#include "common/logbuf.h"

#define MAX_PORTS_NUM 8
/* Description: app service interface */
typedef struct _app_service_t {
    uint32_t mod_id;
    uint16_t mod_port_list[14];
    const char *mod_name;

    int (*mod_init_service)(void);
    int (*mod_post_init_service)(void);
    void (*mod_uninit_service)(void);

    void *(*mod_init_sess)(char *src_str, char *dst_str);
    void (*mod_release_sess)(void *sess_handle);
    int (*mod_data_handle)(void *sess_handle, logbuf_t *buffer, bool is_request);

    void (*mod_record_core_info)(int signum, int intr_type, FILE *fp);

    bool init_ok;
} app_service_t;   /* -- end of app_service_t -- */

typedef enum _APP_SERVICE_ID_EN {
    ASI_HTTP,
    ASI_MAIL,
    ASI_MAX
} APP_SERVICE_ID_EN;   /* -- end of APP_SERVICE_ID_EN -- */

#ifdef APP_SONAME_DEFINE
const char *app_soname[ASI_MAX] = {
    [ASI_HTTP] = "http_srv",
    [ASI_MAIL] = "mail_srv"
};
#else
const char *app_soname[ASI_MAX];
#endif

#endif   /* -- #ifndef _APP_SERVICE_INTERFACE_H -- */
