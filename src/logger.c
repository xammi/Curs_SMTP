#include "inc/logger.h"


// logger_loop() executes in logger process

int logger_loop(const char *log_file_name) {

    key_t key = ftok("/tmp", 'S');
    if (key < 0) {
        perror("ftok() failed");
        return 1;
    }
    int msg_queue = msgget(key, 0644 | IPC_CREAT);
    if (msg_queue < 0) {
        perror("msgget() failed");
        return 1;
    }
    FILE *log_file = fopen(log_file_name, "w");
    if (log_file == NULL) {
        perror("fopen() failed");
        return 2;
    }

    char buffer[512];
    buffer[0] = '\0';

    printf("Logger cycle ready...\n");
    while(1) {
        int res_code = msgrcv(msg_queue, &buffer, sizeof(buffer), 0, 0);
        if (res_code < 0) {
            perror("msgrcv() failed");
            break;
        }
        if (strcmp(buffer, "Stop") == 0) {
            break;
        }
        char now[40];
        formatted_now(now, 40);
        fprintf(log_file, "[%s] %s\n", now, buffer);
    }

    fclose(log_file);
    return 0;
}


// write_log() executes in main process

int write_log(const char *msg) {
    key_t key = ftok("/tmp", 'S');
    if (key < 0) {
        perror("ftok() failed");
        return 1;
    }
    int msg_queue = msgget(key, 0644 | IPC_CREAT);
    if (msg_queue < 0) {
        perror("msgget() failed");
        return 1;
    }

    int res_code = msgsnd(msg_queue, msg, strlen(msg), 0);
    if (res_code < 0) {
        perror("msgsnd() failed");
        return 2;
    }
    return 0;
}

int wait_logger() {
    return write_log("Connecting to log");
}

int stop_logger() {
    return write_log("Stop");
}
