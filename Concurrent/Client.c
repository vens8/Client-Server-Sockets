#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>

#define MAX 1024
#define PORT 8080


void* clientThread() {
    int clientSocket;
	struct sockaddr_in serverAddress;
	char buffer[MAX];
    int flag = 1;

	// Create client socket
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket < 0) {
		printf("Couldn't create client socket!\n");
		exit(0);
	}
    if (setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
        perror("Couldn't set socket options!\n");
        exit(0);
    }
	printf("Successfully created a client socket.\n");
	
	bzero(&serverAddress, sizeof(serverAddress));

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(PORT);

	// Connect client to server
	if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) != 0) {
		perror("Connection failed!\n");
		exit(0);
	}
	printf("Connected established with server.\n");

    for (int i = 1; i <= 20; i++) {

        sprintf(buffer, "%d", i);  // Store integer in buffer string
        printf("Sending to server: %s\n", buffer);

		// Send request and message to client
		int sendStatus = write(clientSocket, buffer, sizeof(buffer));
        if (sendStatus == -1) {
            perror("Failed to send request to server!\n");
        }

		// Clear buffer
        bzero(buffer, sizeof(buffer));

		// Receive response from client
		int readStatus = read(clientSocket, buffer, sizeof(buffer));
        if (readStatus == -1) {
            perror("Failed to read response from server!\n");
        }
        else
		    printf("Response from server: %s\n", buffer);

	}
    printf("\nThread has finished!\n\n");
}

int main()
{	
    pthread_t clients[10];
    pthread_attr_t detachedThread;
    pthread_attr_init(&detachedThread);
    pthread_attr_setdetachstate(&detachedThread, PTHREAD_CREATE_DETACHED);

    for (int i = 0; i < 10; i++) {
        if (pthread_create(clients + i, &detachedThread, &clientThread, NULL) != 0) {
            perror("Failed to create a thread!\n");
            exit(0);
        }
        printf("Thread %d has started...\n", i + 1);
    }
    pthread_attr_destroy(&detachedThread);
    pthread_exit(NULL);
    return 0;
}
