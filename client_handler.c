#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "client_handler.h"

#define MAX_BUFFER_LEN 500

int get_client_index(int *array, int client, int array_length)
{
	for (int i = 0; i < array_length; i++) {
		if (client == array[i]) {
			return i;
		}
	}
	return -1;
}

void remove_client(int *array, int index, int array_length)
{
	for (int i = 0; i < array_length; i++) {
		array[i] = array[i + 1];
	}
}

void *listen_for_messages(void *args)
{
	listener_struct *listener_info = (listener_struct*)args;
	int client_sock = *listener_info->socket;
	int message_size;
	int buffer[MAX_BUFFER_LEN + 1]; // +1 for null terminator

	while (1){
		message_size = recv(client_sock, buffer, MAX_BUFFER_LEN, 0);
		if (message_size > 0) {
			//TODO: add sleep to test buffer mesage stack
			buffer[message_size] = '\0';
			printf("Message(%d):: %s (%d bytes)\n", client_sock, buffer, message_size);

			for (int i = 0; i < *listener_info->num_clients; i++) {
				write(listener_info->client_list[i], buffer, message_size);
			}

			memset(buffer, '\0', sizeof(buffer));//empty buffer
		} else if (message_size == 0) {
			printf("Client %d disconnected\n", client_sock);

			int index = get_client_index(listener_info->client_list, client_sock, *listener_info->num_clients);
			if (index == -1) {
				printf("Could not remove client %d\n", client_sock);
				break;
			}
			remove_client(listener_info->client_list, index, *listener_info->num_clients);
			(*listener_info->num_clients)--;

			break;
		}
	}

	close(client_sock);
//	free(client_sock);
//	free(listener_info);

	return 0;
}
