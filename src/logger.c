#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>


#ifdef LOGGER

int main(int argc, char *argv[]) {
    const char *log_file_name = "/var/log/max_smtp.log";
    if (argc >= 2) {
        log_file_name = argv[1];
    }
    key_t key = ftok("/var", 'c');
    if (key != 0) {
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
        printf("Logger cycle ready ...\n");

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
#endif
