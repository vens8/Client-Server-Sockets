/*
Rahul Maddula
*/

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>

#define MAX 1024
#define PORT 8080

int main()
{	
	int clientSocket;
	struct sockaddr_in serverAddress;
	char buffer[MAX];

	// Create client socket
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket < 0) {
		printf("Couldn't create client socket!\n");
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

        sprintf(buffer, "%d", i);
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

	// close the socket
	close(clientSocket);

	return 0;
}

