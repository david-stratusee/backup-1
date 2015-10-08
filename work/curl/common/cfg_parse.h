/************************************************
 *       Filename: cfg_parse.h
 *    Description: 
 *        Created: 2014-12-13 14:42
 *         Author: dengwei david@stratusee.com
 ************************************************/

#ifndef _CFG_PARSE_H
#define _CFG_PARSE_H

#include "common/types.h"
#include "common/string_s.h"

typedef int32_t (*cfg_action_ft)(const char *name, char *string, split_node_t *argv, int32_t argv_num, void *setdata, uint32_t setdata_len);

/* Description: config item define */
#define MAX_SPLIT_NUM 3
typedef struct _cfg_item_t {
    const char *name;
    unsigned long name_len;

    void *setdata;
    unsigned long setdata_len;

    cfg_action_ft action;
    const char *desc;
} cfg_item_t;       /* -- end of cfg_item_t -- */
#define FIX_CFGNAME(__name)    (const char *)(__name), (unsigned long)FLEN(__name)
#define FIX_CFG_STRDATA(__setdata) (void *)(__setdata), (unsigned long)(sizeof(__setdata))
#define FIX_CFG_INTDATA(__setdata) (void *)(&(__setdata)), (unsigned long)(sizeof(__setdata))

int32_t parse_cfg_linux_fmt(const char *cfg_filename, cfg_item_t *cfg_list, uint32_t cfg_num);
#define parse_cfglist_linux_fmt(__cfg_filename, __cfg_list) \
    parse_cfg_linux_fmt(__cfg_filename, __cfg_list, (sizeof(__cfg_list) / sizeof(cfg_item_t)));

static inline int32_t dft_cfg_set_string(const char ARG_UNUSED *name, char *string, split_node_t *argv, int32_t ARG_UNUSED argv_num, void *setdata, uint32_t setdata_len)
{
    return strncpy_s(setdata, setdata_len, string + argv[0].begin_offset, argv[0].part_len);
}

static inline int32_t dft_cfg_set_int(const char ARG_UNUSED *name, char *string, split_node_t *argv, int32_t ARG_UNUSED argv_num, void *setdata, uint32_t setdata_len)
{
    unsigned long result = strtoul_s(string + argv[0].begin_offset, argv[0].part_len, NULL, 0);
    switch (setdata_len) {
        case sizeof(uint32_t):
            *((uint32_t *)setdata) = (uint32_t)result;
            break;
        case sizeof(uint16_t):
            *((uint16_t *)setdata) = (uint16_t)result;
            break;
        case sizeof(uint8_t):
            *((uint8_t *)setdata) = (uint8_t)result;
        default:
            break;
    }

    return setdata_len;
}

#endif   /* -- #ifndef _CFG_PARSE_H -- */
