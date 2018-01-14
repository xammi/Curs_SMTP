#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <regex.h>
#include <libconfig.h>
#include <time.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <uuid/uuid.h>


int VERBOSE;
void set_verbose(int flag);

config_t cfg;

// strings
void strpart(const char *src, char *dst, regmatch_t match);
void strncchr(const char *src, char *dst, char sep);

// files, dirs, paths
void append_path(char *path, const char *dir, int n);
int replace_path(char *path, int pos, const char *new_dir);
int mkdir_ifno(char *path);

// uuid's
void create_unique_id(char *unique_id);

// timestamps
void formatted_now(char *buffer, int n);

#endif // UTILS_H
