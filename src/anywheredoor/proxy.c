#include "proxy.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* --- Default test URLs (same as Python version) --- */
static const char *default_test_urls[] = {
    "https://www.google.com",
    "https://www.facebook.com",
    "https://www.twitter.com",
    "https://www.instagram.com",
};
#define DEFAULT_TEST_URL_COUNT 4
#define DEFAULT_TEST_TIMEOUT_MS 10000

/* --- ProxyConfig --- */

ProxyConfig *proxy_new(const char *url,
                       const char *http_port,
                       const char *socks_port,
                       const char *auth_user,
                       const char *password,
                       const char *label) {
    ProxyConfig *p = calloc(1, sizeof(ProxyConfig));
    p->url = url ? strdup(url) : NULL;
    p->http_port = http_port ? strdup(http_port) : NULL;
    p->socks_port = socks_port ? strdup(socks_port) : NULL;
    p->auth_user = auth_user ? strdup(auth_user) : NULL;
    p->password = password ? strdup(password) : NULL;
    p->label = label ? strdup(label) : strdup("Unnamed");
    p->alt_url_count = 0;
    p->test_timeout_ms = DEFAULT_TEST_TIMEOUT_MS;

    /* Set default test URLs */
    p->test_url_count = DEFAULT_TEST_URL_COUNT;
    for (int i = 0; i < DEFAULT_TEST_URL_COUNT; i++) {
        p->test_urls[i] = strdup(default_test_urls[i]);
    }

    return p;
}

void proxy_free(ProxyConfig *p) {
    if (!p) return;
    free(p->url);
    free(p->http_port);
    free(p->socks_port);
    free(p->auth_user);
    free(p->password);
    free(p->label);
    for (int i = 0; i < p->alt_url_count; i++) {
        free(p->alt_urls[i]);
    }
    for (int i = 0; i < p->test_url_count; i++) {
        free(p->test_urls[i]);
    }
    free(p);
}

void proxy_add_alt_url(ProxyConfig *p, const char *url) {
    if (p->alt_url_count >= MAX_ALT_URLS) return;
    p->alt_urls[p->alt_url_count++] = strdup(url);
}

char *proxy_http_url(const ProxyConfig *p) {
    StrBuf *sb = sb_new();
    sb_append(sb, "http://");
    if (p->auth_user && p->password) {
        sb_append(sb, p->auth_user);
        sb_append_char(sb, ':');
        sb_append(sb, p->password);
        sb_append_char(sb, '@');
    }
    sb_append(sb, p->url ? p->url : "");
    sb_append_char(sb, ':');
    sb_append(sb, p->http_port ? p->http_port : "");
    return sb_detach(sb);
}

char *proxy_socks_url(const ProxyConfig *p) {
    StrBuf *sb = sb_new();
    sb_append(sb, "socks5h://");
    if (p->auth_user && p->password) {
        sb_append(sb, p->auth_user);
        sb_append_char(sb, ':');
        sb_append(sb, p->password);
        sb_append_char(sb, '@');
    }
    sb_append(sb, p->url ? p->url : "");
    sb_append_char(sb, ':');
    sb_append(sb, p->socks_port ? p->socks_port : "");
    return sb_detach(sb);
}

char *proxy_format(const ProxyConfig *p) {
    StrBuf *sb = sb_new();
    if (p->auth_user && p->password) {
        sb_append(sb, p->auth_user);
        sb_append_char(sb, ':');
        sb_append(sb, p->password);
        sb_append_char(sb, '@');
    }
    sb_append(sb, p->url ? p->url : "");
    sb_append_char(sb, ':');
    sb_append(sb, p->http_port ? p->http_port : "");
    sb_append_char(sb, '/');
    sb_append(sb, p->socks_port ? p->socks_port : "");

    if (p->alt_url_count > 0) {
        sb_append(sb, " [");
        for (int i = 0; i < p->alt_url_count; i++) {
            if (i > 0) sb_append(sb, ", ");
            sb_append_char(sb, '(');
            sb_append(sb, p->alt_urls[i]);
            sb_append_char(sb, ')');
        }
        sb_append(sb, "]");
    }
    return sb_detach(sb);
}

ProxyConfig **proxy_expand(const ProxyConfig *p, int *count) {
    if (p->alt_url_count == 0) {
        /* No alternatives — return a single copy */
        ProxyConfig **result = malloc(sizeof(ProxyConfig *));
        ProxyConfig *copy = proxy_new(p->url, p->http_port, p->socks_port,
                                      p->auth_user, p->password, p->label);
        result[0] = copy;
        *count = 1;
        return result;
    }

    int total = 1 + p->alt_url_count;
    ProxyConfig **result = malloc(sizeof(ProxyConfig *) * total);

    /* Build labels with suffixes */
    char label_buf[256];

    /* First: original URL */
    snprintf(label_buf, sizeof(label_buf), "%s_0", p->label);
    result[0] = proxy_new(p->url, p->http_port, p->socks_port,
                          p->auth_user, p->password, label_buf);

    /* Then: each alternative */
    for (int i = 0; i < p->alt_url_count; i++) {
        snprintf(label_buf, sizeof(label_buf), "%s_%d", p->label, i + 1);
        result[i + 1] = proxy_new(p->alt_urls[i], p->http_port, p->socks_port,
                                  p->auth_user, p->password, label_buf);
    }
    *count = total;
    return result;
}

bool proxy_eq(const ProxyConfig *a, const ProxyConfig *b) {
    if (!a || !b) return false;
    return (a->url && b->url && strcmp(a->url, b->url) == 0) ||
           (!a->url && !b->url);
}

bool proxy_matches_env(const ProxyConfig *p,
                       const char *http_proxy_env,
                       const char *https_proxy_env,
                       const char *socks_proxy_env) {
    if (!p || !http_proxy_env || !https_proxy_env || !socks_proxy_env) {
        return false;
    }
    char *http_url = proxy_http_url(p);
    char *socks_url = proxy_socks_url(p);
    bool match = (strcmp(http_url, http_proxy_env) == 0 ||
                  strcmp(http_url, https_proxy_env) == 0 ||
                  strcmp(socks_url, socks_proxy_env) == 0);
    free(http_url);
    free(socks_url);
    return match;
}

/* --- ProxyTable --- */

void proxy_table_init(ProxyTable *table) {
    table->count = 0;
}

void proxy_table_add(ProxyTable *table, ProxyConfig *p) {
    if (table->count >= MAX_PROXIES) return;
    table->proxies[table->count++] = p;
}

void proxy_table_free(ProxyTable *table) {
    for (int i = 0; i < table->count; i++) {
        proxy_free(table->proxies[i]);
    }
    table->count = 0;
}

ProxyConfig **proxy_table_expand(const ProxyTable *table, int *count) {
    /* First pass: count total */
    int total = 0;
    for (int i = 0; i < table->count; i++) {
        int expanded_count;
        ProxyConfig **expanded = proxy_expand(table->proxies[i], &expanded_count);
        total += expanded_count;
        for (int j = 0; j < expanded_count; j++) {
            proxy_free(expanded[j]);
        }
        free(expanded);
    }

    /* Second pass: allocate and fill */
    ProxyConfig **result = malloc(sizeof(ProxyConfig *) * (total + 1));
    int idx = 0;
    for (int i = 0; i < table->count; i++) {
        int expanded_count;
        ProxyConfig **expanded = proxy_expand(table->proxies[i], &expanded_count);
        for (int j = 0; j < expanded_count; j++) {
            result[idx++] = expanded[j];
        }
        free(expanded);
    }
    *count = idx;
    return result;
}

/* --- NoProxy --- */

void noproxy_add(NoProxy *np, const char *entry) {
    if (np->count >= MAX_NO_PROXY) return;
    np->entries[np->count++] = strdup(entry);
}

char *noproxy_format(const NoProxy *np) {
    StrBuf *sb = sb_new();
    for (int i = 0; i < np->count; i++) {
        if (i > 0) sb_append_char(sb, ';');
        sb_append(sb, np->entries[i]);
    }
    return sb_detach(sb);
}

void noproxy_free(NoProxy *np) {
    for (int i = 0; i < np->count; i++) {
        free(np->entries[i]);
    }
    np->count = 0;
}
