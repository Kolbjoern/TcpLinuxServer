#ifndef CLIENT_HANDLER_H_
#define CLIENT_HANDLER_H_

typedef struct {
	int *socket;
	int *client_list;
	int *num_clients;
} listener_struct;

void *listen_for_messages(void *args);

#endif // CLIENT_HANDLER_H_
