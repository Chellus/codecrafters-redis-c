#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "time/getmillis.h"

#include "redis/commands.h"
#include "redis/resp_parser.h"
#include "hash_table/hash_table.h"

#define PORT 6379
#define BACKLOG 5

int init_server_socket();
int handle_client_connection(int, hash_table*, long);


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

	hash_table* memory = ht_create();

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
			
			handle_client_connection(client_fd, memory, currentMillis());

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
				// returns 1 if the connection was closed, 0 for success,
				// -1 for error
				int ret = handle_client_connection(client_fds[i], memory, currentMillis());
				if (ret == 1) {
					client_fds[i] = 0;
				}
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

int handle_client_connection(int client_fd, hash_table* memory, long received_at)
{
    char buffer[BUFFER_SIZE];
    int valread;
	int return_value = 0;
	int command;

	char* response = NULL;
	bool free_response = false;

    if (client_fd >= 0) {
        valread = read(client_fd, buffer, BUFFER_SIZE);
        
        if (valread == 0) {
            // Connection was closed by the client
            printf("Client %d disconnected\n", client_fd);
            close(client_fd);
			return_value = 1;
        } else if (valread > 0) {
            // parse message
			int array_len = get_array_len(buffer);
			struct array_element* received = parse_array(buffer);
			command = get_command(received, array_len);

			switch (command) {
			case ECHO:
				response = redis_echo(received, array_len);
				free_response = true;
				break;
			case PING:
				response = redis_ping();
				free_response = false;
				break;
			case SET:
				response = redis_set(memory, received, array_len);
				free_response = false;
				break;
			case GET:
				printf("GET request received at %d\n", received_at);
				response = redis_get(memory, received, array_len, received_at);
				printf("Response from GET: %s\n", response);
				free_response = true;
				break;
			default:
				free_response = false;
				break;
			}

			send(client_fd, response, strlen(response), 0);

			// Free the allocated memory for received array
            for (int i = 0; i < array_len; i++) {
                // Assuming each element in received array needs to be freed
                if (received[i].type == BULK_STRING) {
                    struct bulk_string* str = (struct bulk_string*)received[i].data;
        			free(str->data);       // Free the bulk string data
        			free(received[i].data); // Free the bulk string structure itself
                }
            }
            free(received); // Free the array of elements
			received = NULL;
			if (free_response && response != NULL) {
				free(response);
			}

        } else {
            printf("Read error on client %d: %s\n", client_fd, strerror(errno));
			return_value = -1;
        }
    }


	return return_value;
}