#include "inc/utils.h"

extern int VERBOSE = 0;

void set_verbose(int flag) {
    VERBOSE = flag;
    printf("Verbose set: %d\n", flag);
}

void strpart(const char *src, char *dst, regmatch_t match) {
    int len = 0;
    if (match.rm_so >= 0) {
        len = match.rm_eo - match.rm_so;
        strncpy(dst, src + match.rm_so, len);
    }
    dst[len] = '\0';
}

void strncchr(const char *src, char *dst, char sep) {
    int len = (int)(strchr(src, sep) - src);
    strncpy(dst, src, len);
    dst[len] = '\0';
}

void append_path(char *path, const char *dir, int n) {
    int len = strlen(path);
    if (path[len - 1] != '/') {
        strcat(path, "/");
    }
    strncat(path, dir, n);
}

int mkdir_ifno(char *path) {
    struct stat sb;

    if (stat(path, &sb) != 0 || ! S_ISDIR(sb.st_mode)) {
        int res_code = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (res_code != 0) {
            return -1;
        }
    }
    return 0;
}

void create_unique_id(char *unique_id) {
    uuid_t uuid;
    uuid_generate_time(uuid);

    char uuid_str[37];
    uuid_unparse_lower(uuid, uuid_str);
    strncpy(unique_id, uuid_str, 37);
}

int replace_path(char *path, int pos, const char *new_dir) {
    char *cursor = path + strlen(path);
    int cur_pos = 0;
    while (cur_pos != pos) {
        cursor -= 1;
        if (*cursor == '/') {
            cur_pos -= 1;
        }
        if (cursor == path) {
            return -1;
        }
    }
    cursor += 1;
    int I = 0;
    while (*cursor != '/' && cursor != path + strlen(path)) {
        *cursor = new_dir[I];
        cursor += 1;
        I += 1;
    }
    return 0;
}

void formatted_now(char *buffer, int n) {
    time_t now;
    struct tm *time_info;

    time(&now);
    time_info = localtime(&now);

    strftime(buffer, n, "%Y-%m-%d %H:%M:%S", time_info);
}
