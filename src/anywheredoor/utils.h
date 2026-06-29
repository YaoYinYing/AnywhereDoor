#ifndef ANYWHEREDOOR_UTILS_H
#define ANYWHEREDOOR_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* --- ANSI color escape codes (ported from color_escape.py) --- */

#define C_RESET        "\033[0m"
#define C_BOLD         "\033[1m"
#define C_FAINT        "\033[2m"
#define C_ITALIC       "\033[3m"
#define C_UNDERLINE    "\033[4m"
#define C_BLINK        "\033[5m"

#define C_BLACK        "\033[0;30m"
#define C_RED          "\033[0;31m"
#define C_GREEN        "\033[0;32m"
#define C_BROWN        "\033[0;33m"
#define C_BLUE         "\033[0;34m"
#define C_PURPLE       "\033[0;35m"
#define C_CYAN         "\033[0;36m"
#define C_LIGHT_GRAY   "\033[0;37m"

#define C_DARK_GRAY    "\033[1;30m"
#define C_LIGHT_RED    "\033[1;31m"
#define C_LIGHT_GREEN  "\033[1;32m"
#define C_YELLOW       "\033[1;33m"
#define C_LIGHT_BLUE   "\033[1;34m"
#define C_LIGHT_PURPLE "\033[1;35m"
#define C_LIGHT_CYAN   "\033[1;36m"
#define C_LIGHT_WHITE  "\033[1;37m"

#define C_RED_BG       "\033[0;41m"
#define C_CYAN_BG      "\033[0;44m"
#define C_MAGENTA_BG   "\033[0;45m"

/* --- Dynamic string builder --- */

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} StrBuf;

StrBuf *sb_new(void);
void sb_free(StrBuf *sb);
void sb_append(StrBuf *sb, const char *str);
void sb_append_char(StrBuf *sb, char c);
void sb_append_int(StrBuf *sb, int n);
char *sb_detach(StrBuf *sb);

/* --- Path utilities --- */

/* Find .anywheredoorrc: tries cwd, then HOME, then ANYWHERE_DOOR_DIR */
char *find_config_file(void);

/* Get the install directory from ANYWHERE_DOOR_DIR env var */
const char *get_install_dir(void);

/* --- Subprocess helper --- */

/* Run a command and capture output. Returns exit code, prints stdout if verbose. */
int run_command(const char *cmd[], bool verbose);

/* --- String helpers --- */

/* Duplicate at most n bytes of src */
char *strndup_safe(const char *src, size_t n);

/* Trim leading and trailing whitespace in-place */
char *str_trim(char *str);

#endif /* ANYWHEREDOOR_UTILS_H */
