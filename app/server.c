#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include <string.h>
#include <errno.h>
#include <unistd.h>

#define PORT 6379
#define BACKLOG 5
#define BUFFER_SIZE 1024

int init_server_socket();
void handle_client_connection(int);

int main()
{
	// Disable output buffering
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");


	int server_fd, client_addr_len, new_fd, client_fds[30], max_clients = 30;
	int activity, valread, i, sd;
	int max_fd;

	fd_set readfds;
	
	// init all client_socket[] to 0
	for (i = 0; i < max_clients; i++)
		client_fds[i] = 0;


	server_fd = init_server_socket();
	if (server_fd == -1)
		return 1;

	struct sockaddr_in client_addr;


	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);

	//int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
	//handle_client_connection(client_fd);

	while (1) {
		// clear the fd_set
		FD_ZERO(&readfds);

		// add server socket to set
		FD_SET(server_fd, &readfds);
		max_fd = server_fd;

		// add client sockets to set
		for (i = 0; i < 30; i++) {
			// if this is a valid connection
			if (client_fds[i] > 0)
				FD_SET(client_fds[i], &readfds);

			// to calculate nfds
			if (client_fds[i] > max_fd)
				max_fd = client_fds[i];
		}

		// wait for an activity on one of the sockets
		activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
		if ((activity < 0) && (errno != EINTR)) {
			printf("Select error: %s...\n", strerror(errno));
			return 1;
		}

		// if something happened on the master socket, its an incoming connection
		if (FD_ISSET(server_fd, &readfds)) {
			printf("Client trying to connect\n");
			int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
			
			handle_client_connection(client_fd);

			for (i = 0; i < max_clients; i++) {
				if (client_fds[i] == 0) {
					client_fds[i] = client_fd;
					break;
				}
			}
		}

		// else its some IO operation in some client socket
		for (i = 0; i < 30; i++) {
			if (FD_ISSET(client_fds[i], &readfds)) {
				printf("Client %d sent ping message\n", client_fds[i]);
				handle_client_connection(client_fds[i]);
			}
		}

	}

	close(server_fd);

	return 0;
}

int init_server_socket()
{
	int server_fd;

	// create the server socket
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return -1;
	}

	int reuse = 1;

	// set reuse address
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return -1;
	}

	// bind the socket
	struct sockaddr_in serv_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(6379),
		.sin_addr = {htonl(INADDR_ANY)},
	};

	if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return -1;
	}

	// start listening for connection requests
	if (listen(server_fd, BACKLOG) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return -1;
	}

	return server_fd;
}

void handle_client_connection(int client_fd)
{
	const char* msg = "+PONG\r\n";
	char buffer[1024];

	if (client_fd >= 0) {
		printf("Client connected\n");

		while (read(client_fd, buffer, BUFFER_SIZE)) {
			send(client_fd, msg, strlen(msg), 0);
		}
		
	}
	else {
		printf("Client connection failed: %s...\n", strerror(errno));
	}
}