#include "tui.h"
#include "proxy.h"
#include "config.h"
#include "test.h"
#include "dnsleak.h"
#include "utils.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef VERSION
#define VERSION "2.0.0-alpha"
#endif

/* --- Color pair IDs --- */
#define CP_GREEN   1
#define CP_RED     2
#define CP_YELLOW  3
#define CP_CYAN    4
#define CP_HEADER  5
#define CP_FOOTER  6

static ProxyTable g_table;
static NoProxy g_noproxy;
static ProxyConfig **g_expanded = NULL;
static int g_expanded_count = 0;
static bool *g_reachable = NULL;
static int g_selected = 0;
static int g_scroll_offset = 0;
static bool g_active = false;
static int g_active_index = -1;

/* --- Window dimensions --- */
static int g_rows, g_cols;

/* Forward declarations */
static void load_config(void);
static void refresh_tests(void);
static void draw_screen(void);
static void print_shell_export(int proxy_index);
static void print_shell_unset(void);

/* --- Initialize --- */

static void init_colors(void) {
    start_color();
    init_pair(CP_GREEN,  COLOR_GREEN,   -1);
    init_pair(CP_RED,    COLOR_RED,     -1);
    init_pair(CP_YELLOW, COLOR_YELLOW,  -1);
    init_pair(CP_CYAN,   COLOR_CYAN,    -1);
    init_pair(CP_HEADER, COLOR_BLACK,   COLOR_CYAN);
    init_pair(CP_FOOTER, COLOR_BLACK,   COLOR_WHITE);
}

static void load_config(void) {
    char *config_path = find_config_file();
    if (!config_path) {
        endwin();
        fprintf(stderr, "No .anywheredoorrc found.\n");
        exit(1);
    }

    if (config_load(config_path, &g_table, &g_noproxy) != 0) {
        endwin();
        fprintf(stderr, "Failed to load config: %s\n", config_path);
        free(config_path);
        exit(1);
    }
    free(config_path);

    g_expanded = proxy_table_expand(&g_table, &g_expanded_count);
    g_reachable = calloc(g_expanded_count, sizeof(bool));

    /* Check if a proxy is currently active from environment */
    const char *http_p = getenv("http_proxy");
    const char *https_p = getenv("https_proxy");
    const char *all_p = getenv("all_proxy");

    if (http_p || https_p || all_p) {
        g_active = true;
        for (int i = 0; i < g_expanded_count; i++) {
            if (proxy_matches_env(g_expanded[i], http_p, https_p, all_p)) {
                g_active_index = i;
                g_selected = i;
                break;
            }
        }
    }
}

static void refresh_tests(void) {
    if (g_expanded_count == 0) return;

    ProxyTestResult result = test_proxies_concurrent(g_expanded, g_expanded_count);
    memcpy(g_reachable, result.results, g_expanded_count * sizeof(bool));
    test_result_free(&result);
}

/* --- Drawing --- */

static void draw_header(void) {
    attrset(COLOR_PAIR(CP_HEADER) | A_BOLD);
    mvhline(0, 0, ' ', g_cols);

    char title[128];
    const char *status_text;
    if (g_active && g_active_index >= 0) {
        status_text = "ACTIVE";
    } else if (g_active) {
        status_text = "ACTIVE (custom)";
    } else {
        status_text = "INACTIVE";
    }

    snprintf(title, sizeof(title), " AnywhereDoor v%s | Status: %s ", VERSION, status_text);
    mvaddstr(0, 0, title);

    /* Right-aligned hint */
    char hint[] = " q:Quit ";
    mvaddstr(0, g_cols - (int)strlen(hint), hint);

    attroff(COLOR_PAIR(CP_HEADER) | A_BOLD);
}

static void draw_proxy_list(void) {
    int list_start = 2;
    int list_height = g_rows - 5;  /* leave room for header + footer */

    /* Column headers */
    attrset(A_BOLD | A_UNDERLINE);
    mvaddstr(list_start, 2, "#");
    mvaddstr(list_start, 6, "Status");
    mvaddstr(list_start, 14, "Label");
    mvaddstr(list_start, 36, "Address");
    attroff(A_BOLD | A_UNDERLINE);

    /* Adjust scroll */
    if (g_selected < g_scroll_offset) {
        g_scroll_offset = g_selected;
    }
    if (g_selected >= g_scroll_offset + list_height) {
        g_scroll_offset = g_selected - list_height + 1;
    }

    for (int i = 0; i < list_height && (i + g_scroll_offset) < g_expanded_count; i++) {
        int idx = i + g_scroll_offset;
        int row = list_start + 1 + i;
        ProxyConfig *p = g_expanded[idx];

        /* Highlight selected row */
        bool is_selected = (idx == g_selected);
        bool is_active = (idx == g_active_index);

        if (is_selected) {
            attrset(A_REVERSE);
        }

        /* Clear the line */
        mvhline(row, 0, ' ', g_cols);

        /* Index number */
        char num[8];
        snprintf(num, sizeof(num), "%2d", idx + 1);
        mvaddstr(row, 2, num);

        /* Status indicator */
        if (is_active) {
            attrset(COLOR_PAIR(CP_YELLOW) | (is_selected ? A_REVERSE : 0));
            mvaddstr(row, 6, ">");
        } else if (g_reachable[idx]) {
            attrset(COLOR_PAIR(CP_GREEN) | (is_selected ? A_REVERSE : 0));
            mvaddstr(row, 6, "*");
        } else {
            attrset(COLOR_PAIR(CP_RED) | (is_selected ? A_REVERSE : 0));
            mvaddstr(row, 6, "X");
        }

        /* Label */
        attrset(is_selected ? A_REVERSE : 0);
        mvaddstr(row, 14, p->label ? p->label : "");

        /* Address */
        char addr[128];
        if (p->auth_user && p->password) {
            snprintf(addr, sizeof(addr), "%s:%s@%s:%s",
                     p->auth_user, p->password,
                     p->url ? p->url : "",
                     p->http_port ? p->http_port : "");
        } else {
            snprintf(addr, sizeof(addr), "%s:%s",
                     p->url ? p->url : "",
                     p->http_port ? p->http_port : "");
        }
        mvaddstr(row, 36, addr);

        attroff(A_REVERSE);
        attroff(COLOR_PAIR(CP_GREEN));
        attroff(COLOR_PAIR(CP_RED));
        attroff(COLOR_PAIR(CP_YELLOW));
    }

    /* Fill remaining lines */
    for (int i = g_expanded_count - g_scroll_offset; i < list_height; i++) {
        int row = list_start + 1 + i;
        mvhline(row, 0, ' ', g_cols);
    }
}

static void draw_footer(void) {
    int row = g_rows - 2;

    attrset(COLOR_PAIR(CP_FOOTER));
    mvhline(row, 0, ' ', g_cols);

    char *help = "[Enter]Select  [o]Toggle On/Off  [t]Test All  [d]DNS Leak  [q]Quit  [j/k/Up/Down]Navigate";
    mvaddstr(row, 2, help);
    attroff(COLOR_PAIR(CP_FOOTER));

    /* Bottom row for status messages */
    mvhline(g_rows - 1, 0, ' ', g_cols);
}

static void draw_screen(void) {
    erase();
    getmaxyx(stdscr, g_rows, g_cols);

    if (g_rows < 10 || g_cols < 60) {
        mvaddstr(0, 0, "Terminal too small. Resize and press any key.");
        return;
    }

    draw_header();
    draw_proxy_list();
    draw_footer();
    refresh();
}

/* --- Status message --- */

static void status_msg(const char *msg) {
    mvhline(g_rows - 1, 0, ' ', g_cols);
    mvaddstr(g_rows - 1, 2, msg);
    refresh();
}

/* --- Shell output --- */

static void print_shell_export(int proxy_index) {
    ProxyConfig *p = g_expanded[proxy_index];
    char *http_url = proxy_http_url(p);
    char *socks_url = proxy_socks_url(p);
    char *no_proxy_str = noproxy_format(&g_noproxy);

    printf("export https_proxy=\"%s\";\n", socks_url);
    printf("export http_proxy=\"%s\";\n", socks_url);
    printf("export all_proxy=\"%s\";\n", socks_url);
    printf("export NO_PROXY=\"%s\";\n", no_proxy_str);
    printf("echo 'Proxy selected: %s'\n", p->label);
    printf("echo '%s'\n", socks_url);

    free(http_url);
    free(socks_url);
    free(no_proxy_str);
}

static void print_shell_unset(void) {
    printf("unset https_proxy;\n");
    printf("unset http_proxy;\n");
    printf("unset all_proxy;\n");
    printf("echo 'Anywhere Door is now inactive!'\n");
}

/* --- Main TUI loop --- */

int tui_run(void) {
    /* Load config before initializing ncurses so errors go to terminal */
    char *config_path = find_config_file();
    if (!config_path) {
        fprintf(stderr, "No .anywheredoorrc found.\n");
        return 0;
    }

    if (config_load(config_path, &g_table, &g_noproxy) != 0) {
        fprintf(stderr, "Failed to load config.\n");
        free(config_path);
        return 0;
    }
    free(config_path);

    g_expanded = proxy_table_expand(&g_table, &g_expanded_count);
    g_reachable = calloc(g_expanded_count, sizeof(bool));

    /* Check if proxy is already active */
    const char *http_p = getenv("http_proxy");
    const char *https_p = getenv("https_proxy");
    const char *all_p = getenv("all_proxy");

    if ((http_p && strlen(http_p) > 0)
        || (https_p && strlen(https_p) > 0)
        || (all_p && strlen(all_p) > 0)) {
        g_active = true;
        for (int i = 0; i < g_expanded_count; i++) {
            if (proxy_matches_env(g_expanded[i], http_p, https_p, all_p)) {
                g_active_index = i;
                g_selected = i;
                break;
            }
        }
    }

    /* Initialize ncurses */
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    init_colors();
    getmaxyx(stdscr, g_rows, g_cols);

    /* Run initial tests */
    status_msg("Testing all proxies...");
    refresh_tests();
    draw_screen();

    int exit_code = 0;
    bool running = true;

    while (running) {
        int ch = getch();

        switch (ch) {
        case 'q':
        case 'Q':
            running = false;
            exit_code = 0;
            break;

        case 'j':
        case KEY_DOWN:
            if (g_selected < g_expanded_count - 1) {
                g_selected++;
                draw_screen();
            }
            break;

        case 'k':
        case KEY_UP:
            if (g_selected > 0) {
                g_selected--;
                draw_screen();
            }
            break;

        case 'g':
            /* Go to top */
            g_selected = 0;
            g_scroll_offset = 0;
            draw_screen();
            break;

        case 'G':
            /* Go to bottom */
            g_selected = g_expanded_count - 1;
            draw_screen();
            break;

        case '\n':
        case '\r':
        case KEY_ENTER:
            /* Select proxy and print export */
            print_shell_export(g_selected);
            running = false;
            exit_code = 1;
            break;

        case 'o':
        case 'O':
            /* Toggle on/off */
            if (g_active) {
                print_shell_unset();
                exit_code = 2;
                running = false;
            } else {
                g_active = true;
                g_active_index = g_selected;
                print_shell_export(g_selected);
                exit_code = 1;
                running = false;
            }
            break;

        case 't':
        case 'T':
            status_msg("Testing all proxies...");
            refresh_tests();
            draw_screen();
            break;

        case 'd':
        case 'D':
            /* DNS leak test */
            endwin();
            dns_leak_test();
            printf("\nPress any key to return to TUI...");
            fflush(stdout);
            /* Re-init ncurses */
            refresh();
            initscr();
            cbreak();
            noecho();
            curs_set(0);
            keypad(stdscr, TRUE);
            init_colors();
            getmaxyx(stdscr, g_rows, g_cols);
            draw_screen();
            break;

        case KEY_RESIZE:
            getmaxyx(stdscr, g_rows, g_cols);
            draw_screen();
            break;

        default:
            /* Ignore other keys */
            break;
        }
    }

    /* Cleanup */
    endwin();

    free(g_reachable);
    for (int i = 0; i < g_expanded_count; i++) {
        proxy_free(g_expanded[i]);
    }
    free(g_expanded);
    proxy_table_free(&g_table);
    noproxy_free(&g_noproxy);

    return exit_code;
}
