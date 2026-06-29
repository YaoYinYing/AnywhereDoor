#ifndef ANYWHEREDOOR_PROXY_H
#define ANYWHEREDOOR_PROXY_H

#include <stdbool.h>

/* Maximum number of test URLs and alternative URLs */
#define MAX_TEST_URLS  16
#define MAX_ALT_URLS   32
#define MAX_NO_PROXY   64
#define MAX_PROXIES    256

/* --- ProxyConfig --- */
typedef struct {
    char *url;
    char *http_port;
    char *socks_port;
    char *auth_user;      /* authentication_user */
    char *password;
    char *alt_urls[MAX_ALT_URLS];   /* alternative_urls */
    int   alt_url_count;
    char *label;
    char *test_urls[MAX_TEST_URLS];
    int   test_url_count;
    int   test_timeout_ms;  /* milliseconds */
} ProxyConfig;

/* Allocate and initialize a ProxyConfig from parameters */
ProxyConfig *proxy_new(const char *url,
                       const char *http_port,
                       const char *socks_port,
                       const char *auth_user,
                       const char *password,
                       const char *label);

/* Free a ProxyConfig */
void proxy_free(ProxyConfig *p);

/* Add an alternative URL to a ProxyConfig */
void proxy_add_alt_url(ProxyConfig *p, const char *url);

/* Build the HTTP proxy connection string (e.g. http://user:pass@host:port) */
char *proxy_http_url(const ProxyConfig *p);

/* Build the SOCKS5 proxy connection string (e.g. socks5h://user:pass@host:port) */
char *proxy_socks_url(const ProxyConfig *p);

/* Human-readable representation for display */
char *proxy_format(const ProxyConfig *p);

/* Expand a ProxyConfig into multiple ProxyConfigs (one per alt_url).
   Returns newly allocated array, sets *count. Caller frees each + the array. */
ProxyConfig **proxy_expand(const ProxyConfig *p, int *count);

/* Compare two ProxyConfigs by connection parameters (url, ports, auth) */
bool proxy_eq(const ProxyConfig *a, const ProxyConfig *b);

/* Check if a ProxyConfig matches a proxy URL tuple (http, https, socks) */
bool proxy_matches_env(const ProxyConfig *p,
                       const char *http_proxy_env,
                       const char *https_proxy_env,
                       const char *socks_proxy_env);

/* --- ProxyTable --- */
typedef struct {
    ProxyConfig *proxies[MAX_PROXIES];
    int count;
} ProxyTable;

/* Initialize an empty ProxyTable */
void proxy_table_init(ProxyTable *table);

/* Add a proxy to the table (takes ownership) */
void proxy_table_add(ProxyTable *table, ProxyConfig *p);

/* Free all proxies in the table */
void proxy_table_free(ProxyTable *table);

/* Expand all proxies' alternative_urls into a flat array.
   Returns newly allocated array, sets *count. Caller frees each + the array. */
ProxyConfig **proxy_table_expand(const ProxyTable *table, int *count);

/* --- NoProxy --- */
typedef struct {
    char *entries[MAX_NO_PROXY];
    int count;
} NoProxy;

/* Add a no-proxy entry */
void noproxy_add(NoProxy *np, const char *entry);

/* Format as semicolon-separated string for NO_PROXY env var */
char *noproxy_format(const NoProxy *np);

/* Free NoProxy entries */
void noproxy_free(NoProxy *np);

#endif /* ANYWHEREDOOR_PROXY_H */
