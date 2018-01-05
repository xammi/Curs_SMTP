#include "server.h"
#include <assert.h>


Server* make_server(int max_clients, int timeout_sec) {
    Server *server = (Server*) malloc(sizeof(Server));
    if (server == NULL) {
        return NULL;
    }
    server->listen_fd = -1;
    server->active_clients = 0;

    server->fds = (struct pollfd *) malloc(sizeof(struct pollfd) * max_clients);
    if (server->fds == NULL) {
        free(server);
        return NULL;
    }
    memset(server->fds, 0, sizeof(struct pollfd) * max_clients);

    server->max_clients = max_clients;
    server->timeout_msec = timeout_sec * 1000;
    return server;
}

void destroy_server(Server *server) {
    if (server != NULL) {
        if (server->fds != NULL) {
            // close for all opened sockets
            for (int I = 0; I < server->active_clients; I++) {
                if (server->fds[I].fd >= 0) {
                    close(server->fds[I].fd);
                    server->fds[I].fd = -1;
                }
            }
            server->active_clients = 0;
            free(server->fds);
        }
        if (server->listen_fd >= 0) {
            close(server->listen_fd);
        }
        free(server);
    }
}

int init_server(Server *server, int port) {
    assert(server);
    assert(server->listen_fd == -1);

    server->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->listen_fd < 0) {
        return -1;
    }

    // set reuse-addr option
    int on = 1;
    int res_code = setsockopt(server->listen_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
    if (res_code < 0) {
        return -1;
    }

    // set non-blocking IO
    res_code = ioctl(server->listen_fd, FIONBIO, (char *)&on);
    if (res_code < 0) {
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    res_code = bind(server->listen_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (res_code < 0) {
        return -1;
    }

    res_code = listen(server->listen_fd, 32);
    if (res_code < 0) {
        return -1;
    }
    return 0;
}

int run_server(Server *server, HandleFunc handler) {
    assert(server);
    assert(server->listen_fd >= 0);
    assert(server->fds);

    server->fds[0].fd = server->listen_fd;
    server->fds[0].events = POLLIN;
    server->active_clients = 1;

    while (1) {
        printf("Waiting on poll()...\n");

        // block until IO operations
        int res_code = poll(server->fds, server->active_clients, server->timeout_msec);
        if (res_code < 0) {
            printf("poll() failed");
            return -1;
        }
        else if (res_code == 0) {
            printf("Timeout expired...\n");
            return 0;
        }

        int active = server->active_clients;
        int compress_fds = 0;

        for (int I = 0; I < active; I++) {
            struct pollfd fd_wrap = server->fds[I];

            if (fd_wrap.revents == 0) {
                continue;
            }

            // unexpected event
            if (fd_wrap.revents != POLLIN) {
                printf("Error: revents == %i\n", fd_wrap.revents);
                return -1;
            }
            if (fd_wrap.fd == server->listen_fd) {
                int res_code = accept_clients(server);
                if (res_code < 0) {
                    return -1;
                }
            }
            else {
                int need_close = handle_client(server, server->fds[I], handler);
                if (need_close != 0) {
                    close(fd_wrap.fd);
                    server->fds[I].fd = -1;
                    compress_fds = 1;
                }
            }
        }
        if (compress_fds > 0) {
            compress_fds = 0;
            for (int I = 0; I < server->active_clients; I++) {
                if (server->fds[I].fd == -1) {
                    for (int J = I; J < server->active_clients; J++) {
                        server->fds[J] = server->fds[J + 1];
                    }
                    server->active_clients -= 1;
                }
            }
        }
    }
    return 0;
}

int accept_clients(Server *server) {
    printf("Listening socket is readable\n");

    while (1) {
        int new_fd = accept(server->listen_fd, NULL, NULL);
        if (new_fd >= 0) {
            printf("New incoming connection - %d\n", new_fd);

            int new_index = server->active_clients;
            server->fds[new_index].fd = new_fd;
            server->fds[new_index].events = POLLIN;
            server->active_clients += 1;
        }
        else {
            if (errno != EWOULDBLOCK) {
                printf("accept() failed\n");
                return -1;
            }
            return 0;
        }
    }
    return 0;
}

int handle_client(Server *server, struct pollfd fd_wrap, HandleFunc handler) {
    printf("Descriptor %d is readable\n", fd_wrap.fd);

    char input_buf[2000];
    while (1) {
        int read_cnt = recv(fd_wrap.fd, input_buf, sizeof(input_buf), 0);
        if (read_cnt > 0) {
            input_buf[read_cnt] = '\0';
            printf("%d bytes received\n", read_cnt);
        }
        else if (read_cnt < 0) {
            if (errno != EWOULDBLOCK) {
                printf("recv() failed\n");
                return -1;
            }
            break;
        }
        else {
            printf("Connection closed by client\n");
            return -1;
        }
    }

    char output_buf[200];
    output_buf[0] = '\0';

    // process request from client
    handler(input_buf, output_buf);

    int written_cnt = send(fd_wrap.fd, output_buf, strlen(output_buf), 0);
    if (written_cnt < 0) {
        perror("send() failed");
        return -1;
    }
    return 0;
}
