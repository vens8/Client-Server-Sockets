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
#define MAX 1024
#define PORT 8080


// To send structures over sockets, they must be serialised.

int main()
{
	int clientSocket;
	struct sockaddr_in serverAddress;

	// Creating client socket
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == -1) {
		printf("Couldn't create client socket!\n");
		exit(0);
	}
	else
		printf("Successfully created a client socket.\n");
	
    bzero(&serverAddress, sizeof(serverAddress));

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(PORT);

	// Connecting to server
	if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) != 0) {
		perror("Connection failed!\n");
		exit(0);
	}
	else
		printf("Connected established with server.\n");

    char buffer[MAX];
	for (int i = 1; i <= 20; i++) {
        bzero(buffer, sizeof(buffer));
        long request = i;
        char num[32], requestString[32], portString[32], ipAddrString[INET_ADDRSTRLEN];

        sprintf(num, "%d", i);
        strcat(num, "\n");
        strcat(buffer, num);

        sprintf(requestString, "%d", request);
        strcat(requestString, "\n");
        strcat(buffer, requestString);

        sprintf(portString, "%d", serverAddress.sin_port);
        strcat(portString, "\n");
        strcat(buffer, portString);

        inet_ntop(AF_INET, &serverAddress.sin_addr.s_addr, ipAddrString, sizeof(ipAddrString));
        strcat(buffer, ipAddrString);
        strcat(buffer, "\0");
        printf("Sending to server: %s\n", buffer);

		int writeStatus = write(clientSocket, buffer, sizeof(buffer));
        if (writeStatus == -1) {
            perror("Failed to send request to server!\n");
        }

        bzero(buffer, sizeof(buffer));

		int readStatus = read(clientSocket, buffer, sizeof(buffer));
        if (readStatus == -1) {
            perror("Failed to read response from server!\n");
        }
        else
		    printf("Result received from server: %s\n", buffer);

	}

	// close the socket
	close(clientSocket);
}
