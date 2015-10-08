/************************************************
 *       Filename: app_service.c
 *    Description:
 *        Created: 2015-05-01 03:30
 *         Author: dengwei
 ************************************************/
#define APP_SONAME_DEFINE

#include "common/types.h"
#include "common/str_def.h"
#include "common/umemory.h"
#include "app_service.h"
#include <dlfcn.h>

static int32_t load_service_module(app_service_opts_t *app_service_opts, const char *srv_name)
{
    char so_path[128];
    fix_snprintf(so_path, "/usr/local/holonet/lib/lib%s.so", srv_name);

    printf("get srv_name: %s, so_path: %s\n", srv_name, so_path);
    void *so_handle = dlopen(so_path, RTLD_NOW | RTLD_GLOBAL);
    if (PTR_NULL(so_handle)) {
        char *error_str = dlerror();
        fprintf(stderr, "error when open service file %s-%s: %s\n", so_path, srv_name, error_str);
        return -1;
    }

    void *srv_handle = dlsym(so_handle, srv_name);
    if (PTR_NULL(srv_handle)) {
        fprintf(stderr, "Not found symbol \"%s\" in library, unload it\n", srv_name);
        dlclose(so_handle);
        return -2;
    }

    app_service_opts->srv_so_handle = so_handle;
    app_service_opts->srv_handle = srv_handle;
    return 0;
}

int32_t load_all_service(app_service_opts_t *app_service_list)
{
    int32_t idx = 0;

    for (idx = 0; idx < ASI_MAX; ++idx) {
        memset(app_service_list + idx, 0, sizeof(app_service_opts_t));
        load_service_module(app_service_list + idx, app_soname[idx]);
    }

    return 0;
}

static void unload_service_module(app_service_opts_t *app_service_opts)
{
    if (app_service_opts->srv_handle && app_service_opts->srv_handle->mod_uninit_service) {
        app_service_opts->srv_handle->mod_uninit_service();
    }

    if (app_service_opts->srv_so_handle) {
        dlclose(app_service_opts->srv_so_handle);
        app_service_opts->srv_so_handle = NULL;
    }
}

void unload_all_service(app_service_opts_t *app_service_list)
{
    int32_t idx = 0;
    for (idx = 0; idx < ASI_MAX; ++idx) {
        unload_service_module(app_service_list + idx);
    }
}
