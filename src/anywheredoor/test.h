#ifndef ANYWHEREDOOR_TEST_H
#define ANYWHEREDOOR_TEST_H

#include "proxy.h"
#include <stdbool.h>

/* Result of testing a set of proxies. Maps 1:1 with the input array. */
typedef struct {
    bool *results;      /* results[i] = true if proxy i is reachable */
    int   count;        /* number of proxies tested */
} ProxyTestResult;

/* Test multiple proxies concurrently against their first test URL.
   Each proxy is tested via its HTTP proxy URL.
   Returns a ProxyTestResult; caller must free result.results. */
ProxyTestResult test_proxies_concurrent(ProxyConfig *const *proxies, int count);

/* Test multiple URLs through a single proxy.
   proxy_http and proxy_https are the proxy strings (or NULL for direct).
   Returns a bool array matching the urls order. Caller must free. */
bool *test_urls_concurrent(const char **urls, int url_count,
                           const char *proxy_http, const char *proxy_https,
                           int timeout_ms);

/* Free a ProxyTestResult */
void test_result_free(ProxyTestResult *r);

#endif /* ANYWHEREDOOR_TEST_H */
