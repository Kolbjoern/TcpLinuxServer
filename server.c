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

void *listenForConnections(void *args)
{
	socklen_t sockSize = sizeof(struct sockaddr_in);
	struct sockaddr_in clientInfo; // socket info about the connecting client

	// start listening, allowing a queue of up to 2 pending connections
	listen(serverSock, 2);
	int clientSock = accept(serverSock, (struct sockaddr*) &clientInfo, &sockSize);

	while (clientSock)
	{
		printf("Established connection with %s\n", inet_ntoa(clientInfo.sin_addr));		
		close(clientSock);
		clientSock = accept(serverSock, (struct sockaddr*) &clientInfo, &sockSize);
	}

	return NULL;
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

	// create tcp socket
	serverSock = socket(AF_INET, SOCK_STREAM, 0);

	// bind server information to socket
	bind(serverSock, (struct sockaddr*) &serverInfo, sizeof(struct sockaddr));
	
	pthread_t listenerThread;
	pthread_create(&listenerThread, NULL, listenForConnections, NULL);

	while (1)
	{
		printf("tick....\n");
		sleep(5);

	}

	pthread_join(listenerThread, NULL);
	close(serverSock);

	return 0;
}
