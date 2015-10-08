/*
 *  UDP-TCP SOCKS DNS Tunnel
 *  (C) 2012 jtRIPper
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#include "common/types.h"
#include "common/misc.h"
#include "common/cmlib_lock.h"
#include "adt/ac_bm.h"
#include "adt/hash.h"
#include "common/string_s.h"
#include "common/timestamp.h"
#include "adt/pool_list.h"
#include "adt/hash_index_func.h"

#include <pthread.h>

#ifdef DEBUG
#define DUMP(fmt, args...) \
do {    \
    if (LOG_FILE) { \
        fprintf(LOG_FILE, "[%s:%u]"fmt, __func__, __LINE__, ##args);    \
        fflush(LOG_FILE);   \
    }   \
} while (0)
#else
#define DUMP(...)
#endif

#define LOG(fmt, args...) \
do {    \
    if (LOG_FILE) { \
        fprintf(LOG_FILE, "[%s:%u]"fmt, __func__, __LINE__, ##args);    \
        fflush(LOG_FILE);   \
    }   \
} while (0)


//DUMP("go here\n")

/********************************************
 * signal
 ********************************************/
void (*setsignal(int signum, void (*sighandler) (int, siginfo_t *, void *))) (int) {
    struct sigaction action;
    struct sigaction old;

    memset(&action, 0, sizeof(action));
    sigemptyset(&action.sa_mask);
    action.sa_sigaction = sighandler;

    action.sa_flags = SA_SIGINFO;
    if (signum == SIGALRM) {
#ifdef SA_INTERRUPT
        action.sa_flags |= SA_INTERRUPT;
#endif
    } else {
#ifdef SA_RESTART
        action.sa_flags |= SA_RESTART;
#endif
    }

    if (sigaction(signum, &action, &old) < 0) {
        printf("setsignal %d err!\n", signum);
        return (SIG_ERR);
    }

    return old.sa_handler;
}

typedef enum _WORK_NODE_STATUS_EN {
    WNSE_NOTHING,
    WNSE_PREPARE,
    WNSE_DEAL,
    WNSE_MAX
} WORK_NODE_STATUS_EN;   /*  -- end of WORK_NODE_STATUS_EN -- */

int   SOCKS_PORT  = 8099;
char *SOCKS_ADDR  = { "127.0.0.1" };
int   LISTEN_PORT = 53;
char *LISTEN_ADDR = { "0.0.0.0" };
int   WORK_THREAD_NUM = 8;

FILE *LOG_FILE = NULL;
char *SYS_RESOLVCONF = "/tmp/resolv.conf";
char *RESOLVCONF = "/usr/local/etc/dns_proxy_resolv.conf";
char *PROXY_LIST = "proxy.list";
char *LOGFILE = "/dev/null";
int NUM_DNS = 0;
in_addr_t *dns_servers;
in_addr_t *udp_dns_servers;
int NUM_UDP_DNS = 0;
acbm_pattern_tree *proxy_list_tree = NULL;
hash_t *cache_hash = NULL;
#define MAX_CACHE_NUM 1024

typedef struct _cache_item_t {
     uint32_t len;
     uint32_t insert_time;
     char data[0];
} cache_item_t;   /* -- end of cache_item_t -- */
time_t OVERTIME = 3600;    /* 1 hour */

#define BUFFER_SIZE 1024
typedef struct {
    char buffer[BUFFER_SIZE];   /* head(2 byte)+buffer */
    unsigned short length;  /* real length */
    unsigned char status;
    unsigned char resv_8;
    socklen_t client_socklen;
    struct sockaddr_in dns_client;
} response;

#define RESPONSE_LIST_NUM 128
static response response_list[RESPONSE_LIST_NUM];
int udp_sock;
unsigned int   reader     = 0,
               pre_reader = 0,
               writer     = 0;
bool do_exit = false;
mutexlock_t mxlock;
#define GET_LOG_OFFSET(reader)   \
    ((uint32_t)(reader) & (RESPONSE_LIST_NUM - 1))

/* thread info */
typedef struct _thread_info_t {
    pthread_t handle;
    response *resp_info;
    int idx;
    unsigned short cur_udp_dns;
    unsigned short cur_sock_dns;
} thread_info_t;   /* -- end of thread_info_t -- */
thread_info_t *thread_list = NULL;

void error(char *e)
{
    perror(e);
    exit(1);
}

static inline void set_timeout(int sock, int timeout_sec)
{
    struct timeval timeout={timeout_sec, 0};
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

static int tcp_query(response *buffer, thread_info_t *thread_info)
{
    int sock;
    struct sockaddr_in socks_server;
    char tmp[256];
    TS_DECLARE(sock);

    TS_BEGIN(sock);
    memset(&socks_server, 0, sizeof(socks_server));
    socks_server.sin_family = AF_INET;
    socks_server.sin_port = htons(SOCKS_PORT);
    socks_server.sin_addr.s_addr = inet_addr(SOCKS_ADDR);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        LOG("[!] Error creating TCP socket");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&socks_server, sizeof(socks_server)) < 0) {
        LOG("[!] Error connecting to proxy\n");
        return -1;
    }

    set_timeout(sock, 3);

    // socks handshake
    // see "http://en.wikipedia.org/wiki/SOCKS"
    send(sock, "\x05\x01\x00", 3, 0);
    recv(sock, tmp, sizeof(tmp), 0);

    in_addr_t remote_dns = dns_servers[thread_info->cur_sock_dns++];
    if (thread_info->cur_sock_dns == NUM_DNS) {
        thread_info->cur_sock_dns = 0;
    }

    memcpy(tmp, "\x05\x01\x00\x01", 4);
    /* ip */
    memcpy(tmp + 4, &remote_dns, 4);
    /* port */
    memcpy(tmp + 8, "\x00\x35", 2);
    send(sock, tmp, 10, 0);
    recv(sock, tmp, sizeof(tmp), 0);

    // forward dns query
    send(sock, buffer->buffer, buffer->length + 2, 0);

    buffer->length = recv(sock, buffer->buffer, sizeof(buffer->buffer), 0);
    if (buffer->length >= 2) {
        buffer->length -= 2;
    }
    TS_END(sock);

    if (LOG_FILE) {
        fprintf(LOG_FILE, "SOCKS: Using DNS server: %s\n", inet_ntoa(*(struct in_addr *)&remote_dns));

#ifdef DEBUG
        unsigned long sock_cputicks = TS_DIFF(sock);
        DUMP("Using DNS server: %s, time: tick(%lu)-ns(%lu)-ms(%lu)\n",
                inet_ntoa(*(struct in_addr *)&remote_dns),
                sock_cputicks, TS_NSDIFF_TICK(sock_cputicks), TS_MSDIFF_TICK(sock_cputicks));
#endif
        fflush(LOG_FILE);
    }

    close(sock);
    return 0;
}

#if 0
int tcp_query_pure(response *buffer)
{
    int sock;
    struct sockaddr_in socks_server;
    memset(&socks_server, 0, sizeof(socks_server));
    socks_server.sin_family = AF_INET;
    socks_server.sin_port = htons(53);
    srand(time(NULL));
    in_addr_t dns_server = dns_servers[rand() % (NUM_DNS - 1)];
    socks_server.sin_addr.s_addr = dns_server;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        if (LOG_FILE) {
            fprintf(LOG_FILE, "[!] Error creating TCP socket");
        }
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&socks_server, sizeof(socks_server)) < 0) {
        if (LOG_FILE) {
            fprintf(LOG_FILE, "[!] Error connecting to proxy\n");
        }
        return -1;
    }

    // forward dns query
    /* int len =  */send(sock, buffer->buffer, buffer->length + 2, 0);
    buffer->length = recv(sock, buffer->buffer, sizeof(buffer->buffer), 0);
/*     if (LOG_FILE) {
 *         fprintf(LOG_FILE, "Using DNS server: %s, send %u, and resv %u\n", inet_ntoa(*(struct in_addr *)&dns_server), len, buffer->length);
 *     }
 */
    DUMP("Using DNS server: %s, send %u, and resv %u\n", inet_ntoa(*(struct in_addr *)&dns_server), len, buffer->length);
    if (buffer->length >= 2) {
        buffer->length -= 2;
    }

    close(sock);
    return 0;
}
#endif

static int udp_query_pure(response *buffer, thread_info_t *thread_info)
{
    int sock;

    TS_DECLARE(udp);
    TS_BEGIN(udp);

    struct sockaddr_in socks_server;
    memset(&socks_server, 0, sizeof(socks_server));
    socks_server.sin_family = AF_INET;
    socks_server.sin_port = htons(53);

    in_addr_t dns_server = udp_dns_servers[thread_info->cur_udp_dns++];
    if (thread_info->cur_udp_dns == NUM_UDP_DNS) {
        thread_info->cur_udp_dns = 0;
    }

    socks_server.sin_addr.s_addr = dns_server;

    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        LOG("[!] Error creating UDP socket");
        return -1;
    }
    set_timeout(sock, 3);

    // forward dns query
    socklen_t fromsize = sizeof(socks_server);
    int send_len = sendto(sock, buffer->buffer + 2, buffer->length, 0, (struct sockaddr *)&socks_server, fromsize);
    buffer->length = recvfrom(sock, buffer->buffer + 2, sizeof(buffer->buffer) - 2, 0, (struct sockaddr *)&socks_server, &fromsize);
    TS_END(udp);

#if 0
    if (LOG_FILE) {
        fprintf(LOG_FILE, "UDP: Using DNS server: %s\n", inet_ntoa(*(struct in_addr *)&dns_server));

#ifdef DEBUG
        unsigned long udp_cputicks = TS_DIFF(udp);
        DUMP("Using DNS server: %s, send %d, and resv %u, time: tick(%lu)-ns(%lu)-ms(%lu)\n",
                inet_ntoa(*(struct in_addr *)&dns_server), send_len, buffer->length,
                udp_cputicks, TS_NSDIFF_TICK(udp_cputicks), TS_MSDIFF_TICK(udp_cputicks));
#endif
        fflush(LOG_FILE);
    }
#endif

    close(sock);
    return 0;
}

int check_overtime(hkey_t key, void *data, void *arg)
{
    time_t now_time = *(time_t *)arg;
    cache_item_t *item = (cache_item_t *)data;

    if (item && now_time - (time_t)(item->insert_time) >= OVERTIME) {
        return 1;
    }

    LOG("key %x is removed\n", key);

    return 0;
}

static void *dns_thread_process(void *arg)
{
    thread_info_t *thread_info = (thread_info_t *)arg;
    response *work_node = NULL;
    uint32_t this_reader = 0;

    while (!do_exit) {
        mutex_lock(mxlock);
        if (!CAN_CONSUME(writer, pre_reader)) {
            mutex_unlock(mxlock);
            sec_sleep(1);
            continue;
        }

        this_reader = pre_reader++;
        DUMP("thread %u deal with reader %u, pre_reader: %u, writer: %u, reader: %u\n", thread_info->idx, this_reader, pre_reader, writer, reader);
        mutex_unlock(mxlock);

        work_node = &(response_list[GET_LOG_OFFSET(this_reader)]);

        while (work_node->status != WNSE_PREPARE && !do_exit) {
            ms_sleep(100);
        }

        if (work_node->status == WNSE_PREPARE) {
            hkey_t key = super_fast_hash(work_node->buffer + 4, work_node->length - 2);

            cache_item_t *item = hash_get(cache_hash, key, NULL, NULL);
            time_t now_time = time(NULL);
            if (item && now_time - (time_t)(item->insert_time) < OVERTIME) {
                //print_buf_tofile(LOG_FILE, work_node->buffer + 2, work_node->length, "get hash from key: %x, hashsize: %u\n", key, hash_count(cache_hash));

                work_node->length = strncpy_s(work_node->buffer + 4, BUFFER_SIZE - 4, item->data, item->len);
                work_node->length += 2;

                LOG("get hash from key: %x, hashsize: %u\n", key, hash_count(cache_hash));
            } else {
                if (item) {
                    hash_remove(cache_hash, key, NULL, NULL);
                    ufree_setnull(item);
                }
                //print_buf_tofile(LOG_FILE, work_node->buffer + 2, work_node->length, "not get hash from key: %x, hashsize: %u\n", key, hash_count(cache_hash));

                work_node->buffer[0] = 0;
                work_node->buffer[1] = work_node->length;

                int match_result[1] = {0};
                // the tcp query requires the length to precede the packet, so we put the length there
                // forward the packet to the tcp dns server
                int result_num = acbm_search(proxy_list_tree, (unsigned char *)(work_node->buffer + 4), work_node->length - 2, match_result, 1);
                if (result_num > 0) {
                    tcp_query(work_node, thread_info);
                } else {
                    udp_query_pure(work_node, thread_info);
                }

                item = umalloc(sizeof(cache_item_t) + work_node->length - 2);
                if (item) {
                    time_t now_time = time(NULL);
                    item->insert_time = now_time;
                    item->len = work_node->length - 2;
                    memcpy(item->data, work_node->buffer + 4, work_node->length - 2);

                    if (hash_getput(cache_hash, key, NULL, item) < 0) {
                        ufree(item);

                        hash_used_iterate_remove(cache_hash, &check_overtime, &now_time, 5);
                    } else {
                        //print_buf_tofile(LOG_FILE, work_node->buffer + 2, work_node->length, "insert hash from key: %x, hashsize: %u\n", key, hash_count(cache_hash));
                        LOG("insert hash from key: %x, hashsize: %u\n", key, hash_count(cache_hash));
                    }
                }
            }

            // send the reply back to the client (minus the length at the beginning)
            //sendto(udp_sock, work_node->buffer + 2, work_node->length, 0, (struct sockaddr *)&(work_node->dns_client), work_node->client_socklen);
            work_node->status = WNSE_DEAL;
            DUMP("thread %u   done with reader %u, pre_reader: %u, writer: %u, reader: %u\n", thread_info->idx, this_reader, pre_reader, writer, reader);
        }
    }

    return NULL;
}

int create_thread_list(void)
{
    thread_list = ucalloc_type(WORK_THREAD_NUM, thread_info_t);
    if (thread_list == NULL) {
        return -1;
    }

    int32_t idx = 0, ret_val;
    pthread_attr_t attr;
    for (idx = 0; idx < WORK_THREAD_NUM; ++idx) {
        pthread_attr_init(&attr);
        pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        ret_val = pthread_create(&(thread_list[idx].handle), &attr, &dns_thread_process, thread_list + idx);
        if (ret_val != 0) {
            printf("error when create thread\n");
            return -1;
        }
        thread_list[idx].idx = idx;
    }

    mutexlock_init(mxlock);

    return 0;
}        /* -- end of function create_thread_list -- */

static inline void thread_list_close(void)
{
    int32_t idx;
    for (idx = 0; idx < WORK_THREAD_NUM; ++idx) {
        pthread_join(thread_list[idx].handle, NULL);
    }
    ufree(thread_list);
    mutexlock_destroy(mxlock);

    return;
}

char *get_value(char *line)
{
    char *token, *tmp;
    token = strtok(line, " ");

    for (;;) {
        if ((tmp = strtok(NULL, " ")) == NULL) {
            break;
        } else {
            token = tmp;
        }
    }

    return token;
}

char *string_value(char *value)
{
    char *tmp = (char *)malloc(strlen(value) + 1);
    strcpy(tmp, value);
    value = tmp;

    if (value[strlen(value) - 1] == '\n') {
        value[strlen(value) - 1] = '\0';
    }

    return value;
}

#define fix_prefix_fix(__s, __n)    (fix_prefix(__s, strlen(__s), __n) ? __s : NULL)
void parse_config(char *file)
{
    char line[80];
    FILE *f = fopen(file, "r");

    if (!f) {
        error("[!] Error opening configuration file");
    }

    while (fgets(line, 80, f) != NULL) {
        if (line[0] == '#') {
            continue;
        }

        if (fix_prefix_fix(line, "socks_port") != NULL) {
            SOCKS_PORT = strtol(get_value(line), NULL, 10);
        } else if (fix_prefix_fix(line, "socks_addr") != NULL) {
            SOCKS_ADDR = string_value(get_value(line));
        } else if (fix_prefix_fix(line, "listen_addr") != NULL) {
            LISTEN_ADDR = string_value(get_value(line));
        } else if (fix_prefix_fix(line, "listen_port") != NULL) {
            LISTEN_PORT = strtol(get_value(line), NULL, 10);
        } else if (fix_prefix_fix(line, "thread_num") != NULL) {
            WORK_THREAD_NUM = strtol(get_value(line), NULL, 10);
        } else if (fix_prefix_fix(line, "resolv_conf") != NULL) {
            RESOLVCONF = string_value(get_value(line));
        } else if (fix_prefix_fix(line, "sys_resolv_conf") != NULL) {
            SYS_RESOLVCONF = string_value(get_value(line));
        } else if (fix_prefix_fix(line, "log_file") != NULL) {
            LOGFILE = string_value(get_value(line));
        } else if (fix_prefix_fix(line, "proxy_list") != NULL) {
            PROXY_LIST = string_value(get_value(line));
        }
    }

    if (fclose(f) != 0) {
        error("[!] Error closing configuration file");
    }
}

static int parse_etc_resolv_conf(void)
{
#define LINE_LEN 1024
    FILE *fp_resolv = NULL;            /* input-file pointer */
    char *resolv_filename = SYS_RESOLVCONF;    /* input-file name    */
    char readline[LINE_LEN] = {0};
    char *pline = readline;
    char *pline_end = NULL;

    fp_resolv = fopen((const char *)resolv_filename, "r");
    if (fp_resolv == NULL) {
        fprintf (stderr, "couldn't open file '%s'; %s\n",
                resolv_filename, strerror(errno));
        goto err;
    }

    udp_dns_servers = malloc(sizeof(in_addr_t) * 8);
    if (dns_servers == NULL) {
        error("[!] Error malloc udp_dns_server, num: 8\n");
    }

    char *ip_addr = NULL;
    while (memset(readline, 0, LINE_LEN) && fgets(readline, LINE_LEN, fp_resolv)) {
        if ((pline = strchr(readline, '#'))) {
            *pline = '\0';
        }

        pline = readline;
        while (*pline == ' ' || *pline == '\t' || *pline == '\r' || *pline == '\n') {
            pline++;
        }

        if (pline[0] == '\0') {
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
        if (strncasecmp(pline, "nameserver", 10) == 0) {
            ip_addr = get_value(pline);
            if (strncasecmp(ip_addr, LISTEN_ADDR, strlen(LISTEN_ADDR)) != 0 && strncasecmp(ip_addr, "127.0.0.1", strlen("127.0.0.1")) != 0) {
                udp_dns_servers[NUM_UDP_DNS] = inet_addr(ip_addr);
                if (INADDR_NONE != udp_dns_servers[NUM_UDP_DNS]) {
                    printf("[%u]get local dns: %s\n", NUM_UDP_DNS, ip_addr);
                    NUM_UDP_DNS++;
                }
            }
        }
    }

    if (NUM_UDP_DNS == 0) {
        printf("get nothing from /etc/resolv.conf, exit\n");
        goto err;
    }

    fclose(fp_resolv);
    fp_resolv = NULL;
    return 0;

err:
    if (fp_resolv) {
        fclose(fp_resolv);
        fp_resolv = NULL;
    }
    return -1;

}

void parse_resolv_conf()
{
    char ns[80];
    int i = 0, len = 0;
    regex_t preg;
    regmatch_t pmatch[1];
    regcomp(&preg, "^[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+\n$", REG_EXTENDED);

    FILE *f = fopen(RESOLVCONF, "r");
    if (!f) {
        error("[!] Error opening resolv.conf");
    }

    while (fgets(ns, 80, f) != NULL) {
        if (!regexec(&preg, ns, 1, pmatch, 0)) {
            NUM_DNS++;
        }
    }

    if (fclose(f)) {
        error("[!] Error closing resolv.conf");
    }

    dns_servers = malloc(sizeof(in_addr_t) * NUM_DNS);
    if (dns_servers == NULL) {
        printf("NUM_DNS: %d\n", NUM_DNS);
        error("[!] Error malloc dns_server\n");
    }
    f = fopen(RESOLVCONF, "r");

    while (fgets(ns, 80, f) != NULL) {
        if (regexec(&preg, ns, 1, pmatch, 0) != 0) {
            continue;
        }

        len = strlen(ns);
        rstrim(ns, &len);
        ns[len] = '\0';
        dns_servers[i] = inet_addr(ns);

        if (INADDR_NONE != dns_servers[i]) {
            printf("[%u] get resv: %s\n", i, ns);
            i++;
        }
    }
    NUM_DNS = i;

    if (fclose(f)) {
        error("[!] Error closing resolv.conf");
    }

    //printf("get %u address from file %s\n", NUM_DNS, RESOLVCONF);
}

#if 0
// handle children
void reaper_handle(int sig)
{
    while (waitpid(-1, NULL, WNOHANG) > 0) { };

    sig = 0;
}
#endif

#if 0
void create_socket_tosock()
{
    int sock;
    struct sockaddr_in socks_server;
    memset(&socks_server, 0, sizeof(socks_server));
    socks_server.sin_family = AF_INET;
    socks_server.sin_port = htons(SOCKS_PORT);
    socks_server.sin_addr.s_addr = inet_addr(SOCKS_ADDR);
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        error("[!] Error creating TCP socket");
    }

    if (connect(sock, (struct sockaddr *)&socks_server, sizeof(socks_server)) < 0) {
        error("[!] Error connecting to proxy\n");
    }
}
#endif

static inline int recv_dns_query(response *work_node)
{
    work_node->client_socklen = sizeof(struct sockaddr_in);
    int len = recvfrom(udp_sock, work_node->buffer + 2, BUFFER_SIZE - 2, 0, (struct sockaddr *)&(work_node->dns_client), &(work_node->client_socklen));
    // lets not fork if recvfrom was interrupted
    if (len < 0 && errno == EINTR) {
        return -1;
    }

    // other invalid values from recvfrom
    if (len < 0) {
#if 0
        if (LOG_FILE) {
            fprintf(LOG_FILE, "recvfrom failed: %u:%s\n", errno, strerror(errno));
        }
#endif

        return -1;
    }

    work_node->length = len;
    work_node->status = WNSE_PREPARE;
    return 0;
}

int32_t post_consume_logs(uint32_t offset, uint32_t work_num)
{
    uint32_t idx = 0;
    response *work_node = NULL;
    int max_idx = -1;

    for (idx = 0; idx < work_num; ++idx) {
        work_node = response_list + GET_LOG_OFFSET(offset + idx);

        switch (work_node->status) {
            case WNSE_DEAL:
                sendto(udp_sock, work_node->buffer + 2, work_node->length, 0, (struct sockaddr *)&(work_node->dns_client), work_node->client_socklen);
                work_node->status = WNSE_NOTHING;
                break;
            case WNSE_NOTHING:
                break;
            default:
                if (max_idx == -1) {
                    max_idx = idx;
                }
                break;
        }
    }

    return max_idx;
}

static void _sig_int(int signum ARG_UNUSED, siginfo_t *info ARG_UNUSED, void *ptr ARG_UNUSED)
{
    do_exit = true;
    exit(0);
}

static inline int setnonblocking(int sockfd)
{
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1) {
        return -1;
    }

    return 0;
}

int udp_listener(void)
{
    struct sockaddr_in dns_listener;
    memset(&dns_listener, 0, sizeof(dns_listener));
    dns_listener.sin_family = AF_INET;
    dns_listener.sin_port = htons(LISTEN_PORT);
    dns_listener.sin_addr.s_addr = inet_addr(LISTEN_ADDR);
    // create our udp listener
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0) {
        error("[!] Error setting up dns proxy");
    }
    int opt = SO_REUSEADDR;
    setsockopt(udp_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setnonblocking(udp_sock);

    if (bind(udp_sock, (struct sockaddr *)&dns_listener, sizeof(dns_listener)) < 0) {
        error("[!] Error binding on dns proxy");
    }

    if (strcmp(LOGFILE, "/dev/null") != 0) {
        LOG_FILE = fopen(LOGFILE, "w");
        if (!LOG_FILE) {
            error("[!] Error opening logfile.");
        }

        printf("open LOGFILE OK: %s\n", LOGFILE);
    }

    printf("[*] No errors, backgrounding process.\n");

    // setup SIGCHLD handler to kill off zombie children
    setsignal(SIGINT, _sig_int);
    setsignal(SIGTERM, _sig_int);

#if 1
    int32_t daemon_ret = daemon(0, 0);
    if (daemon_ret < 0) {
        error("daemon error\n");
    }
#endif

    cache_hash = hash_init(MAX_CACHE_NUM, true);
    if (cache_hash == NULL) {
        printf("error when create dns cache\n");
    }

    if (create_thread_list() < 0) {
        printf("error when create_thread_list\n");
        exit(1);
    }

    uint32_t nothing_times = 0;
    uint32_t deal_num = 0;
    int32_t ret = 0;

#define SLEEP_TIMES 100
    while (!do_exit) {
        deal_num = 0;

        /* produce */
        if (CAN_PRODUCE(writer, reader, RESPONSE_LIST_NUM)) {
            // receive a dns request from the client
            if (recv_dns_query(response_list + GET_LOG_OFFSET(writer)) == 0) {
                DUMP("after produce one work, len: %u, pre_reader: %u, writer: %u, reader: %u\n",
                        response_list[GET_LOG_OFFSET(writer)].length,
                        pre_reader, writer, reader);
                writer++;
                deal_num++;
            }
        }

        /* consume */
        if (CAN_CONSUME(writer, reader)) {
            ret = post_consume_logs(reader, writer - reader);
            if (ret > 0) {
                DUMP("deal with %d works\n", ret);
                DUMP("[%d]after post consume, pre_reader: %u, writer: %u, reader: %u\n", ret, pre_reader, writer, reader);
            }

            reader += ret;
            deal_num += ret;
        }

        if (deal_num == 0) {
            if (++nothing_times > SLEEP_TIMES) {
                sec_sleep(1);
                nothing_times = 0;
            }
        }
    }

    thread_list_close();
    return 0;
}

int parse_proxy_list(void)
{
#define LINE_LEN 1024
#define MAX_PATTERN_NUM 4096
#define SPLIT_NUM 8
    FILE *fp_plist = NULL;            /* input-file pointer */
    char *plist_filename = PROXY_LIST;        /* input-file name    */
    char readline[LINE_LEN] = {0};
    char *pline = readline;
    char *pline_end = NULL;
    acbm_pattern_data patterns[MAX_PATTERN_NUM];
    memset(patterns, 0, sizeof(patterns));
    int pattern_count = 0;

    split_node_t split_nodes[SPLIT_NUM] = {SPLIT_NODE_INIT_VALUE};
    int split_num = 0;

    fp_plist = fopen((const char *)plist_filename, "r");
    if (fp_plist == NULL) {
        fprintf (stderr, "couldn't open file '%s'; %s\n",
                plist_filename, strerror(errno));
        goto err;
    }

    int32_t idx = 0;
    unsigned char *ptr = NULL;
    while (memset(readline, 0, LINE_LEN) && fgets(readline, LINE_LEN, fp_plist)) {
        if ((pline = strchr(readline, '#'))) {
            *pline = '\0';
        }

        pline = readline;
        while (*pline == ' ' || *pline == '\t' || *pline == '\r' || *pline == '\n') {
            pline++;
        }

        if (pline[0] == '\0') {
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
        patterns[pattern_count].data = umalloc(strlen(pline) + 1);
        if (patterns[pattern_count].data == NULL) {
            printf("[%s:%u]error when malloc proxy.list pattern\n", __func__, __LINE__);
            goto err;
        }

        patterns[pattern_count].len = 0;
        split_num = split_pos(pline, strlen(pline), '.', split_nodes, SPLIT_NUM, true);
        ptr = patterns[pattern_count].data;
        for (idx = 0; idx < split_num; ++idx) {
            *(ptr++) = (unsigned char)(split_nodes[idx].part_len);
            memcpy((char *)ptr, pline + split_nodes[idx].begin_offset, split_nodes[idx].part_len);
            ptr += split_nodes[idx].part_len;
        }
        patterns[pattern_count].len = ptr - patterns[pattern_count].data;
        pattern_count++;
    }

    proxy_list_tree = acbm_search_init(patterns, pattern_count, NULL);
    if (proxy_list_tree == NULL) {
        goto err;
    }
    DUMP("tree size: %lu, tree:%p\n", GET_ACBM_TREE_SIZE(proxy_list_tree), proxy_list_tree);

    for (idx = 0; idx < pattern_count; ++idx) {
        ufree(patterns[pattern_count].data);
    }

    fclose(fp_plist);
    fp_plist = NULL;
    return 0;

err:
    if (fp_plist) {
        fclose(fp_plist);
        fp_plist = NULL;
    }
    return -1;

}

int main(int argc, char *argv[])
{
    if (argc == 1) {
        parse_config("dns_proxy.conf");
    } else if (argc == 2) {
        if (!strcmp(argv[1], "-h")) {
            printf("Usage: %s [options]\n", argv[0]);
            printf(" * With no parameters, the configuration file is read from 'dns_proxy.conf'.\n\n");
            printf(" -n          -- No configuration file (socks: 127.0.0.1:9999, listener: 0.0.0.0:53).\n");
            printf(" -h          -- Print this message and exit.\n");
            printf(" config_file -- Read from specified configuration file.\n\n");
            printf(" * The configuration file should contain any of the following options (and ignores lines that begin with '#'):\n");
            printf("   * socks_addr  -- socks listener address\n");
            printf("   * socks_port  -- socks listener port\n");
            printf("   * listen_addr -- address for the dns proxy to listen on\n");
            printf("   * listen_port -- port for the dns proxy to listen on (most cases 53)\n");
            printf("   * resolv_conf -- location of resolv.conf to read from\n");
            printf("   * log_file    -- location to log server IPs to. (only necessary for debugging)\n\n");
            printf(" * Configuration directives should be of the format:\n");
            printf("   option = value\n\n");
            printf(" * Any non-specified options will be set to their defaults:\n");
            printf("   * socks_addr   = 127.0.0.1\n");
            printf("   * socks_port   = 8099\n");
            printf("   * listen_addr  = 0.0.0.0\n");
            printf("   * listen_port  = 53\n");
            printf("   * resolv_conf  = resolv.conf\n");
            printf("   * log_file     = /dev/null\n");
            exit(0);
        } else {
            parse_config(argv[1]);
        }
    }

    if (getuid() != 0) {
        printf("Error: this program must be run as root! Quitting\n");
        exit(1);
    }

    if (parse_proxy_list() < 0) {
        printf("error when parse proxy.list\n");
        exit(1);
    }

    printf("[*] Listening on: %s:%d\n", LISTEN_ADDR, LISTEN_PORT);
    printf("[*] Using SOCKS proxy: %s:%d\n", SOCKS_ADDR, SOCKS_PORT);
    parse_resolv_conf();
    if (parse_etc_resolv_conf() < 0) {
        printf("error when parse /etc/resolv.conf\n");
        exit(1);
    }

    TS_INIT();

    if (strcmp(LOGFILE, "/dev/null") != 0) {
        printf("[*] Log saved to %s.\n", LOGFILE);
    }
    printf("[*] Loaded %d DNS servers from %s.\n\n", NUM_DNS, RESOLVCONF);

    // start the dns proxy
    udp_listener();

    exit(EXIT_SUCCESS);
}

