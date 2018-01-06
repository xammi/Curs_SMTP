#include "server.h"


int main(int argc, char *argv[]) {

    Server *server = make_server(100, 3 * 60);
    if (server == NULL) {
        printf("malloc() failed\n");
    }

    int res_code = init_server(server, 9090);
    if (res_code < 0) {
        perror("init_server() failed\n");
        destroy_server(server);
        return -1;
    }

    // main server loop
    run_server(server);

    destroy_server(server);
    return 0;
}
