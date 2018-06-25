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

void *listenForMessages(void *args)
{
	int clientSock = *(int*) args;
	int messageSize;
	int buffer[MAX_BUFFER_LEN + 1]; // +1 for null terminator

	while (1)
	{
		messageSize = recv(clientSock, buffer, MAX_BUFFER_LEN, 0);
		if (messageSize > 0)
		{
			//TODO: add sleep to test buffer message stack
			buffer[messageSize] = '\0';
			printf("Message(%d):: %s (%d bytes).\n", clientSock, buffer, messageSize);
			memset(buffer, '\0', sizeof(buffer));//emtpy buffer
		}
		else if(messageSize == 0)
		{
			printf("Client %d disconnected\n", clientSock);
			break;
		}
	}

	close(clientSock);

	return 0;
}

struct sockaddr_in getServerAddressInfo()
{
	struct sockaddr_in serverInfo;
	memset(&serverInfo, 0, sizeof(serverInfo)); //zero struct before filling
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr.s_addr = htonl(INADDR_ANY);
	serverInfo.sin_port = htons(PORTNUM);
	return serverInfo;
}

int main(int argc, char * argv[])
{
	printf("Server init\n");

	struct sockaddr_in serverInfo = getServerAddressInfo();

	// create tcp socket, with IP version 4
	int serverSock = socket(AF_INET, SOCK_STREAM, 0); //socket for incoming connections
	if (serverSock == -1)
		printf("Could not create socket\n");

	// bind server information to socket
	int bindResponse;
	bindResponse = bind(serverSock, (struct sockaddr*) &serverInfo, sizeof(struct sockaddr));
	if (bindResponse < 0)
		printf("Could not bind socket\n");

	socklen_t sockSize = sizeof(struct sockaddr_in);
	struct sockaddr_in clientInfo; // socket info about the connecting client

	// start listening, allowing a queue of up to 20 pending connections
	listen(serverSock, 2);
	int currentClientSock;
	int *newClientSock;

	while (1)
	{
		currentClientSock = accept(serverSock, (struct sockaddr*) &clientInfo, &sockSize);
		printf("Established connection with %s\n", inet_ntoa(clientInfo.sin_addr));
		
		pthread_t listenerThread;
		newClientSock = malloc(1);
		*newClientSock = currentClientSock;
		pthread_create(&listenerThread, NULL, listenForMessages, (void*) newClientSock);
	}

	//TODO:join threads?

	close(serverSock);

	return 0;
}
