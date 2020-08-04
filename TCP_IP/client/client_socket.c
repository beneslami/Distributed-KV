#include "socket.h"

int
init_socket_client(){
  int socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(socket < 0){
    perror("socket");
    return -1;
  }
  return socketfd;
}

int
connect_to_server(int socketfd, char *ip, int port){
  struct sockaddr_in servAddr;
  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  int rtnVal = inet_pton(AF_INET, ip, &servAddr.sin_addr.s_addr);
  if(rtnVal <= 0){
    printf("inet_pton() failed\n");
    return -1;
  }
  servAddr.sin_port = htons(port);
  if(connect(socketfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0){
    printf("connect failed\n");
    return -1;
  }
  return 1;
}
