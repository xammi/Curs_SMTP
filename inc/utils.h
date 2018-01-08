#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <uuid/uuid.h>
#include <regex.h>


void strpart(const char *src, char *dst, regmatch_t match);
void strncchr(const char *src, char *dst, char sep);

void append_path(char *path, const char *dir, int n);
int replace_path(char *path, int pos, const char *new_dir);
int mkdir_ifno(char *path);

void create_unique_id(char *unique_id);


#endif // UTILS_H
