#include "dnsleak.h"
#include "utils.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* --- libcurl write callback: append response to a StrBuf --- */

struct curl_buffer {
    char *data;
    size_t len;
    size_t cap;
};

static size_t write_cb(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct curl_buffer *buf = (struct curl_buffer *)userp;

    size_t needed = buf->len + realsize + 1;
    if (needed > buf->cap) {
        while (buf->cap < needed) buf->cap *= 2;
        buf->data = realloc(buf->data, buf->cap);
    }

    memcpy(buf->data + buf->len, contents, realsize);
    buf->len += realsize;
    buf->data[buf->len] = '\0';
    return realsize;
}

static struct curl_buffer *curl_buf_new(void) {
    struct curl_buffer *buf = malloc(sizeof(struct curl_buffer));
    buf->cap = 4096;
    buf->data = malloc(buf->cap);
    buf->data[0] = '\0';
    buf->len = 0;
    return buf;
}

static void curl_buf_free(struct curl_buffer *buf) {
    if (buf) {
        free(buf->data);
        free(buf);
    }
}

/* --- HTTP GET helper --- */

static char *http_get(const char *url) {
    CURL *curl = curl_easy_init();
    if (!curl) return NULL;

    struct curl_buffer *buf = curl_buf_new();

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, buf);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        curl_buf_free(buf);
        return NULL;
    }

    char *data = buf->data;
    free(buf);
    return data;
}

/* --- Ping helper --- */

static bool ping_host(const char *host) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "ping -c 1 -W 2 %s >/dev/null 2>&1", host);
    int rc = system(cmd);
    return (rc == 0);
}

/* --- Minimal JSON parser for bash.ws response --- */

/* The bash.ws response is a JSON array of objects like:
   [{"type":"ip","ip":"1.2.3.4","country_name":"...","asn":"..."},
    {"type":"dns",...},
    {"type":"conclusion","ip":"..."}]

   We extract fields with a simple string-scanning approach.
*/

typedef struct {
    char *type;
    char *ip;
    char *country_name;
    char *asn;
} DnsEntry;

static char *json_extract_string(const char *json, const char *key) {
    /* Look for "key":"value" */
    char search[128];
    snprintf(search, sizeof(search), "\"%s\":\"", key);
    const char *start = strstr(json, search);
    if (!start) return NULL;
    start += strlen(search);
    const char *end = strchr(start, '"');
    if (!end) return NULL;
    return strndup_safe(start, end - start);
}

static int dns_parse_results(const char *json, DnsEntry **entries_out) {
    /* Count objects by counting "type": */
    int count = 0;
    const char *p = json;
    while ((p = strstr(p, "\"type\":")) != NULL) {
        count++;
        p++;
    }

    DnsEntry *entries = calloc(count, sizeof(DnsEntry));
    *entries_out = entries;

    /* Extract each object */
    p = json;
    for (int i = 0; i < count; i++) {
        const char *obj_start = strstr(p, "{");
        if (!obj_start) break;
        const char *obj_end = strchr(obj_start, '}');
        if (!obj_end) break;

        size_t obj_len = obj_end - obj_start + 1;
        char *obj_str = strndup_safe(obj_start, obj_len);

        entries[i].type = json_extract_string(obj_str, "type");
        entries[i].ip = json_extract_string(obj_str, "ip");
        entries[i].country_name = json_extract_string(obj_str, "country_name");
        entries[i].asn = json_extract_string(obj_str, "asn");

        free(obj_str);
        p = obj_end + 1;
    }

    return count;
}

static void dns_entries_free(DnsEntry *entries, int count) {
    for (int i = 0; i < count; i++) {
        free(entries[i].type);
        free(entries[i].ip);
        free(entries[i].country_name);
        free(entries[i].asn);
    }
    free(entries);
}

/* --- Main test function --- */

int dns_leak_test(void) {
    /* 1. Fetch leak ID */
    printf("Fetching leak ID...\n");
    char *leak_id = http_get("https://bash.ws/id");
    if (!leak_id) {
        fprintf(stderr, "Error fetching leak ID\n");
        return -1;
    }
    /* Trim whitespace */
    char *trimmed_id = str_trim(leak_id);
    printf("Leak ID: " C_YELLOW "%s" C_RESET "\n", trimmed_id);

    /* 2. Ping 10 domains */
    printf("Simulating DNS requests...\n");
    for (int i = 0; i < 10; i++) {
        char domain[256];
        snprintf(domain, sizeof(domain), "%d.%s.bash.ws", i, trimmed_id);
        ping_host(domain);
    }

    /* 3. Retrieve results */
    printf("Retrieving test results...\n");
    char url[512];
    snprintf(url, sizeof(url),
             "https://bash.ws/dnsleak/test/%s?json", trimmed_id);
    free(leak_id);

    char *json_response = http_get(url);
    if (!json_response) {
        fprintf(stderr, "Error retrieving test results\n");
        return -1;
    }

    /* 4. Parse and display */
    DnsEntry *entries = NULL;
    int entry_count = dns_parse_results(json_response, &entries);

    /* Display IP info */
    for (int i = 0; i < entry_count; i++) {
        if (entries[i].type && strcmp(entries[i].type, "ip") == 0
            && entries[i].ip) {
            printf("Your IP:\n");
            printf(C_GREEN "  %s" C_RESET, entries[i].ip);
            if (entries[i].country_name
                && strlen(entries[i].country_name) > 0) {
                printf(" [%s]", entries[i].country_name);
            }
            if (entries[i].asn && strlen(entries[i].asn) > 0) {
                printf(", %s", entries[i].asn);
            }
            printf("\n");
        }
    }

    /* Display DNS servers */
    int dns_count = 0;
    for (int i = 0; i < entry_count; i++) {
        if (entries[i].type && strcmp(entries[i].type, "dns") == 0) {
            dns_count++;
        }
    }

    if (dns_count == 0) {
        printf("No DNS servers found\n");
    } else {
        printf("You use " C_BOLD "%d" C_RESET " DNS servers:\n", dns_count);
        for (int i = 0; i < entry_count; i++) {
            if (entries[i].type && strcmp(entries[i].type, "dns") == 0
                && entries[i].ip) {
                printf("  %s", entries[i].ip);
                if (entries[i].country_name
                    && strlen(entries[i].country_name) > 0) {
                    printf(" [%s]", entries[i].country_name);
                }
                if (entries[i].asn && strlen(entries[i].asn) > 0) {
                    printf(", %s", entries[i].asn);
                }
                printf("\n");
            }
        }
    }

    /* Display conclusion */
    for (int i = 0; i < entry_count; i++) {
        if (entries[i].type && strcmp(entries[i].type, "conclusion") == 0
            && entries[i].ip) {
            printf(C_BOLD C_YELLOW "Conclusion:" C_RESET "\n");
            printf("%s\n", entries[i].ip);
        }
    }

    dns_entries_free(entries, entry_count);
    free(json_response);
    return 0;
}
