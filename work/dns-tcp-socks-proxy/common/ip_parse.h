/************************************************
 *       Filename: ip_parse.h
 *    Description: 
 *        Created: 2015-01-03 10:24
 *         Author: dengwei david@stratusee.com
 ************************************************/

#ifndef _IP_PARSE_H
#define _IP_PARSE_H

#include "common/types.h"
#include "common/string_s.h"
#include "endian.h"

#if __BYTE_ORDER == __BIG_ENDIAN
#define NIPQUAD(addr) \
    ((uint8_t *)&addr)[0], \
    ((uint8_t *)&addr)[1], \
    ((uint8_t *)&addr)[2], \
    ((uint8_t *)&addr)[3]
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define NIPQUAD(addr) \
    ((uint8_t *)&addr)[3], \
    ((uint8_t *)&addr)[2], \
    ((uint8_t *)&addr)[1], \
    ((uint8_t *)&addr)[0]
#endif
#define NIPQUAD_FMT "%u.%u.%u.%u"

#define MAX_IP_SPLIT_NUM 4
static inline uint32_t ip_parse(char *ipstr, int32_t ipstr_len)
{
    split_node_t split_list[MAX_IP_SPLIT_NUM] = {SPLIT_NODE_INIT_VALUE};
    int32_t split_num = split_pos(ipstr, ipstr_len, '.', split_list, MAX_IP_SPLIT_NUM, true);

    if (split_num < MAX_IP_SPLIT_NUM) {
        CMLIB_ERR(ELSE, "split ip[%.*s] error, split_num:%d, is not %u\n", ipstr_len, ipstr, split_num, MAX_IP_SPLIT_NUM);
        return 0;
    }

    int32_t idx = 0;
    uint32_t ip_output = 0;
    for (idx = 0; idx < split_num ; ++idx) {
        ip_output = ((ip_output << 8) | (uint8_t)strtoul_s(ipstr + split_list[idx].begin_offset, split_list[idx].part_len, NULL, 10));
    }

    CMLIB_DBG(ELSE, "get ip: 0x%x, str: %.*s\n", ip_output, ipstr_len, ipstr);
    return ip_output;
}

#endif   /* -- #ifndef _IP_PARSE_H -- */
