/*
Rahul Maddula
*/

#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
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

// Function to parse received buffer
char ** parseBuffer(char * buffer) {
    char** parsed = NULL;
    // Split buffer by newline character
    char * p = strtok(buffer, "\n");
    int items = 0, i;
    while (p) {
        parsed = realloc(parsed, sizeof (char*) * ++items);
        // If memory allocation fails
        if (parsed == NULL)
            exit(-1);
        parsed[items-1] = p;
        p = strtok (NULL, "\n");
    }
    // Reallocate for the last null element
    parsed = realloc(parsed, sizeof (char*) * (items + 1));
    parsed[items] = 0;
    return parsed;
}

int main()
{
	int serverSocket, clientSocket;
	struct sockaddr_in serverAddress, clientAddress;

    // File to store results based on client request
    FILE *results;
    results = fopen("results.txt", "w");

	// Create server socket
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1) {
		perror("Couldn't create server socket!\n");
		exit(0);
	}
	else
		printf("Successfully created a server socket.\n");
	
    bzero(&serverAddress, sizeof(serverAddress));

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(PORT);

	// Binding the socket to specified IP and port
	if ((bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress))) != 0) {
		perror("Bind failed!\n");
		exit(0);
	}
	else
		printf("Socket binding complete.\n");

	// Listen for up to 10 clients
	if ((listen(serverSocket, 10)) != 0) {
		perror("Listen failed!\n");
		exit(0);
	}
	else
		printf("Server listening...\n");
	int clientLength = sizeof(clientAddress);

	// Accept a client
	clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, &clientLength);
	if (clientSocket < 0) {
		perror("Accept failed!\n");
		exit(0);
	}
	else
		printf("Fetched client!\n");

    char buffer[MAX];
	// Infinite loop that waits for client requests
	while(1) {
		bzero(buffer, MAX);
		// Receive the request buffer from client
		int readStatus = read(clientSocket, buffer, sizeof(buffer));
        if (readStatus == -1) {
            perror("Failed to read from client!\n");
        }
        if (strlen(buffer) == 0) {
            printf("Finished handling requests!\n");
            exit(0);
        }
        // Parsing buffer
        char bufferCopy[MAX];
        strcpy(bufferCopy, buffer);
        char** parsed = NULL;
        parsed = parseBuffer(&bufferCopy);

        printf("Number received from client: %s\n", parsed[1]);
        long fact = factorial(atoi(parsed[1]));
        printf("Factorial calculated: %ld\n", fact);

        // Write to file
        fprintf(results, "Request No: %s\nRequest Data: %s\nRequest Result: %ld\nPort: %s\nIP Address: %s\n\n", parsed[0], parsed[1], fact, parsed[2], parsed[3]);
		// printf("Stored the results into /results.txt.\n");

        // Free/zero allocated memory
        free (parsed);
        bzero(buffer, MAX);
        sprintf(buffer, "%ld", fact);

        // Send the computed factorial to the client
		write(clientSocket, buffer, sizeof(buffer));
	}

    // Close file
    fclose(results);

    // Close client socket
    close(clientSocket);

	// Close server socket after handling all requests
	close(serverSocket);
}
