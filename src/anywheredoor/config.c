#include "config.h"
#include "utils.h"
#include <yaml.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef enum {
    STATE_TOP,
    STATE_PROXIES,
    STATE_PROXY_ITEM,
    STATE_ALT_URLS,
    STATE_NO_PROXY,
} ParseState;

typedef enum {
    CTX_NONE,
    CTX_PROXIES,
    CTX_NO_PROXY,
} SequenceContext;

int config_load(const char *config_path, ProxyTable *table, NoProxy *noproxy) {
    FILE *fh = fopen(config_path, "r");
    if (!fh) {
        fprintf(stderr, "Cannot open config file: %s\n", config_path);
        return -1;
    }

    yaml_parser_t parser;
    yaml_event_t event;

    if (!yaml_parser_initialize(&parser)) {
        fclose(fh);
        return -1;
    }
    yaml_parser_set_input_file(&parser, fh);

    proxy_table_init(table);
    noproxy->count = 0;

    ProxyConfig *cur_proxy = NULL;
    char *scalar_key = NULL;
    ParseState state = STATE_TOP;
    SequenceContext seq_ctx = CTX_NONE;
    int done = 0;
    int rc = 0;

    while (!done) {
        if (!yaml_parser_parse(&parser, &event)) {
            fprintf(stderr, "YAML parse error at line %zu\n",
                    parser.mark.line + 1);
            rc = -1;
            break;
        }

        switch (event.type) {

        case YAML_STREAM_START_EVENT:
        case YAML_DOCUMENT_START_EVENT:
            break;

        case YAML_STREAM_END_EVENT:
        case YAML_DOCUMENT_END_EVENT:
            done = 1;
            break;

        case YAML_MAPPING_START_EVENT:
            if (state == STATE_PROXIES) {
                /* Starting a new proxy mapping inside the proxies sequence */
                cur_proxy = proxy_new(NULL, NULL, NULL, NULL, NULL, "Unnamed");
                state = STATE_PROXY_ITEM;
            }
            break;

        case YAML_MAPPING_END_EVENT:
            if (state == STATE_PROXY_ITEM && cur_proxy) {
                proxy_table_add(table, cur_proxy);
                cur_proxy = NULL;
                state = STATE_PROXIES;
            }
            break;

        case YAML_SEQUENCE_START_EVENT:
            if (scalar_key) {
                if (strcmp(scalar_key, "proxies") == 0) {
                    state = STATE_PROXIES;
                    seq_ctx = CTX_PROXIES;
                } else if (strcmp(scalar_key, "no_proxy") == 0) {
                    state = STATE_NO_PROXY;
                    seq_ctx = CTX_NO_PROXY;
                } else if (strcmp(scalar_key, "alternative_urls") == 0 && cur_proxy) {
                    state = STATE_ALT_URLS;
                }
                free(scalar_key);
                scalar_key = NULL;
            }
            break;

        case YAML_SEQUENCE_END_EVENT:
            if (state == STATE_ALT_URLS) {
                state = STATE_PROXY_ITEM;
            } else if (state == STATE_NO_PROXY) {
                state = STATE_TOP;
                seq_ctx = CTX_NONE;
            } else if (state == STATE_PROXIES) {
                state = STATE_TOP;
                seq_ctx = CTX_NONE;
            }
            break;

        case YAML_SCALAR_EVENT: {
            /* Build value string safely */
            size_t vlen = event.data.scalar.length;
            const char *raw = (const char *)event.data.scalar.value;
            char *value = malloc(vlen + 1);
            memcpy(value, raw, vlen);
            value[vlen] = '\0';

            switch (state) {

            case STATE_TOP:
                /* Expecting top-level keys: "proxies" or "no_proxy" */
                free(scalar_key);
                scalar_key = value;
                break;

            case STATE_PROXIES:
                /* Between proxy items — should only hit if the sequence
                   has scalar entries (unlikely). Free and move on. */
                free(value);
                break;

            case STATE_PROXY_ITEM:
                if (!scalar_key) {
                    /* This is a key in the proxy mapping */
                    scalar_key = value;
                } else {
                    /* This is a value for the previous key */
                    if (strcmp(scalar_key, "label") == 0) {
                        if (cur_proxy) { free(cur_proxy->label); cur_proxy->label = value; }
                        else free(value);
                    } else if (strcmp(scalar_key, "url") == 0) {
                        if (cur_proxy) { free(cur_proxy->url); cur_proxy->url = value; }
                        else free(value);
                    } else if (strcmp(scalar_key, "http_port") == 0) {
                        if (cur_proxy) { free(cur_proxy->http_port); cur_proxy->http_port = value; }
                        else free(value);
                    } else if (strcmp(scalar_key, "socks_port") == 0) {
                        if (cur_proxy) { free(cur_proxy->socks_port); cur_proxy->socks_port = value; }
                        else free(value);
                    } else if (strcmp(scalar_key, "authentication_user") == 0) {
                        if (cur_proxy) { free(cur_proxy->auth_user); cur_proxy->auth_user = value; }
                        else free(value);
                    } else if (strcmp(scalar_key, "password") == 0) {
                        if (cur_proxy) { free(cur_proxy->password); cur_proxy->password = value; }
                        else free(value);
                    } else {
                        /* Unknown key — skip value */
                        free(value);
                    }
                    free(scalar_key);
                    scalar_key = NULL;
                }
                break;

            case STATE_ALT_URLS:
                if (cur_proxy) proxy_add_alt_url(cur_proxy, value);
                free(value);
                break;

            case STATE_NO_PROXY:
                noproxy_add(noproxy, value);
                free(value);
                break;
            }
            break;
        }

        default:
            break;
        }

        yaml_event_delete(&event);
    }

    free(scalar_key);
    if (cur_proxy) proxy_free(cur_proxy);

    yaml_parser_delete(&parser);
    fclose(fh);
    return rc;
}
