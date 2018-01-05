#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>


typedef struct {
    int listen_fd;
    struct pollfd* fds;
    int active_clients;

    int max_clients;
    int timeout_msec;
} Server;

typedef void (*HandleFunc) (char *in, char *out);

Server* make_server(int max_clients, int timeout);
void destroy_server(Server *server);

int init_server(Server *server, int port);
int run_server(Server *server, HandleFunc handler);

int accept_clients(Server *server);
int handle_client(Server *server, struct pollfd fd_wrap, HandleFunc handler);


#endif // SERVER_H
