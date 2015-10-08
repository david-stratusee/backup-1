/************************************************
 *       Filename: cfg_parse.c
 *    Description:
 *        Created: 2014-12-13 12:41
 *         Author: dengwei david@stratusee.com
 ************************************************/

#include "common/types.h"
#include "common/cfg_parse.h"
#include <errno.h>

#define VAR_NAME_INDEX  0
static void _parse_cfg_line(char *pline, cfg_item_t *cfg_list, uint32_t cfg_num)
{
    split_node_t split_list[MAX_SPLIT_NUM];
    ZERO_STRUCT(split_list);

    int32_t split_num = split_sp_pos(pline, strlen(pline), split_list, MAX_SPLIT_NUM);
    if (split_num <= 1) {
        return;
    }

    uint32_t idx = 0;
    for (idx = 0; idx < cfg_num; ++idx) {
        if (case_strequal(pline + split_list[VAR_NAME_INDEX].begin_offset,
                    split_list[VAR_NAME_INDEX].part_len,
                    cfg_list[idx].name, cfg_list[idx].name_len)) {
            if (cfg_list[idx].action) {
                cfg_list[idx].action(cfg_list[idx].name, pline, split_list + 1, split_num - 1,
                                    cfg_list[idx].setdata, (uint32_t)(cfg_list[idx].setdata_len));
            }

            break;
        }
    }
}

int32_t parse_cfg_linux_fmt(const char *cfg_filename, cfg_item_t *cfg_list, uint32_t cfg_num)
{
#define CFG_LINE_LEN 1024
    char readline[CFG_LINE_LEN] = {0};
    char *pline = readline;
    char *pline_end = NULL;

    FILE *fp_cfg = fopen(cfg_filename, "r");
    if (unlikely(fp_cfg == NULL)) {
        fprintf(stderr, "couldn't open file '%s'; %s\n", cfg_filename, strerror(errno));
        return -1;
    }

    /********************************************
     * parse file
     ********************************************/
    while (memset(readline, 0, CFG_LINE_LEN) && fgets(readline, CFG_LINE_LEN, fp_cfg)) {
        pline = readline;
        while (*pline == ' ' || *pline == '\t' || *pline == '\r' || *pline == '\n') {
            pline++;
        }

        if (pline[0] == '\0' || pline[0] == '#') {
            continue;
        }

        pline_end = pline + strlen(pline) - 1;
        while ((*pline_end == '\r' || *pline_end == '\n') && pline_end >= pline) {
            pline_end--;
        }
        *(pline_end + 1) = '\0';

        /***************************************************
         * TODO: use pline here, filtered # and empty lines
         ***************************************************/
        _parse_cfg_line(pline, cfg_list, cfg_num);
    }

    fclose(fp_cfg);
    return 0;
}
