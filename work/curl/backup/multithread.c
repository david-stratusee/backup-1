#include <stdio.h>
#include <pthread.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "timestamp.h"
#include "setsignal.h"
#include "ssl_lock.h"

#define mutexlock_init(lock)    pthread_mutex_init(&(lock), NULL)
#define mutex_trylock(lock)     pthread_mutex_trylock(&(lock))
#define mutex_lock(lock)        pthread_mutex_lock(&(lock))
#define mutex_unlock(lock)      pthread_mutex_unlock(&(lock))
#define mutexlock_destroy(lock) pthread_mutex_destroy(&(lock))

typedef enum _WORK_STATUS_EN {
    STAT_INIT,
    STAT_DONE
} WORK_STATUS_EN;   /* -- end of WORK_STATUS_EN -- */

/* Description: work info */
typedef struct _work_info_t {
    char *url;
    int status;
    int idx;
    int data_len;
} work_info_t;   /* -- end of work_info_t -- */

/* Description: global info */
typedef struct _global_info_t {
    pthread_mutex_t rmtx;
    pthread_mutex_t wmtx;
    work_info_t **work_list;
    int read_work_idx;
    int write_work_idx;
    int work_num;
    int thread_num;
    int do_exit;
    int round;

    unsigned int error_num;
} global_info_t;   /* -- end of global_info_t -- */
global_info_t global_info;

/* Description: thread info */
typedef struct _thread_info_t {
    pthread_t tid;
    CURLM *multi_handle;
    CURL *curl;
    global_info_t *global_info;
    int idx;
} thread_info_t;   /* -- end of thread_info_t -- */

char *http_url = "http://192.168.101.102:8080/static/code.py";
//char *http_url = "http://192.168.10.31/assets/jquery.flot.js";
char *https_url = "https://192.168.10.31:3000/assets/jquery.flot.js";

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

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
    work_info_t *work_info = (work_info_t *)userp;
    size_t real_size = size * nmemb;
    work_info->data_len += real_size;
    return real_size;
}

#ifdef DEBUG
#define DUMP printf
#else
#define DUMP(...)
#endif

#define WAITMS(x)                               \
  struct timeval wait = { 0, (x) * 1000 };      \
  (void)select(0, NULL, NULL, NULL, &wait);

static void *pull_one_url(void *arg)
{
    thread_info_t *thread_info = (thread_info_t *)arg;
    global_info_t *global_info = thread_info->global_info;
    work_info_t *work_info = NULL;
    int still_running;
    int ignore_sig[] = {SIGINT, SIGTERM};
    pthread_set_ignore_sig(ignore_sig, sizeof(ignore_sig) / sizeof(int));
    curl_easy_setopt(thread_info->curl, CURLOPT_WRITEFUNCTION, write_data);
    int repeats = 0;

    while (global_info->read_work_idx < global_info->work_num && !(global_info->do_exit)) {
        repeats = 0;
        DUMP("[%u:%lu]enter, read_work_idx: %u\n", thread_info->idx, time(NULL), global_info->read_work_idx);
        mutex_lock(global_info->rmtx);
        work_info = global_info->work_list[global_info->read_work_idx];
        global_info->work_list[global_info->read_work_idx] = NULL;
        global_info->read_work_idx++;
        mutex_unlock(global_info->rmtx);
        DUMP("[%u:%lu]work %u, read_work_idx: %u\n", thread_info->idx, time(NULL), work_info->idx, global_info->read_work_idx);

        curl_easy_setopt(thread_info->curl, CURLOPT_WRITEDATA, work_info);
        curl_easy_setopt(thread_info->curl, CURLOPT_URL, work_info->url);

        int idx = 0;
        for (idx = 0; idx < global_info->round; ++idx) {
            curl_multi_add_handle(thread_info->multi_handle, thread_info->curl);
            curl_multi_perform(thread_info->multi_handle, &still_running);
            do {
                CURLMcode mc; /* curl_multi_wait() return code */
                int numfds;

                /* wait for activity, timeout or "nothing" */
                mc = curl_multi_wait(thread_info->multi_handle, NULL, 0, 1000, &numfds);

                if(mc != CURLM_OK)
                {
                    fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);
                    break;
                }

                /* 'numfds' being zero means either a timeout or no file descriptors to
                   wait for. Try timeout on first occurance, then assume no file
                   descriptors and no file descriptors to wait for means wait for 100
                   milliseconds. */
                if(!numfds) {
                    repeats++; /* count number of repeated zero numfds */
                    if(repeats > 1) {
                        WAITMS(100); /* sleep 100 milliseconds */
                    }
                }
                else
                    repeats = 0;

                curl_multi_perform(thread_info->multi_handle, &still_running);
            } while(still_running);
            curl_multi_remove_handle(thread_info->multi_handle, thread_info->curl);
        }

        DUMP("[%u:%lu]done %u, write_work_idx: %u\n", thread_info->idx, time(NULL), work_info->idx, global_info->write_work_idx);
        work_info->status = STAT_DONE;
        mutex_lock(global_info->wmtx);
        global_info->work_list[global_info->write_work_idx++] = work_info;
        mutex_unlock(global_info->wmtx);
        DUMP("[%u:%lu]out, write_work_idx: %u\n", thread_info->idx, time(NULL), global_info->write_work_idx);
    }

    return NULL;
}

static void _sig_int(int signum, siginfo_t *info, void *ptr)
{
    global_info.do_exit = 1;
    return;
}

static void show_help(void)
{
    printf("USAGE: \n\t-w for work_num\n\t-t for thread num\n\t-r for round\n\t-s for https test\n");
}

int main(int argc, char **argv)
{
    memset(&global_info, 0, sizeof(global_info_t));
    char *url = http_url;
    global_info.round = 1;

    int opt;
    while ((opt = getopt(argc, argv, "w:t:r:hs")) != -1) {
        switch (opt) {
            case 'w':
                global_info.work_num = atoi(optarg);
                break;

            case 't':
                global_info.thread_num = atoi(optarg);
                break;

            case 'r':
                global_info.round = atoi(optarg);
                break;

            case 's':
                url = https_url;
                break;

            case 'h':
            default:
                show_help();
                return EXIT_SUCCESS;
        }
    }

    if (global_info.work_num == 0 || global_info.thread_num == 0) {
        show_help();
        return EXIT_FAILURE;
    }

    setsignal(SIGINT, _sig_int);
    setsignal(SIGTERM, _sig_int);
    printf("get work num: %u, thread num: %u\n", global_info.work_num, global_info.thread_num);
    thread_info_t *thread_list = calloc(global_info.thread_num, sizeof(thread_info_t));

    if (thread_list == NULL) {
        printf("alloc threads error\n");
        return EXIT_FAILURE;
    }

    global_info.work_list = calloc(global_info.work_num, sizeof(work_info_t *));

    if (global_info.work_list == NULL) {
        printf("alloc work_list error\n");
        return EXIT_FAILURE;
    }

    int idx = 0;

    for (idx = 0; idx < global_info.work_num; ++idx) {
        global_info.work_list[idx] = calloc(1, sizeof(work_info_t));

        if (global_info.work_list[idx] == NULL) {
            printf("alloc work_list %u error\n", idx);
            return EXIT_FAILURE;
        }

        global_info.work_list[idx]->url = url;
        global_info.work_list[idx]->idx = idx;
    }

    /* Must initialize libcurl before any threads are started */
    curl_global_init(CURL_GLOBAL_ALL);
    init_locks();
    TS_INIT();
    mutexlock_init(global_info.rmtx);
    mutexlock_init(global_info.wmtx);
    TS_DECLARE(perf);

    for (idx = 0; idx < global_info.thread_num; ++idx) {
        thread_list[idx].global_info = &global_info;
        thread_list[idx].curl = curl_easy_init();
        thread_list[idx].multi_handle = curl_multi_init();

        if (thread_list[idx].curl == NULL || thread_list[idx].multi_handle == NULL) {
            printf("error when curl init\n");
            return EXIT_FAILURE;
        }

        //curl_easy_setopt(thread_list[idx].curl, CURLOPT_FORBID_REUSE, 1L);
        curl_easy_setopt(thread_list[idx].curl, CURLOPT_NOSIGNAL, 1L);

        if (url == https_url) {
            curl_easy_setopt(thread_list[idx].curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(thread_list[idx].curl, CURLOPT_SSL_VERIFYHOST, 0L);
        }

        thread_list[idx].idx = idx;
        set_share_handle(thread_list[idx].curl);
    }

    int error;
    TS_BEGIN(perf);

    for (idx = 0; idx < global_info.thread_num; ++idx) {
        error = pthread_create(&(thread_list[idx].tid),
                               NULL, /* default attributes please */
                               pull_one_url,
                               (void *) & (thread_list[idx]));

        if (0 != error) {
            fprintf(stderr, "Couldn't run thread number %d, errno %d\n", idx, error);
            return EXIT_FAILURE;
        }
    }

    /* now wait for all threads to terminate */
    for (idx = 0; idx < global_info.thread_num; idx++) {
        error = pthread_join(thread_list[idx].tid, NULL);
        //printf("[%u:%lu]Thread %d terminated\n", idx, time(NULL), idx);
    }

    TS_END(perf);

    for (idx = 0; idx < global_info.thread_num; idx++) {
        curl_easy_cleanup(thread_list[idx].curl);
        curl_multi_cleanup(thread_list[idx].multi_handle);
    }

    free(thread_list);
    unsigned long total_length = 0;

    for (idx = 0; idx < global_info.work_num; idx++) {
        if (global_info.work_list[idx]) {
            if (global_info.work_list[idx]->status == STAT_DONE) {
                total_length += global_info.work_list[idx]->data_len;
            }

            free(global_info.work_list[idx]);
        }
    }

    free(global_info.work_list);
    printf("RATE: worknum:%u total_length:%lu total_ms_diff:%lu, %.1fK\n",
           global_info.work_num, total_length, TS_MSDIFF(perf), (double)(total_length) / (double)(TS_MSDIFF(perf)));
    kill_locks();
    curl_global_cleanup();
    return EXIT_SUCCESS;
}

