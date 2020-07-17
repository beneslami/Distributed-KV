#include "socket.h"

int main(int argc, char **argv){
  int data_socket, ret;
  char *server_ip = argv[1];
  in_port_t server_port = atoi(argv[2]);
  unsigned int secret;

  data_socket = init_socket_client();
  ret = connect_to_server(data_socket, server_ip, server_port);
  if(ret < 0){
    printf("connection failed\n");
    exit(0);
  }
  printf("connection established\n");


  secret = 100;

  if(send(data_socket, &secret, sizeof(int), 0) == -1){
    printf("sending failed\n");
    exit(0);
  }
  char buffer[50];
  int num = recv(data_socket, buffer, sizeof(buffer), 0);
  if(num <= 0){
    printf("receive failed\n");
    exit(0);
  }
  buffer[num] = '\0';
  int server_secret;
  memcpy(&server_secret, buffer, sizeof(int));
  printf("Server = %u\n", server_secret);

  close(data_socket);
  exit(EXIT_SUCCESS);
}
