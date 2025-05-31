#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/_types/_socklen_t.h>

#define PORT 8080
#define CONNECTION_QUEUE_SIZE 128
#define REQUEST_BUFFER_SIZE 1024

int main() {
  char request_buffer[REQUEST_BUFFER_SIZE];
  struct sockaddr client_address;
  socklen_t client_address_length;
  int server_socket, client_socket, ret, yes = 1;
  struct sockaddr_in server_address = {0};

  // Create TCP socket
  server_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (server_socket < 0) {
    return 1;
  }
  printf("Server socket file discriptor: %d\n", server_socket);


  // Set socket options
  ret = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

  if (ret < 0) {
      printf("setsockopt: error setting socket option\n");
      return 1;
  }

  // Bind server socket to an address
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(PORT);

  ret = bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));

  if (ret < 0) {
      perror("bind");
      return 1;
  }


  // Listen for incoming TCP connection
  ret = listen(server_socket, CONNECTION_QUEUE_SIZE);

  if (ret < 0) {
      perror("listen");
      return 1;
  }
  printf("Listening at http://localhost:%d\n", PORT);

  // Accept incoming TCP connections
  for (;;) {
    client_socket = accept(server_socket, &client_address, &client_address_length);

    if (client_socket < 0) {
	perror("accept");
	return 1;
    }

    // Read request data
    do {
	bzero(request_buffer, REQUEST_BUFFER_SIZE);

	ret = recv(client_socket, request_buffer, REQUEST_BUFFER_SIZE, 0);

	if (ret < 0) {
	    printf("recv: an unexpected error has occured");
	    goto close;
	}

	if (ret == 0) {
	    printf("recv: connection has been closed by the client");
	    goto close;
	}

	// printf("Bytes read: %d\n", ret);
	printf("%s", request_buffer);
    } while (ret == REQUEST_BUFFER_SIZE);
    printf("\n\n");


    // Send a response to the client
    const char *response = "HTTP 1.1 200 OK\r\n\r\n";

    send(client_socket, response, strlen(response), 0);

    // Close client socket
close:
    close(client_socket);
  }

  return 0;
}
