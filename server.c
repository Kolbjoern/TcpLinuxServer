#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORTNUM 9002
#define MAX_BUFFER_LEN 500
#define CLIENT_CHUNK 10

void remove_element(int *array, int index, int array_length)
{
 	for (int i = index; i < array_length-1; i++) {
   		array[i] = array[i + 1];
	}
}

int get_client_index(int *array, int client, int array_length)
{
	for (int i = 0; i < array_length; i++) {
		if (client == array[i]){
			return i;
		}
	}
 	return -1;
}

struct listener_struct {
	int *socket;
	int *client_list;
	int *num_clients;
};

char* concat(char *s1, char *s2)
{
	char *result = malloc(strlen(s1) + strlen(s2) + 1);//null term
	strcpy(result, s1);
	strcat(result, s2);
	return result;
}

void *listen_for_messages(void *args)
{
	struct listener_struct *listener_info = (struct listener_struct*)args;

	int client_sock = *listener_info->socket;
	int message_size;
	int buffer[MAX_BUFFER_LEN + 1]; // +1 for null terminator

	while (1) {
		message_size = recv(client_sock, buffer, MAX_BUFFER_LEN, 0);
		if (message_size > 0) {
			//TODO: add sleep to test buffer message stack
			buffer[message_size] = '\0';
//			printf("Message(%d):: %s (%d bytes).\n", client_sock, buffer, message_size);

			printf("Broadcast to entire client list (%d)\n", *listener_info->num_clients);
			for (int i = 0; i < *listener_info->num_clients; i++) {
				write(listener_info->client_list[i], buffer, message_size);
			}

			memset(buffer, '\0', sizeof(buffer));//emtpy buffer
		}
		else if (message_size == 0) {
			printf("Client %d disconnected\n", client_sock);

			int client_index = get_client_index(listener_info->client_list, client_sock, *listener_info->num_clients);
			printf("Remove at index: %d\n", client_index);
			if (client_index == -1) {
				printf("Could not remove client %d\n", client_sock);
				break;
			}
			remove_element(listener_info->client_list, client_index, *listener_info->num_clients);
			(*listener_info->num_clients)--;

			break;
		}
	}

	close(client_sock);
//	free(client_sock);
//	free(listener_info);

	return 0;
}

struct sockaddr_in get_server_address_info()
{
	struct sockaddr_in server_info;
	memset(&server_info, 0, sizeof(server_info)); //zero struct before filling
	server_info.sin_family = AF_INET;
	server_info.sin_addr.s_addr = htonl(INADDR_ANY);
	server_info.sin_port = htons(PORTNUM);
	return server_info;
}

int main(int argc, char * argv[])
{
	if (argc == 2){
		printf("Argument supplied: %s\n", argv[1]);
	}

	printf("SERVER INIT::");

	int *client_list = malloc(CLIENT_CHUNK * sizeof(int));
	int client_roof = CLIENT_CHUNK;
	int client_counter = 0;

	struct sockaddr_in server_info = get_server_address_info();

	// create tcp socket, with IP version 4
	int server_sock = socket(AF_INET, SOCK_STREAM, 0); //socket for incoming connections
	if (server_sock == -1) {
		printf("Could not create socket\n");
	}

	// bind server information to socket
	int bind_response;
	bind_response = bind(server_sock, (struct sockaddr*) &server_info, sizeof(struct sockaddr));
	if (bind_response < 0){
		printf("Could not bind socket\n");
	}

	socklen_t sock_size = sizeof(struct sockaddr_in);
	struct sockaddr_in client_info; // socket info about the connecting client

	// start listening, allowing a queue of up to 20 pending connections
	listen(server_sock, 20);

	printf("OK...\n");
	printf("Hosting on: %s\n", inet_ntoa(server_info.sin_addr));

	int current_client_sock;
	struct listener_struct new_client;
	new_client.client_list = client_list;
	new_client.num_clients = &client_counter;

	while (1) {
		current_client_sock = accept(server_sock, (struct sockaddr*) &client_info, &sock_size);
		printf("Established connection with %s\n", inet_ntoa(client_info.sin_addr));

		client_list[client_counter++] = current_client_sock;
		if (client_counter >= client_roof-1) {//remove -1?
			client_roof += CLIENT_CHUNK;
			client_list = realloc(client_list, client_roof);
			printf("Reallocated clientlist, roof is now at %d\n", client_roof);
		}
		printf("Added %d to client list\n", current_client_sock);
		//TODO: also resize down

		pthread_t listener_thread;
		new_client.socket = malloc(1);//TODO:better malloc assign and free this fool
		*new_client.socket = current_client_sock;
		pthread_create(&listener_thread, NULL, listen_for_messages, (void*) &new_client);
	}

	//TODO:join threads

	close(server_sock);
	free(client_list);

 	return 0;
}
