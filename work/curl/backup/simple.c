/************************************************
 *       Filename: simple.c
 *    Description:
 *        Created: 2015-01-18 18:32
 *         Author: dengwei david@stratusee.com
 ************************************************/
#include <stdlib.h>
#include <stdio.h>

#include <stdio.h>
#include <curl/curl.h>

int main(void)
{
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.101.102:8080/static/code.py");
        /* example.com is redirected, so we tell libcurl to follow redirection */
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        /* Perform the request, res will get the return code */
        int idx = 0;

        for (idx = 0; idx < 10 ; ++idx) {
            res = curl_easy_perform(curl);

            /* Check for errors */
            if (res != CURLE_OK)
                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                        curl_easy_strerror(res));
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    return 0;
}
