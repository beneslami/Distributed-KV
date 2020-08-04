#ifndef SOCKET_H
#define SOCKET_H

#define SOCKET_NAME "/tmp/socket"
#define BUFFER_SIZE 128
#define MAX_CLIENT_SUPPORTED   32

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<sys/un.h>
#include<unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct server_struct_ server_struct_t; /* opaque data structure */

/* Function Declarations */
int
init_socket_server(server_struct_t*);
int
bind_create_socket(int, int);
void
refresh(server_struct_t*, fd_set*);
int
get_max(server_struct_t*);
void
add_to_monitor(server_struct_t*, int);
int
get_monitored_fd_set(server_struct_t*, int);
void
remove_from_monitor(server_struct_t*, int);

#endif
