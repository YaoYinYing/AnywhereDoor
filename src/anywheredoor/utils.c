#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>

/* --- Dynamic string builder --- */

StrBuf *sb_new(void) {
    StrBuf *sb = malloc(sizeof(StrBuf));
    sb->cap = 256;
    sb->data = malloc(sb->cap);
    sb->data[0] = '\0';
    sb->len = 0;
    return sb;
}

void sb_free(StrBuf *sb) {
    if (sb) {
        free(sb->data);
        free(sb);
    }
}

void sb_append(StrBuf *sb, const char *str) {
    if (!str) return;
    size_t add_len = strlen(str);
    while (sb->len + add_len + 1 > sb->cap) {
        sb->cap *= 2;
        sb->data = realloc(sb->data, sb->cap);
    }
    memcpy(sb->data + sb->len, str, add_len);
    sb->len += add_len;
    sb->data[sb->len] = '\0';
}

void sb_append_char(StrBuf *sb, char c) {
    if (sb->len + 2 > sb->cap) {
        sb->cap *= 2;
        sb->data = realloc(sb->data, sb->cap);
    }
    sb->data[sb->len++] = c;
    sb->data[sb->len] = '\0';
}

void sb_append_int(StrBuf *sb, int n) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", n);
    sb_append(sb, buf);
}

char *sb_detach(StrBuf *sb) {
    char *data = sb->data;
    free(sb);
    return data;
}

/* --- Path utilities --- */

const char *get_install_dir(void) {
    const char *dir = getenv("ANYWHERE_DOOR_DIR");
    return dir ? dir : ".";
}

char *find_config_file(void) {
    const char *locations[3];
    char cwd_buf[1024];

    locations[0] = getcwd(cwd_buf, sizeof(cwd_buf)) ? cwd_buf : ".";
    locations[1] = getenv("HOME");
    locations[2] = getenv("ANYWHERE_DOOR_DIR");

    for (int i = 0; i < 3; i++) {
        if (!locations[i]) continue;
        char *path = malloc(strlen(locations[i]) + 32);
        sprintf(path, "%s/.anywheredoorrc", locations[i]);
        if (access(path, R_OK) == 0) {
            return path;
        }
        free(path);
    }
    return NULL;
}

/* --- Subprocess helper --- */

int run_command(const char *cmd[], bool verbose) {
    if (verbose) {
        for (int i = 0; cmd[i]; i++) {
            fprintf(stderr, "%s ", cmd[i]);
        }
        fprintf(stderr, "\n");
    }

    pid_t pid = fork();
    if (pid == 0) {
        /* child */
        freopen("/dev/null", "r", stdin);
        if (!verbose) {
            freopen("/dev/null", "w", stdout);
            freopen("/dev/null", "w", stderr);
        }
        execvp(cmd[0], (char *const *)cmd);
        _exit(127);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    }
    return -1;
}

/* --- String helpers --- */

char *strndup_safe(const char *src, size_t n) {
    if (!src) return NULL;
    size_t len = strlen(src);
    if (len > n) len = n;
    char *dst = malloc(len + 1);
    memcpy(dst, src, len);
    dst[len] = '\0';
    return dst;
}

char *str_trim(char *str) {
    if (!str) return NULL;
    while (isspace((unsigned char)*str)) str++;
    if (*str == '\0') return str;
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return str;
}
