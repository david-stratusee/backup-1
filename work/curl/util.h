/************************************************
 *       Filename: util.h
 *    Description: 
 *        Created: 2015-01-20 16:54
 *         Author: dengwei david@stratusee.com
 ************************************************/


#ifndef _UTIL_H
#define _UTIL_H

#include "types.h"
#include "data_struct.h"
int32_t parse_cmd(int argc, char **argv, global_info_t *global_info);
void print_global_info(global_info_t *global_info);

#endif   /* -- #ifndef _UTIL_H -- */

