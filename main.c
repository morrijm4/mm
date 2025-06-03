#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>

#include "string.c"

#define PORT 8080
#define CONNECTION_QUEUE_SIZE 128

#define REQUEST_BUFFER_SIZE 1024
#define RESPONSE_BUFFER_SIZE 65536
#define PATH_BUFFER_SIZE 256

#define FILE_PATH_BUFFER_SIZE 256

int main() {
  char request_buffer[REQUEST_BUFFER_SIZE];
  char path_buffer[PATH_BUFFER_SIZE];
  char response_buffer[RESPONSE_BUFFER_SIZE];
  char *mime;

  FILE *file = NULL;
  char *file_buffer = NULL;
  long file_size;
  char file_path_buffer[FILE_PATH_BUFFER_SIZE];

  struct sockaddr client_address;
  socklen_t client_address_length;
  int server_socket, client_socket, start, end, ret, yes = 1;
  struct sockaddr_in server_address = {0};



  // Create TCP socket
  server_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (server_socket < 0) {
    return 1;
  }
  printf("Server socket file discriptor: %d\n", server_socket);


  // Allow address to be reused by the socket
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
    bzero(path_buffer, PATH_BUFFER_SIZE);
    do {
	ret = recv(client_socket, request_buffer, REQUEST_BUFFER_SIZE, 0);

	if (ret < 0) {
	    printf("recv: an unexpected error has occured: %d\n", errno);
	    goto close;
	}

	if (ret == 0) {
	    printf("recv: connection has been closed by the client\n");
	    goto close;
	}

	if (strncmp("GET", request_buffer, 3) == 0) {
	    // Copy the path into a seperate buffer

	    end = 0, start = 4; // "GET /path"
				//      ^ start here

	    // Find the end index of the path
	    for (int i = start; i < PATH_BUFFER_SIZE && end == 0; ++i) {
		if (request_buffer[i] == ' ') {
		    end = i;
		}
	    }

	    if (end == 0 || start == end) {
		// Path too big or invalid request
		// TODO: return 4XX error
		goto close;
	    }

	    memcpy(path_buffer, request_buffer + start, end - start);
	}


	printf("%s", request_buffer);
	bzero(request_buffer, REQUEST_BUFFER_SIZE);
    } while (ret == REQUEST_BUFFER_SIZE);

    printf("\n\n");

    // Create the file path
    snprintf(file_path_buffer, FILE_PATH_BUFFER_SIZE, strcmp(path_buffer, "/") == 0 ? "./www%sindex" : "./www%s", path_buffer);

    // Open file
    file = fopen(file_path_buffer, "rb");

    if (file == NULL) {
	if (errno != ENOENT) {
	    perror("fopen");
	    goto close;
	} 

	// Redirect to home page
	snprintf(response_buffer, RESPONSE_BUFFER_SIZE,
		"HTTP/1.1 307 Temporary Redirect\r\n"
		"Location: /\r\n"
		"Content-Length: 0\r\n"
		"\r\n");
	goto send;
    }

    // Get file size
    ret = fseek(file, 0, SEEK_END);
    if (ret < 0) {
	perror("fseek");
	goto close;
    }

    file_size = ftell(file);
    if (file_size < 0) {
	perror("ftell");
	goto close;
    }

    // TODO: handle rewind errors
    rewind(file);

    // Allocate memory for file contents
    file_buffer = malloc(file_size + 1);

    if (file_buffer == NULL) {
	perror("malloc");
	goto close;
    }

    fread(file_buffer, sizeof(char), file_size, file);

    if (ferror(file)) {
	printf("fread: an error reading the file has occured\n");
	goto close;
    }

    // Determine content type
    if (ends_with(file_path_buffer, ".css")) {
	mime = "text/css";
    } else if (ends_with(file_path_buffer, ".js")) {
	mime = "text/javascript";
    } else if (ends_with(file_path_buffer, ".svg")) {
	mime = "image/svg+xml";
    } else if (ends_with(file_path_buffer, ".png")) {
	mime = "image/png";
    } else if (ends_with(file_path_buffer, ".ttf")) {
	mime = "font/ttf";
    } else {
	mime = "text/html";
    }

    // Send a response to the client
    snprintf(response_buffer, RESPONSE_BUFFER_SIZE,
	    "HTTP/1.1 200 OK\r\n"
	    "Content-Type: %s\r\n"
	    "Content-Length: %lu\r\n"
	    "Cache-Control: max-age=604800\r\n"
	    "\r\n",
	    mime, file_size);

send:
    ret = send(client_socket, response_buffer, strlen(response_buffer), 0);

    if (ret == (RESPONSE_BUFFER_SIZE - 1)) {
	// TODO: return a 500 instead
	printf("Response buffer is too small. File size %ld\n", file_size);
    }

    if (ret < 0) {
	perror("send");
	goto close;
    }

    if (file_buffer == NULL) {
	goto close;
    }

    ret = send(client_socket, file_buffer, file_size, 0);

    if (ret < 0) {
	perror("send");
	goto close;
    }


close:
    // Close client socket
    close(client_socket);

    // Close file if open
    if (file != NULL) {
	fclose(file);
	file = NULL;
    }

    // Close file buffer if open
    if (file_buffer != NULL) {
	// TODO: use realloc instead
	free(file_buffer);
	file_buffer = NULL;
    }
  }

  return 0;
}
