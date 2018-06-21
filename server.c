#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORTNUM 9002

int serverSock; // socket for incoming connections

void *listenForConnections(void *vargp)
{
	socklen_t sockSize = sizeof(struct sockaddr_in);
	struct sockaddr_in clientInfo; // socket info acout the connecting client

	// start listening, allowing a queue of up to 2 pending connections
	listen(serverSock, 2);
	int clientSock = accept(serverSock, (struct sockaddr*) &clientInfo, &sockSize);

	while (clientSock)
	{
		printf("Established connection with %s\n", inet_ntoa(clientInfo.sin_addr));
		
		// TODO: send confirmation to the client??
		
		close(clientSock);
		clientSock = accept(serverSock, (struct sockaddr*) &clientInfo, &sockSize);
	}

	return NULL;
}

int main(int argc, char * argv[])
{
	printf("Server init\n");

	struct sockaddr_in serverInfo; // socket info aout the server
	socklen_t sockSize = sizeof(struct sockaddr_in);

	memset(&serverInfo, 0, sizeof(serverInfo)); //zero struct before filling
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("128.199.38.220");
	serverInfo.sin_port = htons(PORTNUM);

	serverSock = socket(AF_INET, SOCK_STREAM, 0);

	// bind server information to socket
	bind(serverSock, (struct sockaddr*) &serverInfo, sizeof(struct sockaddr));
	
	pthread_t listenerThread;
	pthread_create(&listenerThread, NULL, listenForConnections, NULL);

	while (1)
	{
		printf("tick....\n");
		sleep(2);
	}

	pthread_join(listenerThread, NULL);
	close(serverSock);

	return 0;
}
