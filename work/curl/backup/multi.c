/************************************************
 *       Filename: multi.c
 *    Description:
 *        Created: 2015-01-20 16:08
 *         Author: dengwei david@stratusee.com
 ************************************************/
#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <iostream>

#include <curl/curl.h>
#include <sys/time.h>
#include <unistd.h>

using namespace std;

size_t curl_writer(void *buffer, size_t size, size_t count, void * stream)
{
    std::string * pStream = static_cast<std::string *>(stream);
    (*pStream).append((char *)buffer, size * count);

    return size * count;
};

/**
 * çæääªeasy curlå¹è¡ïèèääçåçè¾ç®æä
 */
CURL * curl_easy_handler(const std::string & sUrl,
                         const std::string & sProxy,
                         std::string & sRsp,
                         unsigned int uiTimeout)
{
    CURL * curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, sUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

    if (uiTimeout > 0)
    {
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, uiTimeout);
    }
    if (!sProxy.empty())
    {
        curl_easy_setopt(curl, CURLOPT_PROXY, sProxy.c_str());
    }

    // write function //
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writer);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &sRsp);

    return curl;
}

/**
 * ä¿ç¨selectå½æ°çå¬multi curlæä¶æè°ç¦çç¶æ
 * çå¬æåèå0ïçå¬å±è¥èå-1
 */
int curl_multi_select(CURLM * curl_m)
{
    int ret = 0;

    struct timeval timeout_tv;
    fd_set  fd_read;
    fd_set  fd_write;
    fd_set  fd_except;
    int     max_fd = -1;

    // æ¨æèéäåèæçºfdset,curl_multi_fdsetääæ§èfdsetçæçºæä  //
    FD_ZERO(&fd_read);
    FD_ZERO(&fd_write);
    FD_ZERO(&fd_except);

    // è¾ç®selectèæ¶æ¶é´  //
    timeout_tv.tv_sec = 1;
    timeout_tv.tv_usec = 0;

    // è·åmulti curléèçå¬çæä¶æè°ç¦éå fd_set //
    curl_multi_fdset(curl_m, &fd_read, &fd_write, &fd_except, &max_fd);

    /**
     * When max_fd returns with -1,
     * you need to wait a while and then proceed and call curl_multi_perform anyway.
     * How long to wait? I would suggest 100 milliseconds at least,
     * but you may want to test it out in your own particular conditions to find a suitable value.
     */
    if (-1 == max_fd)
    {
        return -1;
    }

    /**
     * æ§èçå¬ïåæä¶æè°ç¦ç¶æåçæ¹åçæ¶åèå
     * èå0ïçåèç¨curl_multi_performéç¥curlæ§èç¸åæä
     * èå-1ïè¨çºselectéè¯
     * æ¨æïå³ä¿selectèæ¶äéèèå0ïå·äå¯ä¥å»åççææ£è´æ
     */
    int ret_code = ::select(max_fd + 1, &fd_read, &fd_write, &fd_except, &timeout_tv);
    switch(ret_code)
    {
    case -1:
        /* select error */
        ret = -1;
        break;
    case 0:
        /* select timeout */
    default:
        /* one or more of curl's file descriptors say there's data to read or write*/
        ret = 0;
        break;
    }

    return ret;
}

#define MULTI_CURL_NUM 3

// èéè¾ç®ä éèè¿é®çurl //
std::string     URL     = "http://website.com";
// èéè¾ç®ä£çipåç¯å£  //
std::string     PROXY   = "ip:port";
// èéè¾ç®èæ¶æ¶é´  //
unsigned int    TIMEOUT = 2000; /* ms */

/**
 * multi curlä¿ç¨demo
 */
int curl_multi_demo(int num)
{
    // åååääªmulti curl å¹è¡ //
    CURLM * curl_m = curl_multi_init();

    std::string     RspArray[num];
    CURL *          CurlArray[num];

    // è¾ç®easy curlå¹è¡å¶æ»å å°multi curlå¹è¡ä­  //
    for (int idx = 0; idx < num; ++idx)
    {
        CurlArray[idx] = NULL;
        CurlArray[idx] = curl_easy_handler(URL, PROXY, RspArray[idx], TIMEOUT);
        if (CurlArray[idx] == NULL)
        {
            return -1;
        }
        curl_multi_add_handle(curl_m, CurlArray[idx]);
    }

    /*
     * èç¨curl_multi_performå½æ°æ§ècurlè·æ
     * url_multi_performèåCURLM_CALL_MULTI_PERFORMæ¶ïè¨çºéèç§ç­èç¨è¥å½æ°ç´å°èåå¼äæ¯CURLM_CALL_MULTI_PERFORMäºæ¢
     * running_handlesåéèåæ£å¨åççeasy curlæ°éïrunning_handlesäº0è¨çºååæ¡ææ£å¨æ§èçcurlè·æ
     */
    int running_handles;
    while (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(curl_m, &running_handles))
    {
        cout << running_handles << endl;
    }

    /**
     * äºäé¿ååªç¯èç¨curl_multi_performä§ççcpuæç­å ç¨çé®éïéç¨selectæ¥çå¬æä¶æè°ç¦
     */
    while (running_handles)
    {
        if (-1 == curl_multi_select(curl_m))
        {
            printf("select error\n");
            break;
        } else {
            // selectçå¬å°ää¶ïèç¨curl_multi_performéç¥curlæ§èç¸åçæä //
            while (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(curl_m, &running_handles))
            {
                cout << "select: " << running_handles << endl;
            }
        }
        cout << "select: " << running_handles << endl;
    }

    // èåºæ§èçæ //
    int         msgs_left;
    CURLMsg *   msg;
    while((msg = curl_multi_info_read(curl_m, &msgs_left)))
    {
        if (CURLMSG_DONE == msg->msg)
        {
            int idx;
            for (idx = 0; idx < num; ++idx)
            {
                if (msg->easy_handle == CurlArray[idx]) break;
            }

            if (idx == num)
            {
                cerr << "curl not found" << endl;
            } else
            {
                cout << "curl [" << idx << "] completed with status: "
                        << msg->data.result << endl;
                cout << "rsp: " << RspArray[idx] << endl;
            }
        }
    }

    // èéèæ¨æcleanupçéºå //
    for (int idx = 0; idx < num; ++idx)
    {
        curl_multi_remove_handle(curl_m, CurlArray[idx]);
    }

    for (int idx = 0; idx < num; ++idx)
    {
        curl_easy_cleanup(CurlArray[idx]);
    }

    curl_multi_cleanup(curl_m);

    return 0;
}

/**
 * easy curlä¿ç¨demo
 */
int curl_easy_demo(int num)
{
    std::string     RspArray[num];

    for (int idx = 0; idx < num; ++idx)
    {
        CURL * curl = curl_easy_handler(URL, PROXY, RspArray[idx], TIMEOUT);
        CURLcode code = curl_easy_perform(curl);
        cout << "curl [" << idx << "] completed with status: "
                << code << endl;
        cout << "rsp: " << RspArray[idx] << endl;

        // clear handle //
        curl_easy_cleanup(curl);
    }

    return 0;
}

#define USE_MULTI_CURL

struct timeval begin_tv, end_tv;

int main(int argc, char * argv[])
{
    if (argc < 2)
    {
        return -1;
    }
    int num = atoi(argv[1]);

    // è·åååæ¶é´ //
    gettimeofday(&begin_tv, NULL);
#ifdef USE_MULTI_CURL
    // ä¿ç¨multiæ¥å£èèè¿é® //
    curl_multi_demo(num);
#else
    // ä¿ç¨easyæ¥å£èèè¿é® //
    curl_easy_demo(num);
#endif
    // è·åçææ¶é´  //
    struct timeval end_tv;
    gettimeofday(&end_tv, NULL);

    // è¡çæ§èå¶æ¶å¶èåºïç¨äæè  //
    int eclapsed = (end_tv.tv_sec - begin_tv.tv_sec) * 1000 +
                   (end_tv.tv_usec - begin_tv.tv_usec) / 1000;

    cout << "eclapsed time:" << eclapsed << "ms" << endl;

    return 0;
}
