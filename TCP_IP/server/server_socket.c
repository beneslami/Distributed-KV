#include "socket.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct server_struct_{
  int monitored_fd_set[MAX_CLIENT_SUPPORTED];
  int comm_socket_fd;
};

/* getter and setter APIs for opaque data structure */
int
get_monitored_fd_set(server_struct_t *server_attribute, int value){
  return server_attribute->monitored_fd_set[value];
}

static void
set_monitored_fd_set(server_struct_t *server_attribute, int index, int skt_fd){
  server_attribute->monitored_fd_set[index] = skt_fd;
}

static void
initialize_monitor_fd_set(server_struct_t *server_attribute){ /* remove all FDs, if any from the array*/
  for(int i=0;i <MAX_CLIENT_SUPPORTED; i++){
      set_monitored_fd_set(server_attribute, i, -1);
  }
  server_attribute->comm_socket_fd = -1;

}

static void
add_monitor_fd_set(server_struct_t *server_attribute, int index, int skt_fd){ /* add new FD to the array*/
  for(int i=0; i<MAX_CLIENT_SUPPORTED; i++){
    if(get_monitored_fd_set(server_attribute, i) != -1) continue;
    set_monitored_fd_set(server_attribute, i, skt_fd);
    break;
  }
}

static void
refresh_fd_set(server_struct_t *server_attribute, fd_set *fd_set_ptr){/* Clone all the FDs in the array into fd_set data structure*/
  FD_ZERO(fd_set_ptr);
  for(int i=0; i<MAX_CLIENT_SUPPORTED; i++){
    if(get_monitored_fd_set(server_attribute, i) != -1){
        FD_SET(get_monitored_fd_set(server_attribute, i), fd_set_ptr); /* copy monitored_fd_set[i] to fd_set_ptr */
    }
  }
}

static void
remove_monitor_fd_set(server_struct_t *server_attribute, int skt_fd){/* remove the FD from the array*/
  for(int i=0; i<MAX_CLIENT_SUPPORTED; i++){
    if(get_monitored_fd_set(server_attribute, i) != skt_fd) continue;
    set_monitored_fd_set(server_attribute, i,-1);
    break;
  }
}

static
int get_max_fd(server_struct_t *server_attribute){/* get the numerical max value among all FDs which server is monitoring*/
  int max = -1;
  for(int i=0; i<MAX_CLIENT_SUPPORTED; i++){
      if(get_monitored_fd_set(server_attribute, i)> max){
        max = get_monitored_fd_set(server_attribute, i);
      }
  }
  return max;
}

int
get_max(server_struct_t *server_attribute){
  int max;
  max = get_max_fd(server_attribute);
  return max;
}

void
refresh(server_struct_t *server_attribute, fd_set *fd_set_ptr){
  refresh_fd_set(server_attribute, fd_set_ptr);
} /* wrapper for the private function */

void
add_to_monitor(server_struct_t *server_attribute, int fd_set){ /* wrapper for the private function */
  add_monitor_fd_set(server_attribute, 0, fd_set);
}

void
remove_from_monitor(server_struct_t *server_attribute, int fd_set){ /* wrapper for the private function */
  remove_monitor_fd_set(server_attribute, fd_set);
}

int
init_socket_server(server_struct_t *server_attribute){
  int connection_socket;
  initialize_monitor_fd_set(server_attribute);
  add_monitor_fd_set(server_attribute, -1, 0);
  unlink(SOCKET_NAME);
  connection_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(connection_socket == -1){
    perror("socket");
    return -1;
  }
  return connection_socket;
}

int
bind_create_socket(int socketfd, int port_num){
  struct sockaddr_in server_address;
  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_port = htons(port_num);
  server_address.sin_addr.s_addr = INADDR_ANY;
  int ret = bind(socketfd, (struct sockaddr*) &server_address, sizeof(server_address));
  if(ret < 0){
    perror("bind");
    return -1;
  }
  return 0;
}
