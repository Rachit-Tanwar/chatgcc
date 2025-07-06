#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8888
#define IP "127.0.0.1"
#define BUFFER_SIZE 1024

int sock;
struct sockaddr_in server_addr;
pthread_t thread_id;

void connection();
void message_receiver();
void send_message();
void cleanup();
void *receive_message(void *socket_desc);

void connection(){
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    perror("Socket creation failed!!");
    exit(EXIT_FAILURE);
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, IP, &server_addr.sin_addr) <= 0){
    perror("Invalid server address!!");
    exit(EXIT_FAILURE);
  }

  if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
    perror("Connection to server failed!!");
    exit(EXIT_FAILURE);
  }

  printf("Connected to server\n");
}

void message_receiver(){
  if (pthread_create(&thread_id, NULL, receive_message, (void*)&sock) < 0) {
    perror("Message thread creation failed!!");
    exit(EXIT_FAILURE);
  }
}

void send_message(){
  char buffer[BUFFER_SIZE];

  while (1) {
    printf("Enter message : ");
    fflush(stdout);
    if (fgets(buffer, BUFFER_SIZE, stdin) == NULL){
      break;
    }
    if (send(sock, buffer, strlen(buffer), 0) < 0){
      perror("Failed to send message!!");
      break;
    }
  }
}

void cleanup(){
  printf("\nDisconneted from server...\n");
  close(sock);
  exit(0);
}

void *receive_message(void *socket_desc){
  int sock = *(int*)socket_desc;
  char buffer[BUFFER_SIZE];
  while(1){
    int read_v = read(sock, buffer, BUFFER_SIZE);
    if (read_v <= 0){
      printf("\nConnection lost!!\n");
      break;
    }
    buffer[read_v] = '\0';
    printf("\nReceived: %s", buffer);
    printf("Enter message: ");
    fflush(stdout);
  }
  return NULL;
}

int main(int argc, char *argv[]){

  connection();
  message_receiver();
  send_message();
  cleanup();

  return 0;
}
