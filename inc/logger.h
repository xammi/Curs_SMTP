#ifndef LOGGER_H
#define LOGGER_H

#include "inc/utils.h"

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>


int wait_logger();
int stop_logger();

int write_log(const char *msg);

int logger_loop(const char *log_file_name);

#endif // LOGGER_H
