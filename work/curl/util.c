/************************************************
 *       Filename: util.c
 *    Description:
 *        Created: 2015-01-20 16:45
 *         Author: dengwei david@stratusee.com
 ************************************************/
#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "data_struct.h"
#include "common/cfg_parse.h"

static void show_help(void)
{
    fprintf(stdout, "USAGE: \n\t-q for request num"
                  "\n\t-a for agent num"
                  "\n\t-b for pipeline length, default is %u"
                  "\n\t-d for desc"
                  "\n\t-D for daemon mode, and output to daemon_out.log"
                  "\n\t-f for config file"
                  "\n\t-n for set thread num per cpu, default is %u"
                  "\n\t-o output file"
                  "\n\t-r for rampup, unit second"
                  "\n\t-s for https test"
                  "\n\t-t for testing time, unit second"
                  "\n\t-v for verbose debug, 0 for no debug, 1 for debug libcurl\n"
                  , DFT_PIPELINE_BATCH_LENGTH, THREADNUM_PER_CPU);
}

#define PRINT_MEM_INT(__stru, __memb)    fprintf(stdout, "  %s: %u\n", #__memb, (__stru)->__memb)
#define PRINT_MEM_LONG(__stru, __memb)    fprintf(stdout, "  %s: %lu\n", #__memb, (__stru)->__memb)
#define PRINT_MEM_STR(__stru, __memb)    \
do {    \
    if (strlen((__stru)->__memb) > 0) {  \
        fprintf(stdout, "  %s: %s\n", #__memb, (__stru)->__memb);    \
    }   \
} while (0)
void print_global_info(global_info_t *global_info)
{
    fprintf(stdout, "----------------------\n");
    time_t now = time(NULL);
    fprintf(stdout, "GLOBAL INFO -- %s", ctime(&now));
    PRINT_MEM_STR(global_info, desc);
    PRINT_MEM_LONG(global_info, work_num);
    PRINT_MEM_INT(global_info, cpu_num);
    PRINT_MEM_INT(global_info, thread_num);
    PRINT_MEM_LONG(global_info, agent_num);
    PRINT_MEM_LONG(global_info, agent_num_per_thread);
    PRINT_MEM_LONG(global_info, pipline_batch_length);
    PRINT_MEM_INT(global_info, agent_num_per_sec_thread);
    PRINT_MEM_INT(global_info, rampup);
    PRINT_MEM_STR(global_info, url[HTTP_TYPE]);
    PRINT_MEM_STR(global_info, url[HTTPS_TYPE]);
    PRINT_MEM_INT(global_info, is_https);
    PRINT_MEM_INT(global_info, debug_level);
    fprintf(stdout, "----------------------\n");
}

int32_t parse_cmd(int argc, char **argv, global_info_t *global_info)
{
    int opt;
    bool is_daemon = false;
    int32_t threadnum_per_cpu = THREADNUM_PER_CPU;
    global_info->pipline_batch_length = DFT_PIPELINE_BATCH_LENGTH;

    while ((opt = getopt(argc, argv, "a:b:d:f:n:o:q:r:t:v:hsD")) != -1) {
        switch (opt) {
            case 'a':
                global_info->agent_num = atoi(optarg);
                break;

            case 'b':
                global_info->pipline_batch_length = strtoul(optarg, NULL, 0);
                if (global_info->pipline_batch_length == 0 || global_info->pipline_batch_length == 1) {
                    global_info->pipline_batch_length = NO_PIPELINE_BATCH_LENGTH;
                }
                break;

            case 'd':
                fix_strcpy_s(global_info->desc, optarg);
                break;

            case 'D':
                is_daemon = true;
                break;

            case 'f':
                {
                    cfg_item_t cfg_list[] = {
                        {FIX_CFGNAME("http"), FIX_CFG_STRDATA(global_info->url[HTTP_TYPE]), dft_cfg_set_string, "http url"},
                        {FIX_CFGNAME("https"), FIX_CFG_STRDATA(global_info->url[HTTPS_TYPE]), dft_cfg_set_string, "https url"},
                    };

                    int32_t ret_val = parse_cfglist_linux_fmt(optarg, cfg_list);
                    if (ret_val < 0) {
                        fprintf(stdout, "error when parse config file: %s\n", optarg);
                        return -1;
                    }

                    break;
                }

            case 'n':
                threadnum_per_cpu = atoi(optarg);
                break;

            case 'o':
                fix_strcpy_s(global_info->output_filename, optarg);
                break;

            case 'q':
                global_info->work_num = atoi(optarg);
                break;

            case 'r':
                global_info->rampup = (uint16_t)atoi(optarg);
                break;

            case 's':
                global_info->is_https = true;
                break;

            case 'v':
                global_info->debug_level = atoi(optarg);
                if (global_info->debug_level > DEBUG_LIBCURL) {
                    fprintf(stdout, "error when parse debug level[%s], value must be less than %u\n", optarg, DEBUG_LIBCURL + 1);
                    show_help();
                    return -1;
                }
                break;

            case 't':
                global_info->during_time = atoi(optarg);
                break;

            case 'h':
            default:
                show_help();
                return 1;
        }
    }

    if ((global_info->work_num == 0 && global_info->during_time == 0) || global_info->agent_num == 0) {
        show_help();
        return -1;
    }

    if (global_info->url[global_info->is_https][0] == '\0') {
        fprintf(stdout, "%s url is not defined in config file\n", global_info->is_https ? "https" : "http");
        return -1;
    }

    if (is_daemon) {
        daemon(1, 0);
        FILE *fout = fopen("daemon_out.log", "a");
        if (fout) {
            stdout = fout;
            stderr = fout;
        }
    }

    global_info->thread_num = global_info->cpu_num * threadnum_per_cpu;
    if (global_info->agent_num < global_info->thread_num) {
        global_info->thread_num = global_info->agent_num;
    }

    global_info->agent_num_per_thread = (global_info->agent_num / global_info->thread_num);
    if ((global_info->agent_num % global_info->thread_num) != 0) {
        global_info->agent_num_per_thread++;
    }
    global_info->agent_num_per_thread = MEM_ALIGN_SIZE(global_info->agent_num_per_thread, 4) * global_info->pipline_batch_length;

    if (global_info->rampup > 0) {
        global_info->agent_num_per_sec_thread = global_info->agent_num_per_thread / global_info->rampup;
        if ((global_info->agent_num_per_thread % global_info->rampup) != 0) {
            global_info->agent_num_per_sec_thread++;
        }
    } else {
        global_info->agent_num_per_sec_thread = global_info->agent_num_per_thread;
    }

    print_global_info(global_info);

    return 0;
}
