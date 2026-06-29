#include "test.h"
#include "utils.h"
#include <curl/curl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* --- Single URL test --- */

static bool test_single_url(const char *url,
                            const char *proxy_http,
                            const char *proxy_https,
                            int timeout_ms) {
    CURL *curl = curl_easy_init();
    if (!curl) return false;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);  /* HEAD request */
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, (long)timeout_ms);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, (long)(timeout_ms / 2));
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    /* Silence curl output */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

    if (proxy_http) {
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy_http);
        curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
    }

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_easy_cleanup(curl);
    return (res == CURLE_OK) && (http_code >= 200 && http_code < 400);
}

/* --- Concurrent proxy testing --- */

ProxyTestResult test_proxies_concurrent(ProxyConfig *const *proxies, int count) {
    ProxyTestResult result;
    result.count = count;
    result.results = calloc(count, sizeof(bool));

    if (count == 0) return result;

    CURLM *multi = curl_multi_init();
    CURL *handles[count];
    int still_running = 0;

    for (int i = 0; i < count; i++) {
        handles[i] = curl_easy_init();
        CURL *curl = handles[i];

        curl_easy_setopt(curl, CURLOPT_URL, proxies[i]->test_urls[0]);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, (long)proxies[i]->test_timeout_ms);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS,
                         (long)(proxies[i]->test_timeout_ms / 2));
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

        /* Set proxy */
        char *proxy_url = proxy_http_url(proxies[i]);
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy_url);
        curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);

        /* Store index for later retrieval */
        curl_easy_setopt(curl, CURLOPT_PRIVATE, (void *)(intptr_t)i);

        curl_multi_add_handle(multi, curl);
        free(proxy_url);
    }

    /* Perform all transfers */
    curl_multi_perform(multi, &still_running);

    while (still_running) {
        int numfds;
        curl_multi_wait(multi, NULL, 0, 1000, &numfds);
        curl_multi_perform(multi, &still_running);
    }

    /* Collect results */
    CURLMsg *msg;
    int msgs_left;
    while ((msg = curl_multi_info_read(multi, &msgs_left))) {
        if (msg->msg == CURLMSG_DONE) {
            CURL *easy = msg->easy_handle;
            void *priv;
            curl_easy_getinfo(easy, CURLINFO_PRIVATE, &priv);
            int idx = (int)(intptr_t)priv;

            long http_code = 0;
            curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &http_code);
            result.results[idx] = (msg->data.result == CURLE_OK)
                                  && (http_code >= 200 && http_code < 400);
        }
    }

    /* Cleanup */
    for (int i = 0; i < count; i++) {
        curl_multi_remove_handle(multi, handles[i]);
        curl_easy_cleanup(handles[i]);
    }
    curl_multi_cleanup(multi);

    return result;
}

/* --- Concurrent URL testing through one proxy --- */

bool *test_urls_concurrent(const char **urls, int url_count,
                           const char *proxy_http, const char *proxy_https,
                           int timeout_ms) {
    bool *results = calloc(url_count, sizeof(bool));
    if (url_count == 0) return results;

    CURLM *multi = curl_multi_init();
    CURL *handles[url_count];
    int still_running = 0;

    for (int i = 0; i < url_count; i++) {
        handles[i] = curl_easy_init();
        CURL *curl = handles[i];

        curl_easy_setopt(curl, CURLOPT_URL, urls[i]);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, (long)timeout_ms);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, (long)(timeout_ms / 2));
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

        if (proxy_http) {
            curl_easy_setopt(curl, CURLOPT_PROXY, proxy_http);
            curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
        }

        curl_easy_setopt(curl, CURLOPT_PRIVATE, (void *)(intptr_t)i);
        curl_multi_add_handle(multi, curl);
    }

    curl_multi_perform(multi, &still_running);
    while (still_running) {
        int numfds;
        curl_multi_wait(multi, NULL, 0, 1000, &numfds);
        curl_multi_perform(multi, &still_running);
    }

    CURLMsg *msg;
    int msgs_left;
    while ((msg = curl_multi_info_read(multi, &msgs_left))) {
        if (msg->msg == CURLMSG_DONE) {
            CURL *easy = msg->easy_handle;
            void *priv;
            curl_easy_getinfo(easy, CURLINFO_PRIVATE, &priv);
            int idx = (int)(intptr_t)priv;

            long http_code = 0;
            curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &http_code);
            results[idx] = (msg->data.result == CURLE_OK)
                           && (http_code >= 200 && http_code < 400);
        }
    }

    for (int i = 0; i < url_count; i++) {
        curl_multi_remove_handle(multi, handles[i]);
        curl_easy_cleanup(handles[i]);
    }
    curl_multi_cleanup(multi);

    return results;
}

void test_result_free(ProxyTestResult *r) {
    if (r && r->results) {
        free(r->results);
        r->results = NULL;
    }
}
