#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX 1024
#define PORT 8080

// To send structures over sockets, they must be serialised.

// Function to calculate and return factorial
long factorial(int x) {
    long fact = 1;
    for (int i = 2; i <= x; i++) {
        fact *= i;
    }
    return fact;
}

int main()
{
    char buffer[MAX];
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientLength;
    FILE *results;
    results = fopen("results.txt", "w");

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0) {
		printf("Couldn't create server socket!\n");
		exit(1);
	}
	printf("Successfully created a server socket.\n");
	
	// memset(&serverAddress, '\0', sizeof(serverAddress));
    bzero(&serverAddress, sizeof(serverAddress));

    // Specify address and port of server
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(PORT);

	// Binding the socket to specified IP and port
	if ((bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress))) != 0) {
		printf("Bind failed!\n");
		exit(0);
	}
	printf("Socket binding complete.\n");

	// Listen for up to 10 clients
	if ((listen(serverSocket, 10)) != 0) {
		printf("Listen failed!\n");
		exit(0);
	}
	else
		printf("Server listening...\n");

    while(1) {
        // Accept a client
        clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, &clientLength);
        if (clientSocket < 0) {
            perror("Accept failed!\n");
            exit(0);
        }
        printf("Fetched client on (%s | %d)\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

        // Infinite loop that communicates with connected client
        while(1) {
            // Receive the request buffer from client
            int readStatus = read(clientSocket, buffer, sizeof(buffer));
            if (readStatus == -1) {
                perror("Failed to read from client!\n");
            }
            if (strlen(buffer) == 0) {
                printf("-- Client has disconnected --\n");
                break;
            }
            else {
                // If client is active and sent something valid, respond with factorial
                printf("Client sent: %s\n", buffer);
                long fact = factorial(atoi(buffer));
                fprintf(results, "Client Address: %s\nClient Port: %d\nResult: %ld\n\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port), fact);
                sprintf(buffer, "%ld", fact);
                printf("Sending to client: %s\n", buffer);
                write(clientSocket, buffer, sizeof(buffer));
                bzero(buffer, sizeof(buffer));
            }
        }

        // Close client socket
        close(clientSocket);
        printf("Waiting for clients to connect...\n");
    }

	// Close server socket after handling all requests
	close(serverSocket);
    return 0;
}
