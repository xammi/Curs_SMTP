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

    time_t now;
    char buffer[256];
    buffer[0] = '\0';

    while(1) {
        printf("Logger cycle ready...\n");

        int res_code = msgrcv(msg_queue, &buffer, sizeof(buffer), 0, 0);
        if (res_code != 0) {
            perror("msgrcv() failed");
            break;
        }
        if (strcmp(buffer, "stop") == 0) {
            break;
        }
        time(&now);
        fprintf(log_file, "[%s] %s\n", asctime(localtime(now)), buffer);
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
    return write_log("Waiting for log...\n");
}
