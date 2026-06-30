#include "proxy.h"
#include "config.h"
#include "test.h"
#include "dnsleak.h"
#include "tui.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef VERSION
#define VERSION "2.0.0-alpha"
#endif

/* Global state — loaded once at startup */
static ProxyTable g_table;
static NoProxy g_noproxy;
static ProxyConfig **g_expanded = NULL;
static int g_expanded_count = 0;
static int g_active_index = -1;  /* index into g_expanded, -1 = none */

/* --- Helpers --- */

/* Print to stderr so it doesn't mix with eval output on stdout */
#define info(fmt, ...)  fprintf(stderr, fmt "\n", ##__VA_ARGS__)

static void load_state(void) {
    char *config_path = find_config_file();
    if (!config_path) {
        fprintf(stderr, C_RED "Config file not found: .anywheredoorrc" C_RESET "\n");
        exit(1);
    }
    int rc = config_load(config_path, &g_table, &g_noproxy);
    free(config_path);
    if (rc != 0) {
        fprintf(stderr, C_RED "Failed to load config" C_RESET "\n");
        exit(1);
    }
    g_expanded = proxy_table_expand(&g_table, &g_expanded_count);

    /* Detect currently active proxy */
    const char *hp = getenv("http_proxy");
    const char *sp = getenv("https_proxy");
    const char *ap = getenv("all_proxy");
    if ((hp && strlen(hp) > 0) || (sp && strlen(sp) > 0) || (ap && strlen(ap) > 0)) {
        for (int i = 0; i < g_expanded_count; i++) {
            if (proxy_matches_env(g_expanded[i], hp, sp, ap)) {
                g_active_index = i;
                break;
            }
        }
    }
}

static void free_state(void) {
    if (g_expanded) {
        for (int i = 0; i < g_expanded_count; i++) {
            proxy_free(g_expanded[i]);
        }
        free(g_expanded);
    }
    proxy_table_free(&g_table);
    noproxy_free(&g_noproxy);
}

static int find_proxy_index(const char *arg) {
    /* Try numeric index first */
    char *end;
    long idx = strtol(arg, &end, 10);
    if (*end == '\0' && idx > 0 && idx <= g_expanded_count) {
        return (int)(idx - 1);
    }
    /* Try label match */
    for (int i = 0; i < g_expanded_count; i++) {
        if (g_expanded[i]->label && strcmp(g_expanded[i]->label, arg) == 0) {
            return i;
        }
    }
    return -1;
}

/* Print export statements for a proxy (to stdout, for shell eval) */
static void print_export(int proxy_index) {
    ProxyConfig *p = g_expanded[proxy_index];
    char *socks_url = proxy_socks_url(p);
    char *no_proxy_str = noproxy_format(&g_noproxy);

    printf("export https_proxy=\"%s\";\n", socks_url);
    printf("export http_proxy=\"%s\";\n", socks_url);
    printf("export all_proxy=\"%s\";\n", socks_url);
    printf("export NO_PROXY=\"%s\";\n", no_proxy_str);

    free(socks_url);
    free(no_proxy_str);
}

/* Print unset statements (to stdout, for shell eval) */
static void print_unset(void) {
    printf("unset https_proxy;\n");
    printf("unset http_proxy;\n");
    printf("unset all_proxy;\n");
}

/* --- Command handlers --- */

static int cmd_on(const char *protocol) {
    if (g_active_index >= 0) {
        /* Already active — print for current selection */
        info(C_GREEN "proxy selected:" C_RESET " " C_YELLOW "%s" C_RESET,
             g_expanded[g_active_index]->label);
        print_export(g_active_index);
        return 0;
    }

    /* Test all proxies and pick the first reachable one */
    info("Testing proxies...");
    ProxyTestResult result = test_proxies_concurrent(g_expanded, g_expanded_count);

    int found = -1;
    for (int i = 0; i < result.count; i++) {
        if (result.results[i]) {
            found = i;
            break;
        }
    }
    test_result_free(&result);

    if (found < 0) {
        info(C_RED "No proxy reachable." C_RESET);
        return 1;
    }

    g_active_index = found;
    info(C_GREEN "proxy selected:" C_RESET " " C_YELLOW "%s" C_RESET,
         g_expanded[found]->label);
    char *fmt = proxy_format(g_expanded[found]);
    info(C_YELLOW "%s" C_RESET, fmt);
    free(fmt);

    print_export(found);
    return 0;
}

static int cmd_off(void) {
    if (g_active_index < 0) {
        info("No proxy is active.");
    }
    info(C_GREEN "Anywhere Door is now inactive!" C_RESET);
    print_unset();
    return 0;
}

static int cmd_use(const char *arg) {
    if (!arg || strlen(arg) == 0) {
        /* No arg: list all proxies with test results */
        info("Testing proxies...");
        ProxyTestResult result = test_proxies_concurrent(g_expanded, g_expanded_count);

        printf("Available proxies:\n");
        printf("---------------------------------------------------------------------------\n");

        const char *hp = getenv("http_proxy");
        const char *sp = getenv("https_proxy");
        const char *ap = getenv("all_proxy");

        for (int i = 0; i < g_expanded_count; i++) {
            ProxyConfig *p = g_expanded[i];
            bool is_active = proxy_matches_env(p, hp, sp, ap);
            bool reachable = result.results[i];

            const char *color = reachable ? C_GREEN : C_RED;
            const char *active_mark = is_active ? C_YELLOW "=> " C_RESET : "";

            char *fmt = proxy_format(p);
            printf("%s%d. %s" C_BOLD "[%s]" C_RESET " %s%s%s\n",
                   active_mark, i + 1, color,
                   p->label ? p->label : "",
                   is_active ? C_YELLOW : "",
                   fmt,
                   is_active ? C_RESET : "");
            free(fmt);
        }
        printf("---------------------------------------------------------------------------\n");
        printf("Please use `anywhere_door use " C_RED "<index>" C_RESET "` to pick one\n");
        printf(C_GREEN C_BOLD "GREEN: passed" C_RESET "  "
               C_YELLOW C_BOLD "YELLOW: in use" C_RESET "  "
               C_RED C_BOLD "RED: failed" C_RESET "\n");

        test_result_free(&result);
        return 0;
    }

    /* Select a proxy by index or label */
    int idx = find_proxy_index(arg);
    if (idx < 0) {
        info(C_RED "Invalid proxy index/label: %s" C_RESET, arg);
        info("Use `anywhere_door use` to list available proxies.");
        return 1;
    }

    g_active_index = idx;
    info(C_GREEN "proxy selected:" C_RESET " " C_YELLOW "%s" C_RESET,
         g_expanded[idx]->label);
    char *fmt = proxy_format(g_expanded[idx]);
    info(C_YELLOW "%s" C_RESET, fmt);
    free(fmt);

    print_export(idx);
    return 0;
}

static int cmd_list(const char *arg) {
    bool show_raw = (arg && strcmp(arg, "all") == 0);

    if (show_raw) {
        /* Show unexpanded proxies */
        printf("Available proxies:\n");
        printf("---------------------------------------------------------------------------\n");
        for (int i = 0; i < g_table.count; i++) {
            ProxyConfig *p = g_table.proxies[i];
            char *fmt = proxy_format(p);
            printf("%d. " C_BOLD "[%s]" C_RESET " %s\n", i + 1, p->label, fmt);
            free(fmt);
        }
        printf("---------------------------------------------------------------------------\n");
    } else {
        /* Show expanded list */
        printf("Available proxies:\n");
        printf("---------------------------------------------------------------------------\n");
        for (int i = 0; i < g_expanded_count; i++) {
            ProxyConfig *p = g_expanded[i];
            char *fmt = proxy_format(p);
            printf("%d. " C_BOLD "[%s]" C_RESET " %s\n", i + 1, p->label ? p->label : "", fmt);
            free(fmt);
        }
        printf("---------------------------------------------------------------------------\n");
        printf("Please use `anywhere_door use " C_RED "<index>" C_RESET "` to pick one\n");
    }
    return 0;
}

static int cmd_show(const char *type) {
    const char *hp = getenv("http_proxy");
    const char *sp = getenv("https_proxy");
    const char *ap = getenv("all_proxy");

    if (!hp && !sp && !ap) {
        printf(C_YELLOW "Anywhere Door is inactive. No configurations to show." C_RESET "\n");
        return 0;
    }

    if (!type || strlen(type) == 0) {
        printf("$http_proxy="  C_GREEN "%s" C_RESET "\n", hp ? hp : "");
        printf("$https_proxy=" C_GREEN "%s" C_RESET "\n", sp ? sp : "");
        printf("$all_proxy="   C_GREEN "%s" C_RESET "\n", ap ? ap : "");
        return 0;
    }

    if (strcmp(type, "http") == 0) {
        printf(C_GREEN "%s" C_RESET "\n", hp ? hp : "");
    } else if (strcmp(type, "https") == 0) {
        printf(C_GREEN "%s" C_RESET "\n", sp ? sp : "");
    } else if (strcmp(type, "all") == 0) {
        printf(C_GREEN "%s" C_RESET "\n", ap ? ap : "");
    }
    return 0;
}

static int cmd_test(const char *arg) {
    if (!arg || strlen(arg) == 0) {
        /* Test current connection through system proxy */
        const char *hp = getenv("http_proxy");
        const char *sp = getenv("https_proxy");
        const char *test_urls[] = {
            "https://www.google.com",
            "https://www.facebook.com",
            "https://www.twitter.com",
            "https://www.instagram.com",
        };
        int n_urls = 4;

        bool *results = test_urls_concurrent(test_urls, n_urls, hp, sp, 10000);
        for (int i = 0; i < n_urls; i++) {
            if (results[i]) {
                printf("Connection test to " C_GREEN "%s: Success" C_RESET "\n", test_urls[i]);
            } else {
                printf("Connection test to " C_RED "%s: Failed" C_RESET "\n", test_urls[i]);
            }
        }
        free(results);
        return 0;
    }

    if (strcmp(arg, "all") == 0) {
        printf("Testing proxies...\n");
        ProxyTestResult result = test_proxies_concurrent(g_expanded, g_expanded_count);
        for (int i = 0; i < result.count; i++) {
            printf(" %s%d" C_RESET, result.results[i] ? C_GREEN : C_RED, i + 1);
        }
        printf("\n");
        test_result_free(&result);
        return 0;
    }

    if (strcmp(arg, "full") == 0) {
        printf("\n===========================================================================\n");
        printf("URL Testing ... ...\n");
        printf("===========================================================================\n");

        ProxyTestResult result = test_proxies_concurrent(g_expanded, g_expanded_count);
        int passed = 0;
        for (int i = 0; i < result.count; i++) {
            ProxyConfig *p = g_expanded[i];
            printf("Testing: %d: " C_GREEN "%s" C_RESET "\n", i + 1, p->label);
            char *fmt = proxy_format(p);
            printf(C_YELLOW "%s" C_RESET "\n", fmt);
            free(fmt);

            if (result.results[i]) {
                printf("Connection test of " C_GREEN "%s: Success" C_RESET "\n",
                       p->test_urls[0]);
                passed++;
            } else {
                printf("Connection test of " C_RED "%s: Failed" C_RESET "\n",
                       p->test_urls[0]);
            }
            printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
        }
        printf("===========================================================================\n");
        printf("Testing Passed: " C_GREEN);
        for (int i = 0; i < result.count; i++) {
            if (result.results[i]) printf("%d ", i + 1);
        }
        printf(C_RESET "\n");
        printf("===========================================================================\n");
        test_result_free(&result);
        return 0;
    }

    printf("Unknown test option: %s\n", arg);
    return 1;
}

static int cmd_config(int argc, char **argv) {
    /* anywhere_door config ip http_port socks_port [user] [pass] */
    if (argc < 3) {
        info(C_RED "Usage: anywhere_door config <ip> <http_port> <socks_port> [user] [pass]" C_RESET);
        return 1;
    }

    const char *ip = argv[0];
    const char *http_port = argv[1];
    const char *socks_port = argv[2];
    const char *user = (argc >= 4) ? argv[3] : NULL;
    const char *pass = (argc >= 5) ? argv[4] : NULL;

    ProxyConfig *custom = proxy_new(ip, http_port, socks_port, user, pass, "Configured");
    char *socks_url = proxy_socks_url(custom);
    char *no_proxy_str = noproxy_format(&g_noproxy);

    printf("export https_proxy=\"%s\";\n", socks_url);
    printf("export http_proxy=\"%s\";\n", socks_url);
    printf("export all_proxy=\"%s\";\n", socks_url);
    printf("export NO_PROXY=\"%s\";\n", no_proxy_str);

    info(C_GREEN "Configured proxy:" C_RESET " " C_YELLOW "%s" C_RESET, socks_url);

    free(socks_url);
    free(no_proxy_str);
    proxy_free(custom);
    return 0;
}

static int cmd_git(void) {
    const char *hp = getenv("http_proxy");
    bool active = (hp && strlen(hp) > 0);

    if (active) {
        printf("Enabling proxy for git\n");
        const char *git_set_http[]  = {"git", "config", "--global", "http.proxy", hp, NULL};
        const char *git_set_https[] = {"git", "config", "--global", "https.proxy", hp, NULL};
        run_command(git_set_http, true);
        run_command(git_set_https, true);
    } else {
        printf("Disabling proxy for git\n");
        const char *git_unset_http[]  = {"git", "config", "--global", "--unset", "http.proxy", NULL};
        const char *git_unset_https[] = {"git", "config", "--global", "--unset", "https.proxy", NULL};
        run_command(git_unset_http, true);
        run_command(git_unset_https, true);
    }
    printf("Done.\n");
    return 0;
}

static int cmd_docker_daemon(void) {
    const char *hp = getenv("http_proxy");
    const char *sp = getenv("https_proxy");
    const char *ap = getenv("all_proxy");

    if (!hp && !sp && !ap) {
        printf(C_YELLOW "No proxy configured. Please activate a proxy first." C_RESET "\n");
        return 1;
    }

    char *no_proxy_str = noproxy_format(&g_noproxy);

    printf("-------------------------------------"
           "--------------------------------------\n");
    printf("JSON\n");
    printf("-------------------------------------"
           "--------------------------------------\n");
    printf("{\n");
    printf("  \"proxies\": {\n");
    printf("    \"default\": {\n");
    printf("      \"httpProxy\": \"%s\",\n", hp ? hp : "");
    printf("      \"httpsProxy\": \"%s\",\n", sp ? sp : "");
    printf("      \"noProxy\": \"%s\"\n", no_proxy_str);
    printf("    }\n");
    printf("  }\n");
    printf("}\n\n");

    printf("-------------------------------------"
           "--------------------------------------\n");
    printf("SYSTEMCTL\n");
    printf("-------------------------------------"
           "--------------------------------------\n");
    printf("[Service]\n");
    printf("Environment=\"HTTP_PROXY=%s\"\n", hp ? hp : "");
    printf("Environment=\"HTTPS_PROXY=%s\"\n", sp ? sp : "");
    printf("Environment=\"ALL_PROXY=%s\"\n", ap ? ap : "");

    /* Replace semicolons with commas for systemd */
    char *comma_no_proxy = strdup(no_proxy_str);
    for (char *c = comma_no_proxy; *c; c++) {
        if (*c == ';') *c = ',';
    }
    printf("Environment=\"NO_PROXY=%s\"\n", comma_no_proxy);
    free(comma_no_proxy);
    free(no_proxy_str);
    return 0;
}

static int cmd_export(void) {
    const char *hp = getenv("http_proxy");
    const char *sp = getenv("https_proxy");
    const char *ap = getenv("all_proxy");
    char *no_proxy_str = noproxy_format(&g_noproxy);

    printf("export HTTP_PROXY=%s\n", hp ? hp : "");
    printf("export HTTPS_PROXY=%s\n", sp ? sp : "");
    printf("export ALL_PROXY=%s\n", ap ? ap : "");

    /* Replace semicolons with commas for systemd-style export */
    char *comma_np = strdup(no_proxy_str);
    for (char *c = comma_np; *c; c++) {
        if (*c == ';') *c = ',';
    }
    printf("export NO_PROXY=%s\n", comma_np);
    free(comma_np);
    free(no_proxy_str);
    return 0;
}

static int cmd_help(const char *topic) {
    if (!topic || strlen(topic) == 0) {
        printf("Anywhere Door: A quick switch for network proxies in the current session.\n");
        printf("Usage: anywhere_door [command]\n\n");
        printf("Commands:\n");
        printf("   on          : Activate Anywhere Door\n");
        printf("   off         : Deactivate Anywhere Door\n");
        printf("   config      : Configure custom IP and port\n");
        printf("   show        : Show current proxy configurations\n");
        printf("   export      : Show quick export lines for shell\n");
        printf("   list/ls     : Show all predefined proxies\n");
        printf("   test        : Test proxy connectivity\n");
        printf("   dns/leak    : DNS Leak Test\n");
        printf("   use         : Use a specific proxy from the configured list\n");
        printf("   git         : Configure git proxy settings\n");
        printf("   docker_daemon : Generate Docker daemon proxy config\n");
        printf("   tui         : Launch interactive TUI\n");
        printf("   help/?      : Show this help message\n");
        printf("   version     : Show version info\n");
        return 0;
    }

    if (strcmp(topic, "test") == 0) {
        printf("Testing proxies.\n");
        printf("Usage: anywhere_door test [opt]\n");
        printf("   <empty>   : Test current proxy.\n");
        printf("   all       : Test all predefined proxies, simplified.\n");
        printf("   full      : Test all predefined proxies, detailed.\n");
    } else if (strcmp(topic, "use") == 0) {
        printf("Call a predefined proxy.\n");
        printf("Usage: anywhere_door use [opt]\n");
        printf("   <empty>   : Show all proxies with test results.\n");
        printf("   [index]   : Select proxy by number.\n");
        printf("   [label]   : Select proxy by label.\n");
    } else if (strcmp(topic, "config") == 0) {
        printf("Configure a new proxy.\n");
        printf("Usage: anywhere_door config <ip> <http_port> <socks_port> [user] [pass]\n");
    } else if (strcmp(topic, "show") == 0) {
        printf("Show current proxy.\n");
        printf("Usage: anywhere_door show [http|https|all]\n");
    } else if (strcmp(topic, "list") == 0 || strcmp(topic, "ls") == 0) {
        printf("Usage: anywhere_door %s [opt]\n", topic);
        printf("   <empty>   : List all proxies expanded.\n");
        printf("   all       : List all predefined proxies (raw).\n");
    } else if (strcmp(topic, "docker_daemon") == 0) {
        printf("Generate Docker daemon proxy config.\n");
        printf("Usage: anywhere_door docker_daemon\n");
    } else if (strcmp(topic, "gost") == 0) {
        printf("Wrap Socks to HTTP by GOST (shell-only command).\n");
        printf("Usage: anywhere_door gost [on|off|clean|update] [port]\n");
    } else {
        printf("No detailed help for '%s'.\n", topic);
    }
    return 0;
}

/* --- Main --- */

int main(int argc, char **argv) {
    const char *command = (argc >= 2) ? argv[1] : "on";

    /* Commands that don't need config loading */
    if (strcmp(command, "help") == 0 || strcmp(command, "?") == 0) {
        const char *topic = (argc >= 3) ? argv[2] : NULL;
        return cmd_help(topic);
    }

    if (strcmp(command, "version") == 0) {
        printf("AnywhereDoor v" VERSION "\n");
        return 0;
    }

    if (strcmp(command, "tui") == 0) {
        return tui_run();
    }

    /* All other commands need config */
    load_state();

    int rc = 0;

    if (strcmp(command, "on") == 0 || argc < 2) {
        rc = cmd_on(argc >= 3 ? argv[2] : NULL);
    } else if (strcmp(command, "off") == 0) {
        rc = cmd_off();
    } else if (strcmp(command, "use") == 0) {
        rc = cmd_use(argc >= 3 ? argv[2] : NULL);
    } else if (strcmp(command, "list") == 0 || strcmp(command, "ls") == 0) {
        rc = cmd_list(argc >= 3 ? argv[2] : NULL);
    } else if (strcmp(command, "show") == 0) {
        rc = cmd_show(argc >= 3 ? argv[2] : NULL);
    } else if (strcmp(command, "test") == 0) {
        rc = cmd_test(argc >= 3 ? argv[2] : NULL);
    } else if (strcmp(command, "config") == 0) {
        rc = cmd_config(argc - 3, argv + 3);
    } else if (strcmp(command, "dns") == 0 || strcmp(command, "leak") == 0) {
        rc = dns_leak_test();
    } else if (strcmp(command, "git") == 0) {
        rc = cmd_git();
    } else if (strcmp(command, "docker_daemon") == 0) {
        rc = cmd_docker_daemon();
    } else if (strcmp(command, "export") == 0) {
        rc = cmd_export();
    } else {
        /* Default: same as "on" */
        rc = cmd_on(NULL);
    }

    free_state();
    return rc;
}
