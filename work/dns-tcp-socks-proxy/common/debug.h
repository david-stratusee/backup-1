/************************************************
 *       Filename: debug.h
 *    Description: 
 *        Created: 2014-10-23 10:22
 *         Author: dengwei david@stratusee.com
 ************************************************/

#ifndef _DEBUG_H
#define _DEBUG_H

#include "common/types.h"
#include "common/misc.h"
#include "common/bitops.h"
#include "common/optimize.h"
#include "common/lock_def.h"

/********************************************
 * DEBUG LEVEL
 *
 * NOTE: LEVEL_ID(MAX) should be 2^n, else it 
 *       should impact DBG_TEST macro
 ********************************************/
#define LEVEL_ID(level) DL_##level
typedef enum _DEBUG_LEVEL_EN {
    LEVEL_ID(DEBUG),   /* debug message */
    LEVEL_ID(MSG),     /* normal message, but should be record */
    LEVEL_ID(ERROR),   /* error message, must be record */
    LEVEL_ID(FATAL),
    LEVEL_ID(MAX)
} DEBUG_LEVEL_EN;   /* -- end of DEBUG_LEVEL_EN -- */
#ifdef DL_FIRST_LOAD
const fix_str_size_t g_debug_level_name[LEVEL_ID(MAX)] = {
    [LEVEL_ID(DEBUG)] = FIX_STR_SIZE_PAIR("DBG"),
    [LEVEL_ID(MSG)] = FIX_STR_SIZE_PAIR("MSG"),
    [LEVEL_ID(ERROR)] = FIX_STR_SIZE_PAIR("ERR"),
    [LEVEL_ID(FATAL)] = FIX_STR_SIZE_PAIR("FATAL")
};
#else
extern const fix_str_size_t g_debug_level_name[LEVEL_ID(MAX)];
#endif

/********************************************
 * DEBUG SYS
 ********************************************/
#define SYS_ID(sys)   DS_##sys

/********************************************
 * DEBUG MODULE
 ********************************************/
#define MODULE_ID(sys, module)     DSM_##sys##_##module
#define MODULE_PAIR(sys, module)   [MODULE_ID(sys, module)] = FIX_STR_SIZE_PAIR(#module)

/********************************************
 * CMLIB
 ********************************************/
#define CM_MODULE_PAIR(sys, module)   [MODULE_ID(sys, module)] = FIX_STR_SIZE_PAIR("CM_"#module)
typedef enum _DSM_CMLIB_EN {
    MODULE_ID(CMLIB, MATCH),
    MODULE_ID(CMLIB, MEMPOOL),
    MODULE_ID(CMLIB, DEBUG),
    MODULE_ID(CMLIB, LOCK),
    MODULE_ID(CMLIB, SLAB),
    MODULE_ID(CMLIB, TICKS),
    MODULE_ID(CMLIB, ELSE),
    MODULE_ID(CMLIB, MAX)
} DSM_CMLIB_EN;   /* -- end of DSM_MODULE_CMLIB_EN -- */
#define DEFINE_DSM_CMLIB_ARRAY      \
    CM_MODULE_PAIR(CMLIB, MATCH),    \
    CM_MODULE_PAIR(CMLIB, MEMPOOL), \
    CM_MODULE_PAIR(CMLIB, DEBUG),   \
    CM_MODULE_PAIR(CMLIB, LOCK),   \
    CM_MODULE_PAIR(CMLIB, SLAB),   \
    CM_MODULE_PAIR(CMLIB, TICKS),  \
    CM_MODULE_PAIR(CMLIB, ELSE)


/********************************************
 * SYS -> MODULE
 ********************************************/
typedef struct _ds_module_info_t {
    const fix_str_size_t name;
    const fix_str_size_t *module_info;
    int32_t module_max_id;
} ds_module_info_t;   /* -- end of ds_module_info_t -- */

/********************************************
 * total control
 ********************************************/
typedef uint64_t dsm_switch_type;
typedef int32_t (*debug_extra_ft)(char *buffer, int32_t maxlen);

#define MAX_DBG_FILENAME_LEN 64
typedef struct _debug_ctl_t {
    dsm_switch_type *dbg_switch;        /* dbg_switch should be [SYS_ID][LEVEL_ID] */
    debug_extra_ft extra_func;

    uint32_t sys_id;   
    uint16_t backup_num;
    uint16_t resv_16;

    char dbg_filename[MAX_DBG_FILENAME_LEN];
    FILE *debug_fp;
    pthread_key_t thread_dbg_buffer_key;

    mutexlock_t mutexlock_file;
} debug_ctl_t;   /* -- end of debug_ctl_t -- */

#define SL_SWITCH_INDEX(sys_id, level) \
    (((sys_id) << 2) + level)

#define GET_SL_SWITCH(sys_id, level)   \
    (g_dbg_ctl->dbg_switch)[SL_SWITCH_INDEX(sys_id, level)]

#define TEST_SL_SWITCH(sys_id, level, module_id) \
    test_bit(GET_SL_SWITCH(sys_id, level), module_id)

#define SL_SWITCH_LEN(sys_max)  ((sys_max) << 2)

/********************************************
 * switch config
 ********************************************/
#define DSM_ALL_SET ((dsm_switch_type)(-1))

typedef struct _debug_config_t {
    int32_t sys_id;
    dsm_switch_type *dbg_switch;
    const char *log_dir;
    debug_extra_ft extra_func;
} debug_config_t;   /* -- end of debug_config_t -- */
int32_t debug_local_init(debug_config_t *dbg_config, const ds_module_info_t *sys_info, debug_ctl_t *_dbg_ctl, bool need_backup);
void debug_switch_init(int32_t sys_max);
void debug_process_destroy(void);
void debug_local_destroy(void);
void debug_backup_file(char *path);
void debug_print(const char *fnc, int32_t lno, int32_t dbg_level, bool need_lock, const char *fmt, ...) PRINTF_FORMAT_ARG(5, 6);
int32_t debug_set(const ds_module_info_t *ds_module_info_in, int32_t sys_max, char *dbg_filename);
int32_t debug_show(const ds_module_info_t *ds_module_info_in, int32_t sys_max, char *dbg_filename);
FILE *export_debug_file_handle(void);
void end_export_debug_file_handle(void);

/********************************************
 * define MACRO
 ********************************************/
extern debug_ctl_t *g_dbg_ctl;
#define DBG_TEST(sys, module, dbg_level) \
    (unlikely(g_dbg_ctl && TEST_SL_SWITCH(g_dbg_ctl->sys_id, LEVEL_ID(dbg_level), MODULE_ID(sys, module))))

#define DBG_PRINT(sys, module, dbg_level, fmt, args...)                             \
do {                                                                                \
    if (DBG_TEST(sys, module, dbg_level)) {                                         \
        debug_print(__func__, __LINE__, LEVEL_ID(dbg_level), true, " "fmt, ##args); \
    } else if (unlikely(!g_dbg_ctl && LEVEL_ID(dbg_level) != LEVEL_ID(DEBUG))) {    \
        printf(fmt, ##args);                                                        \
    }                                                                               \
} while (0)

#define DBG_PRINT_NOLOCK(sys, module, dbg_level, fmt, args...)                       \
do {                                                                                 \
    if (DBG_TEST(sys, module, dbg_level)) {                                          \
        debug_print(__func__, __LINE__, LEVEL_ID(dbg_level), false, " "fmt, ##args); \
    } else if (unlikely(!g_dbg_ctl && LEVEL_ID(dbg_level) != LEVEL_ID(DEBUG))) {     \
        printf(fmt, ##args);                                                         \
    }                                                                                \
} while (0)

/********************************************
 * FOR COMMON_LIB PRINT
 ********************************************/
#define CMLIB_PRINT(sys, module, level, fmt, args...)  \
    DBG_PRINT(sys, module, level, fmt, ##args)

#define CMLIB_DBG(module, fmt, args...)     DBG_PRINT(CMLIB, module, DEBUG, fmt, ##args)
#define CMLIB_MSG(module, fmt, args...)     DBG_PRINT(CMLIB, module, MSG, fmt, ##args)
#define CMLIB_ERR(module, fmt, args...)     DBG_PRINT(CMLIB, module, ERROR, fmt, ##args)
#define CMLIB_FATAL(module, fmt, args...)   DBG_PRINT(CMLIB, module, FATAL, fmt, ##args)

#endif   /* -- #ifndef _DEBUG_H -- */

