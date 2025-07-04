#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8888
#define BUFFER_SIZE 1024

void *receive_message(void *socket_desc);

void *receive_message(void *socket_desc){
  int sock = *(int*)socket_desc;
  char buffer[BUFFER_SIZE];
  while(1){
    int read_v = read(sock, buffer, BUFFER_SIZE);
    if (read_v <= 0){
      break;
    }
    buffer[read_v] = '\0';
    printf("Recieved: %s", buffer);
  }
  return NULL;
}

int main(int argc, char *argv[]){
  int sock = 0;
  struct sockaddr_in serv_addr;
  char buffer[BUFFER_SIZE];
  pthread_t thread_id;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    perror("Socket creation failed!!");
    exit(EXIT_FAILURE);
  }
  
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0){
    perror("Invalid address!!");
    exit(EXIT_FAILURE);
  }
  
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
    perror("Connection failed!!");
    exit(EXIT_FAILURE);
  }

  printf("Connected to server\n");

  if (pthread_create(&thread_id, NULL, receive_message, (void*)&sock) < 0){
    perror("Thread creation failed!!");
    exit(EXIT_FAILURE);
  }

  while (1) {
    printf("Enter message : ");
    fgets(buffer, BUFFER_SIZE, stdin);
    send(sock, buffer, strlen(buffer), 0);
  }

  return 0;
}
