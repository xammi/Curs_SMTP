#include "inc/server.h"
#include "inc/logger.h"


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
    config_lookup_string(&cfg, "logger.path", log_file_name);

    int res_code = logger_loop(log_file_name);
    return res_code;
}


int main(int argc, char *argv[]) {
    if (argc >= 2) {
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--verbose") == 0) {
            set_verbose(1);
        }
    }
    config_init(&cfg);
    if (! config_read_file(&cfg, "settings.cfg")) {
        printf("Can not read config\n");
        return -1;
    }

    int pid = fork();
    printf("Process ID: %d\n", getpid());

    if (pid == 0) {
        sleep(2);
        while (wait_logger() != 0) {
            printf("Waiting for logger...\n");
        }
        main_process();
    }
    else {
        logger_process();
    }

    config_destroy(&cfg);
    return 0;
}
