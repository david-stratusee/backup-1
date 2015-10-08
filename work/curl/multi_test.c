/************************************************
 *       Filename: multi_test.c
 *    Description:
 *        Created: 2015-01-20 16:28
 *         Author: dengwei david@stratusee.com
 ************************************************/
#define PS_DEFINED

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "common/types.h"
#include "common/timestamp.h"
#include "common/setsignal.h"
#include "common/atomic_def.h"
#include "common/ssl_lock.h"
#include "common/optimize.h"
#include "common/misc.h"
#include "common/string_s.h"
#include "common/file_op.h"
#include "data_struct.h"
#include "util.h"

global_info_t global_info;
#ifdef DEBUG
#define DUMP(fmt, args...) \
do {    \
    fprintf(stdout, "[DBG:%s-%u-%lu]"fmt, __func__, __LINE__, time(NULL), ##args);  \
    fflush(stdout); \
} while (0)
#else
#define DUMP(...) EMPTY_STATE
#endif

#define PRINT(fmt, args...) \
do {    \
    fprintf(stdout, fmt, ##args);  \
    fflush(stdout); \
} while (0)

static void _sig_int(int signum, siginfo_t *info, void *ptr)
{
    global_info.do_exit = G_FORCE_EXIT;
    return;
}

static inline size_t write_data(void *buffer ARG_UNUSED, size_t size, size_t nmemb, void *userp ARG_UNUSED)
{
    //work_info_t *work_info = (work_info_t *)userp;
    //size_t real_size = size * nmemb;
    //work_info->data_len += real_size;
    return size * nmemb;
}

static void _itox(unsigned long value, char *value_str, int32_t max_len)
{
    int32_t idx = 0;
    char *ptr = value_str + max_len - 1;
    uint8_t ch = 0;

    for (idx = 0; idx < max_len && value > 0; ++idx) {
        ch = (value & 0xF);
        value >>= 4;

        if (ch < 10) {
            *(ptr--) = ch + '0';
        } else {
            *(ptr--) = ch - 10 + 'A';
        }
    }

    while (ptr >= value_str) {
        *(ptr--) = '0';
    }
}

struct curl_slist *no_cache_list = NULL;
static void add_no_cache_list_header()
{
    //no_cache_list = curl_slist_append(no_cache_list, "Cache-control: no-cache");
    //no_cache_list = curl_slist_append(no_cache_list, "Pragma: no-cache");
    no_cache_list = curl_slist_append(no_cache_list, "Cache-control: no-store");
    no_cache_list = curl_slist_append(no_cache_list, "Connection: Keep-Alive");
    no_cache_list = curl_slist_append(no_cache_list, "Proxy-Connection: Keep-Alive");
}

static void set_share_handle(CURL *curl_handle)
{
    static CURLSH *share_handle = NULL;

    if (!share_handle) {
        share_handle = curl_share_init();
        curl_share_setopt(share_handle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
    }

    curl_easy_setopt(curl_handle, CURLOPT_SHARE, share_handle);
    curl_easy_setopt(curl_handle, CURLOPT_DNS_CACHE_TIMEOUT, 60 * 5);
}

static inline unsigned long _get_one_work(global_info_t *global_info, thread_info_t *thread_info, const char *func, const int line)
{
    if (HAVE_WORK_AVAILABLE(global_info)) {
        unsigned long read_work_idx = atomic_inc_and_return(global_info->read_work_idx);
        thread_info->work_num++;
        DUMP("[%s-%u]thread %u add one work, work_num is %lu\n", func, line, thread_info->idx, thread_info->work_num);
        return read_work_idx;
    } else {
        return 0UL;
    }
}
#define get_one_work(global_info, thread_info, read_work_idx)  \
    (read_work_idx = _get_one_work(global_info, thread_info, __func__, __LINE__))

static CURL *curl_handle_init(global_info_t *global_info, work_info_t *work_info)
{
    CURL *curl = curl_easy_init();
    if (curl == NULL) {
        return NULL;
    }

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, work_info);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_2) AppleWebKit/600.3.18 (KHTML, like Gecko) Version/8.0.3 Safari/600.3.18");
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    if (global_info->debug_level == DEBUG_LIBCURL) {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    }

    if (global_info->is_https) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }

    set_share_handle(curl);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, no_cache_list);
    return curl;
}

static inline thread_info_t *thread_init(global_info_t *global_info)
{
    thread_info_t *thread_list = calloc(global_info->thread_num, sizeof(thread_info_t));
    if (thread_list == NULL) {
        PRINT("alloc threads error\n");
        return NULL;
    }

    add_no_cache_list_header();

    uint32_t idx = 0, jdx = 0, agent_num_per_sec_thread = 0;
    work_info_t *work_info = NULL;
    thread_info_t *thread_info;
    unsigned long read_work_idx = 0UL;

    for (idx = 0; idx < global_info->thread_num; ++idx) {
        thread_info = thread_list + idx;
        thread_info->global_info = global_info;
        thread_info->min_latency = (unsigned int)(-1);
        thread_info->url_buffer = umalloc_empty(strlen(global_info->url[global_info->is_https]) + FLEN("?id=") + 16 + 1);
        if (PTR_NULL(thread_info->url_buffer)) {
            PRINT("error when malloc url_buffer in thread_info\n");
            return NULL;
        }

        thread_info->multi_handle = curl_multi_init();
        thread_info->work_list = calloc(global_info->agent_num_per_thread * global_info->pipline_batch_length, sizeof(work_info_t));
        if (thread_info->multi_handle == NULL || thread_info->work_list == NULL) {
            PRINT("error when curl init\n");
            return NULL;
        }

        if (global_info->pipline_batch_length != NO_PIPELINE_BATCH_LENGTH) {
            curl_multi_setopt(thread_info->multi_handle, CURLMOPT_PIPELINING, 1L);
            curl_multi_setopt(thread_info->multi_handle, CURLMOPT_MAX_TOTAL_CONNECTIONS, (global_info->agent_num_per_thread / global_info->pipline_batch_length));
            curl_multi_setopt(thread_info->multi_handle, CURLMOPT_MAXCONNECTS, (global_info->agent_num_per_thread / global_info->pipline_batch_length));
            curl_multi_setopt(thread_info->multi_handle, CURLMOPT_MAX_PIPELINE_LENGTH, global_info->pipline_batch_length);
        }

        thread_info->url_buffer_len = (uint16_t)sprintf(thread_info->url_buffer, "%s?id=", global_info->url[global_info->is_https]);

        agent_num_per_sec_thread = MIN(global_info->agent_num_per_sec_thread, global_info->agent_num_per_thread);
        for (jdx = 0; jdx < global_info->agent_num_per_thread; ++jdx) {
            work_info = thread_info->work_list + jdx;

            work_info->curl = curl_handle_init(global_info, work_info);
            if (work_info->curl == NULL) {
                return NULL;
            }

            if (jdx < agent_num_per_sec_thread && get_one_work(global_info, thread_info, read_work_idx) > 0) {
                work_info->data_len = 0;

                _itox(read_work_idx, thread_info->url_buffer + thread_info->url_buffer_len, 16);
                curl_easy_setopt(work_info->curl, CURLOPT_URL, thread_info->url_buffer);
                curl_multi_add_handle(thread_info->multi_handle, work_info->curl);
                DUMP("[%u]add handle %p to multi_handle %p\n", idx, work_info->curl, thread_info->multi_handle);

                thread_info->alloc_agent_num = jdx + 1;
                thread_info->last_alloc_time = time(NULL);
            }
        }

        thread_info->idx = idx;
    }

    return thread_list;
}

static void thread_destroy(global_info_t *global_info, thread_info_t *thread_list)
{
    uint32_t idx = 0, jdx = 0;

    DUMP("clear thread info\n");

    for (idx = 0; idx < global_info->thread_num; idx++) {
        for (jdx = 0; jdx < global_info->agent_num_per_thread; ++jdx) {
            curl_easy_cleanup(thread_list[idx].work_list[jdx].curl);
        }
        curl_multi_cleanup(thread_list[idx].multi_handle);
        ufree_setnull(thread_list[idx].url_buffer);
    }

    free(thread_list);
    curl_slist_free_all(no_cache_list);
}

static inline void global_init(void)
{
    curl_global_init(CURL_GLOBAL_ALL);
    init_locks();
    TS_INIT();
}

static inline void global_destroy(void)
{
    kill_locks();
    curl_global_cleanup();
}

static inline int32_t global_info_init(global_info_t *global_info)
{
    memset(global_info, 0, sizeof(global_info_t));

    int cpu_num = get_cpu_num();
    if (cpu_num <= 0) {
        fprintf(stderr, "error when get cpu num, set to default 4\n");
        cpu_num = 4;
    }

    global_info->cpu_num = cpu_num;
    return 0;
}

static int32_t check_rampup_agent(CURLM *multi_handle, global_info_t *global_info, thread_info_t *thread_info)
{
    time_t ts;
    time(&ts);

    if (ts != thread_info->last_alloc_time) {
        work_info_t *work_info = NULL;
        int num = 0;
        int32_t idx = 0;
        unsigned long read_work_idx = 0UL;

        int32_t agent_num_per_sec_thread = MIN(global_info->agent_num_per_sec_thread, global_info->agent_num_per_thread - thread_info->alloc_agent_num);
        for (idx = 0; idx < agent_num_per_sec_thread; ++idx) {
            work_info = thread_info->work_list + thread_info->alloc_agent_num;

            if (get_one_work(global_info, thread_info, read_work_idx) > 0) {
                work_info->data_len = 0;

                _itox(read_work_idx, thread_info->url_buffer + thread_info->url_buffer_len, 16);
                curl_easy_setopt(work_info->curl, CURLOPT_URL, thread_info->url_buffer);
                curl_multi_add_handle(multi_handle, work_info->curl);

                thread_info->alloc_agent_num++;
                thread_info->last_alloc_time = ts;
                DUMP("[%u]add handle %p to multi_handle %p\n", thread_info->idx, work_info->curl, multi_handle);
            } else {
                break;
            }

            num++;
        }

        return num;
    } else {
        return 0;
    }
}

#define fill_thread_error(thread_info, fmt, args...) \
    do {    \
        (thread_info)->error_num++;   \
        if (unlikely((thread_info)->sample_error[0] == '\0')) {   \
            fix_snprintf((thread_info)->sample_error, fmt, ##args); \
        }   \
    } while (0)

#define fill_thread_fixerr(thread_info, desc) \
    do {    \
        (thread_info)->error_num++;   \
        if (unlikely((thread_info)->sample_error[0] == '\0')) {   \
            fix_strcpy_s((thread_info)->sample_error, desc);   \
        }   \
    } while (0)

static int32_t check_available(CURLM *multi_handle, global_info_t *global_info, thread_info_t *thread_info)
{
    int msgs_left;
    CURLMsg *msg;
    CURL *easy_handle = NULL;
    int num = 0;

    if (unlikely(thread_info->alloc_agent_num < global_info->agent_num_per_thread)) {
        num += check_rampup_agent(multi_handle, global_info, thread_info);
    }

    long response_code = 200;
    double total_time = 0.0f, total_length = 0.0f;
    unsigned int latency = 0;
    unsigned long read_work_idx = 0UL;
    char *last_url = NULL;
    bool need_add_handle = true;

    while ((msg = curl_multi_info_read(multi_handle, &msgs_left))) {
        if (CURLMSG_DONE == msg->msg) {
            easy_handle = msg->easy_handle;
            DUMP("easy_handle %p is done, code: %u-%s, msg_left: %d\n",
                    easy_handle, msg->data.result, curl_easy_strerror(msg->data.result),
                    msgs_left);
            DUMP("  %u:S[%u]-R[%u]-D[%lu-%lu]\n",
                    thread_info->idx, thread_info->work_done, thread_info->still_running,
                    thread_info->work_num, thread_info->succ_num);

            if (easy_handle) {
                if (curl_easy_getinfo(easy_handle, CURLINFO_EFFECTIVE_URL, &last_url) != CURLE_OK || last_url == NULL) {
                    last_url = NULL;
                }
            }

            need_add_handle = true;
            if (likely(msg->data.result == CURLE_OK && easy_handle)) {
                /*  TODO: curl_easy_getinfo */
                if (likely(curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &response_code) == CURLE_OK
                        && (response_code >= 200 && response_code < 400))) {
                    curl_easy_getinfo(easy_handle, CURLINFO_SIZE_DOWNLOAD, &total_length);
                    curl_easy_getinfo(easy_handle, CURLINFO_TOTAL_TIME, &(total_time));

                    latency = (unsigned int)(unsigned long)(total_time * 1000);
                    thread_info->total_latency += latency;

                    if (latency > thread_info->max_latency) {
                        thread_info->max_latency = latency;
                    }
                    if (latency > 0 && latency < thread_info->min_latency) {
                        thread_info->min_latency = latency;
                    }

                    thread_info->total_data_len += total_length;
                    thread_info->succ_num++;
                } else {
                    fill_thread_error(thread_info, "get response_code %ld", response_code);
                }

                curl_multi_remove_handle(multi_handle, easy_handle);
            } else {
                fill_thread_fixerr(thread_info, curl_easy_strerror(msg->data.result));
                if (easy_handle) {
#if 0
                    if (global_info->pipline_batch_length == NO_PIPELINE_BATCH_LENGTH) {
                        need_add_handle = false;
                    }
#endif

                    curl_multi_remove_handle(multi_handle, easy_handle);
                } else {
                    need_add_handle = false;
                }
            }

            if (need_add_handle && get_one_work(global_info, thread_info, read_work_idx) > 0) {
                if (PTR_NOT_NULL(last_url)) {
                    _itox(read_work_idx, last_url + thread_info->url_buffer_len, 16);
                    DUMP("[%u]last_url: %s\n", thread_info->idx, last_url);
                } else {
                    _itox(read_work_idx, thread_info->url_buffer + thread_info->url_buffer_len, 16);
                    curl_easy_setopt(easy_handle, CURLOPT_URL, thread_info->url_buffer);
                }
                curl_multi_add_handle(multi_handle, easy_handle);

                num++;
            }
        } else {
            PRINT("-------------------- [easy_handle-%p] not OK\n", msg->easy_handle);
        }
    }

    return num;
}

static void print_thread_info(thread_info_t *thread_list, global_info_t *global_info)
{
    int idx = 0;
    static unsigned int print_thread_count = 0;
    PRINT("------------------\n");
    PRINT("[%u-%lu]do_exit:%u, threads info:\n", print_thread_count++, time(NULL), global_info->do_exit);
    for (idx = 0; idx < global_info->thread_num; idx++) {
        if (thread_list[idx].error_num == 0) {
            PRINT("  %u:S[%u]-R[%u-%d]-D[%lu-%lu]-[%s]\n",
                    idx, thread_list[idx].work_done, thread_list[idx].still_running, thread_list[idx].numfds,
                    thread_list[idx].work_num, thread_list[idx].succ_num,
                    ps_desc[thread_list[idx].proc_state]);
        } else {
            PRINT("  %u:S[%u]-R[%u-%d]-D[%lu-%lu]-E[%lu]-ES[%s]-[%s]\n",
                    idx, thread_list[idx].work_done, thread_list[idx].still_running, thread_list[idx].numfds,
                    thread_list[idx].work_num, thread_list[idx].succ_num,
                    thread_list[idx].error_num, thread_list[idx].sample_error,
                    ps_desc[thread_list[idx].proc_state]);
            if (global_info->sample_error[0] == '\0') {
                fix_strcpy_s(global_info->sample_error, thread_list[idx].sample_error);
            }
        }
    }
    PRINT("------------------\n");
}

static inline CURLMcode curl_multi_perform_cont(CURLM *multi_handle, int *running_handles, global_info_t *global_info)
{
    CURLMcode ret_code;
    while ((ret_code = curl_multi_perform(multi_handle, running_handles)) == CURLM_CALL_MULTI_PERFORM) {
        DUMP("get still_running: %u\n", *running_handles);
        if (unlikely(global_info->do_exit == G_FORCE_EXIT)) {
            return CURLM_BAD_HANDLE;
        }
    }

    return ((*running_handles >= 0) ? ret_code : CURLM_BAD_HANDLE);
}

static void *pull_one_url(void *arg)
{
    thread_info_t *thread_info = (thread_info_t *)arg;
    global_info_t *global_info = thread_info->global_info;

    int ignore_sig[] = {SIGINT, SIGTERM};
    pthread_set_ignore_sig(ignore_sig, sizeof(ignore_sig) / sizeof(int));

    CURLMcode mc; /* curl_multi_wait() return code */
    int calc_num = 0;

    if (curl_multi_perform_cont(thread_info->multi_handle, &(thread_info->still_running), global_info) != CURLM_OK || thread_info->still_running < 0) {
        fprintf(stderr, "[%u-%lu-%u]error when call curl_multi_perform\n", thread_info->idx, time(NULL), __LINE__);
        return NULL;
    }

    DUMP("get still_running: %u, multi_handle: %p\n", thread_info->still_running, thread_info->multi_handle);
    do {
        /* wait for activity, timeout or "nothing" */
        thread_info->numfds = 0;

        thread_info->proc_state = PS_WAIT;

        mc = curl_multi_wait(thread_info->multi_handle, NULL, 0, 100, &(thread_info->numfds));
        if (mc != CURLM_OK) {
            fprintf(stderr, "[%u]curl_multi_fdset() failed, code %d.\n", thread_info->idx, mc);
            break;
        }

        DUMP("[%u]get still_running: %u, multi_handle: %p, numfds: %u, calc_num: %u, alloc_agent_num: %u\n",
                thread_info->idx, thread_info->still_running, thread_info->multi_handle, thread_info->numfds, calc_num, thread_info->alloc_agent_num);

        thread_info->proc_state = PS_CHECK_AVAILABLE;

        if (thread_info->numfds || check_available(thread_info->multi_handle, global_info, thread_info) > 0 || calc_num >= 30) {
            calc_num = 0;

            thread_info->proc_state = PS_PERFORM;
            if (curl_multi_perform_cont(thread_info->multi_handle, &(thread_info->still_running), global_info) != CURLM_OK) {
                fprintf(stderr, "[%u-%lu-%u]error when call curl_multi_perform\n", thread_info->idx, time(NULL), __LINE__);
                break;
            }

#if 1
            thread_info->proc_state = PS_CHECK_AVAILABLE_AGAIN;
            if (check_available(thread_info->multi_handle, global_info, thread_info) > 0) {
                thread_info->proc_state = PS_PERFORM_AGAIN;
                if (curl_multi_perform_cont(thread_info->multi_handle, &(thread_info->still_running), global_info) != CURLM_OK) {
                    fprintf(stderr, "[%u-%lu-%u]error when call curl_multi_perform\n", thread_info->idx, time(NULL), __LINE__);
                    break;
                }
            }
#endif
        } else {
            calc_num++;
        }
    } while ((thread_info->still_running > 0 || HAVE_WORK_AVAILABLE(global_info)) && (global_info->do_exit != G_FORCE_EXIT));

    DUMP("[%u]last check avail\n", thread_info->idx);

#ifdef DEBUG
    if (thread_info->error_num == 0) {
        PRINT("  %u:S[%u]-R[%u]-D[%lu-%lu]\n",
                thread_info->idx, thread_info->work_done, thread_info->still_running, thread_info->work_num, thread_info->succ_num);
    } else {
        PRINT("  %u:S[%u]-R[%u]-D[%lu-%lu]-E[%lu]-ES[%s]\n",
                thread_info->idx, thread_info->work_done, thread_info->still_running, thread_info->work_num, thread_info->succ_num,
                thread_info->error_num, thread_info->sample_error);
    }
#endif

    check_available(thread_info->multi_handle, global_info, thread_info);
    thread_info->work_done = TSE_DONE;
    return NULL;
}

static int32_t start_thread_list(thread_info_t *thread_list, global_info_t *global_info)
{
    int error;
    int idx = 0;

    for (idx = 0; idx < global_info->thread_num; ++idx) {
        error = pthread_create(&(thread_list[idx].tid),
                               NULL, /* default attributes please */
                               pull_one_url,
                               (void *) & (thread_list[idx]));
        if (0 != error) {
            fprintf(stderr, "Couldn't run thread number %d, errno %d\n", idx, error);
            return error;
        }

        PRINT("[%lu]Thread %u start\n", time(NULL), idx);
    }

    return 0;
}

#define PRINT_ROUND 5
static int32_t check_thread_end(thread_info_t *thread_list, global_info_t *global_info)
{
    int idx = 0;
    time_t end_time = 0, now_time = 0, force_endtime = 0;

    if (global_info->during_time > 0) {
        time(&now_time);
        end_time = now_time + global_info->during_time;
        force_endtime = end_time + (global_info->during_time >> 1);
    }

    int finish_num;
    int check_num = 0;
    uint8_t get_exit = global_info->do_exit;
    do {
        finish_num = 0;
        for (idx = 0; idx < global_info->thread_num; idx++) {
            if (thread_list[idx].work_done >= TSE_DONE) {
                if (thread_list[idx].work_done == TSE_DONE) {
                    pthread_join(thread_list[idx].tid, NULL);
                    PRINT("[%lu]Thread %d terminated\n", time(NULL), idx);

                    thread_list[idx].work_done = TSE_VERIFY;
                }

                finish_num++;
            }
        }

        if (finish_num < global_info->thread_num) {
            sec_sleep(1);

            if (++check_num >= PRINT_ROUND) {
                print_thread_info(thread_list, global_info);
                if (get_exit == G_FORCE_EXIT) {
                    return -1;
                }

                check_num = 0;
                get_exit = global_info->do_exit;
            }
        } else {
            print_thread_info(thread_list, global_info);
            break;
        }

        if (global_info->during_time > 0) {
            time(&now_time);
            if (now_time >= end_time) {
                if (global_info->do_exit == G_RUNNING) {
                    global_info->do_exit = G_FORCE_EXIT;
                } else if (now_time >= force_endtime) {
                    global_info->do_exit = G_FORCE_EXIT;
                }
            }
        }
    } while (finish_num < global_info->thread_num);

    return 0;
}

static inline char *get_url_file(char *full_path)
{
    char *last_url = strrchr(full_path, '/');
    if (last_url) {
        last_url++;
    } else {
        last_url = full_path;
    }

    return last_url;
}

static void calc_stat(global_info_t *global_info, thread_info_t *thread_list, unsigned long msdiff)
{
    double total_length = 0.0;
    unsigned long total_time = 0;
    unsigned int max_latency = 0, min_latency = (unsigned int)(-1);
    int idx = 0;
    unsigned long suc_num = 0, error_num = 0;

    char *last_url = get_url_file(global_info->url[global_info->is_https]);
    for (idx = 0; idx < global_info->thread_num; idx++) {
        total_length += thread_list[idx].total_data_len;
        suc_num += thread_list[idx].succ_num;
        error_num += thread_list[idx].error_num;

        total_time += thread_list[idx].total_latency;
        if (thread_list[idx].max_latency > max_latency) {
            max_latency = thread_list[idx].max_latency;
        }
        if (thread_list[idx].min_latency > 0 && thread_list[idx].min_latency < min_latency) {
            min_latency = thread_list[idx].min_latency;
        }
    }

    PRINT("----------------------\n");
    time_t now = time(NULL);
    PRINT("RESULT: \"%s\" \"%s\" -- %s", global_info->desc, last_url, ctime(&now));
    PRINT("%16s : %lu\n", "request num", global_info->read_work_idx);
    PRINT("%16s : %lu\n", "error num", error_num);
    PRINT("%16s : %lu\n", "succ num", suc_num);
    PRINT("%16s : %.1f\n", "total length", total_length);
    PRINT("%16s : %lu\n", "total time(ms)", msdiff);
    PRINT("%16s : %.1fKb/s-%.1fMb/s\n", "throughput", (total_length * 125) / (msdiff * 16), (total_length * 125) / (msdiff * 16 * 1024));
    PRINT("%16s : %lu/s\n", "request rate", (suc_num * 1000) / msdiff);
    if (suc_num > 0) {
        PRINT("%16s : %lums[max:%ums, min:%ums]\n", "latency", total_time / suc_num, max_latency, min_latency);
    }
    PRINT("----------------------\n");

    if (global_info->output_filename[0]) {
        bool file_exist = isfile(global_info->output_filename);
        FILE *fp = fopen(global_info->output_filename, "a");
        if (fp) {
            if (!file_exist) {
                fprintf(fp,
                        "\"%s\"," "\"%s\"," "\"%s\"," "\"%s\","
                        "\"%s\"," "\"%s\"," "\"%s\"," "\"%s\","
                        "\"%s\"," "\"%s\"," "\"%s\"," "\"%s\","
                        "\"%s\"," "\"%s\"," "\"%s\"," "\"%s\","
                        "\"%s\"," "\"%s\"\n",
                        "desc", "url", "agentN", "pipeline",
                        "threadN", "rampup", "reqN", "sucN",
                        "Tlen", "Tms", "perf[Kb]", "perf[Mb]",
                        "req[N/s]", "latency(a)", "latency(x)", "latency(n)",
                        "errN", "estr"
                       );
            }

            fprintf(fp,
                    "\"%s\"," "\"%s\"," "\"%lu\"," "\"%lu\","
                    "\"%u\"," "\"%u\"," "\"%lu\"," "\"%lu\","
                    "\"%.1f\"," "\"%lu\"," "\"%.1f\"," "\"%.1f\","
                    "\"%lu\"," "\"%lu\"," "\"%u\"," "\"%u\","
                    "\"%lu\"," "\"%s\"\n" ,
                    global_info->desc, last_url, global_info->agent_num, global_info->pipline_batch_length,
                    global_info->thread_num / global_info->cpu_num, global_info->rampup, global_info->read_work_idx, suc_num,
                    total_length, msdiff, (total_length * 125) / (msdiff * 16), (total_length * 125) / (msdiff * 16 * 1024),
                    (suc_num * 1000) / msdiff, (suc_num > 0 ? total_time / suc_num : 0), max_latency, min_latency,
                    error_num, global_info->sample_error
                   );
            fclose(fp);
        }
    }
}

/************************************************
 *         Name: main
 *  Description: main function
 *     Argument:
 *       Return:
 ************************************************/
int main(int argc, char *argv[])
{
    TS_DECLARE(perf);

    global_init();
    if (global_info_init(&global_info) < 0) {
        return EXIT_FAILURE;
    }

    setsignal(SIGINT, _sig_int);
    setsignal(SIGTERM, _sig_int);
    signal(SIGPIPE, SIG_IGN);

    if (parse_cmd(argc, argv, &global_info) != 0) {
        return EXIT_FAILURE;
    }

    thread_info_t *thread_list = thread_init(&global_info);
    if (thread_list == NULL) {
        PRINT("error when init thread\n");
        return EXIT_FAILURE;
    }

    TS_BEGIN(perf);
    if (start_thread_list(thread_list, &global_info) != 0) {
        PRINT("error when start thread\n");
        return EXIT_FAILURE;
    }
    check_thread_end(thread_list, &global_info);
    TS_END(perf);

    calc_stat(&global_info, thread_list, TS_MSDIFF(perf));

    thread_destroy(&global_info, thread_list);
    global_destroy();
    return EXIT_SUCCESS;
}       /* -- end of function main -- */
