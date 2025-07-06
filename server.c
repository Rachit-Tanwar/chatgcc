#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define MAX_CLIENTS 8
#define BUFFER_SIZE 1024

int server_fd, client_sockets[MAX_CLIENTS];
struct sockaddr_in server_address;

void server_init();
void opt_setup();
void bind_and_listen();
void handle_new_connection();
void client_message(int client_index);
void broadcast_message(const char* message, int sender_index);
void disconnected_client(int client_index);

void server_init(){

  for (int i = 0;i < MAX_CLIENTS;i++){
    client_sockets[i] = 0;
  }

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
    perror("Socket failed!!");
    exit(EXIT_FAILURE);
  }
}

void opt_setup(){
  int socket_option = 1;

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &socket_option, sizeof(socket_option))){
    perror("Setting socket options failed!!");
    exit(EXIT_FAILURE);
  }
}

void bind_and_listen(){

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0){
    perror("Bind failed!!");
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 3) < 0){
    perror("Listen failed!!");
    exit(EXIT_FAILURE);
  }
}

void handle_new_connection(){
  int new_socket;
  if ((new_socket = accept(server_fd, NULL, NULL)) < 0){
    perror("Accept connection failed!!");
    return;
  }

  for (int i = 0;i < MAX_CLIENTS;i++){
    if (client_sockets[i] == 0){
      client_sockets[i] = new_socket;
      printf("New client connected\n");
      return;
    }
  }

  printf("Maximum clients reached. Connection rejected.\n");
  close(new_socket);
}

void client_message(int client_index){
  char buffer[BUFFER_SIZE];
  
  int read_v = read(client_sockets[client_index], buffer, BUFFER_SIZE - 1);
  if (read_v <= 0){
    disconnected_client(client_index);
  }
  else {
    buffer[read_v] = '\0';
    printf("Client %d: %s", client_index, buffer);

    broadcast_message(buffer, client_index);
  }
}

void broadcast_message(const char* buffer, int sender_index){
  for (int i = 0;i < MAX_CLIENTS;i++){
    if (client_sockets[i] > 0 && i != sender_index){
      send(client_sockets[i], buffer, strlen(buffer), 0);
    }
  }
}

void disconnected_client(int client_index){
  printf("Client %d disconnected\n", client_index);
  close(client_sockets[client_index]);
  client_sockets[client_index] = 0;
}

int main(int argc, char *argv[]){

  int max_sd, activity;
  fd_set active_sockets;

  server_init();
  opt_setup();
  bind_and_listen();

  printf("Server listening on port %d...\n", PORT);
  printf("Waiting for port connections...\n");

  while (1) {
    FD_ZERO(&active_sockets);
    FD_SET(server_fd, &active_sockets);
    max_sd = server_fd;

    for (int i = 0;i < MAX_CLIENTS;i++){
      int current_socket = client_sockets[i];
      if (current_socket > 0){
        FD_SET(current_socket, &active_sockets);
      }
      if (current_socket > max_sd){
        max_sd = current_socket;
      }
    }

    activity = select(max_sd + 1, &active_sockets, NULL, NULL, NULL);

    if (activity < 0){
      perror("Select error occurred!!");
      continue;
    }

    if (FD_ISSET(server_fd, &active_sockets)){
      handle_new_connection();
    }

    for (int i = 0;i < MAX_CLIENTS;i++){
      if (client_sockets[i] > 0 && FD_ISSET(client_sockets[i], &active_sockets)){
        client_message(i);
      }
    }
}

  return 0;
}
