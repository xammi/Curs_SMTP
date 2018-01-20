#include "inc/server.h"
#include "inc/logger.h"

#include <signal.h>


int main_process() {
    int max_clients = 100;
    config_lookup_int(&cfg, "server.max_clients", &max_clients);

    int timeout_sec = 180;
    config_lookup_int(&cfg, "server.timeout_sec", &timeout_sec);

    Server *server = make_server(max_clients, timeout_sec);
    if (server == NULL) {
        printf("malloc() failed\n");
        return -1;
    }

    int port = 9090;
    config_lookup_int(&cfg, "server.port", &port);

    int res_code = init_server(server, port);
    if (res_code < 0) {
        perror("init_server() failed");
        destroy_server(server);
        return -1;
    }

    // main server loop
    run_server(server);

    destroy_server(server);
    return 0;
}

int logger_process() {
    const char *log_file_name;
    config_lookup_string(&cfg, "logger.path", &log_file_name);

    int res_code = logger_loop(log_file_name);
    return res_code;
}


int main(int argc, char *argv[]) {
    const char *config_path = "settings.cfg";

    if (argc >= 2) {
        if (strcmp(argv[1], "--verbose") == 0) {
            set_verbose(1);
        }
        else if (strcmp(argv[1], "--version") == 0) {
            printf("Max's SMTP server: v1.0.0");
            return 0;
        }
        else {
            config_path = argv[1];
        }
    }
    config_init(&cfg);
    if (! config_read_file(&cfg, config_path)) {
        printf("Can not read config\n");
        config_destroy(&cfg);
        return -1;
    }

    int pid = fork();
    printf("Process forked [%d]\n", getpid());
    if (pid < 0) {
        perror("fork() failed");
    }
    else if (pid == 0) {
        sleep(1);

        // few attempts
        while (wait_logger() != 0) { sleep(1); perror("wait_logger() failed"); }
        main_process();
        stop_logger();
    }
    else {
        logger_process();
    }

    config_destroy(&cfg);
    return 0;
}
