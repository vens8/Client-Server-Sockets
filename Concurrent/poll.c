#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <poll.h>
#include <limits.h>		/*for MAXIMUM_CONNECTIONS*/
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <pthread.h>


#define MAXIMUM_CONNECTIONS 20

#define PORT 8080
#define MAX 1024
FILE *results;
pthread_mutex_t mutex;

// Function to calculate and return factorial
long factorial(int x) {
    long fact = 1;
    for (int i = 2; i <= x; i++) {
        fact *= i;
    }
    return fact;
}

int main() {
	int serverSocket, clientSocket, currentClient, pollReturn, flag = 1, i, useNow = 0;
    struct pollfd clientConnections[MAXIMUM_CONNECTIONS];
	ssize_t bytesRead, bytesWritten;
	char buffer[MAX];
    clock_t start, end;

	socklen_t clientAddressLength;
	struct sockaddr_in serverAddress, clientAddress;
    clientAddressLength = sizeof(clientAddress);

	// Create server socket
    serverSocket = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP);
    if (serverSocket < 0) {
        perror("Couldn't create server socket!\n");
        exit(0);
    }
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
        perror("Couldn't set socket options!\n");
        exit(0);
    }
    printf("Successfully created a server socket.\n");
     
    // Specify address and port of server
    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(PORT);
     
    // Binding the socket to specified IP and port0
    if(bind(serverSocket,(struct sockaddr *)&serverAddress , sizeof(serverAddress)) < 0) {
        perror("Bind failed!\n");
        exit(0);
    }
    printf("Socket binding complete.\n");
     
    // Listen for up to 10 clients
	if ((listen(serverSocket, MAXIMUM_CONNECTIONS)) != 0) {
		perror("Listen failed!\n");
		exit(0);
	}
	printf("Server listening...\n");

    clientConnections[0].events = POLLIN;
	clientConnections[0].fd = serverSocket;
	for(i = 1 ; i < MAXIMUM_CONNECTIONS; i++) {
		clientConnections[i].fd = -1;
	}

    pthread_mutex_init(&mutex, NULL);
	while(1)
	{
		pollReturn = poll(clientConnections , useNow + 1 , -1);
		if(clientConnections[0].revents & POLLIN) {
			printf("Connection established!\n");

			if((clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress , &clientAddressLength)) < 0) {
				perror("accept error.\n");
				exit(0);
			}	

            printf("Fetched client on (%s | %d)\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
			start = clock();
			for(i = 1; i < MAXIMUM_CONNECTIONS; i++) {
				if(clientConnections[i].fd < 0) {
					clientConnections[i].fd = clientSocket;
					break;
				}
			}

			if(i == MAXIMUM_CONNECTIONS) {
				printf("Connection limit reached. Server busy.\n");
				exit(0);
			}
	
			clientConnections[i].events = POLLIN;
			if(useNow < i) useNow = i;

			if(--pollReturn < 0) continue;
		}
			
		for(i = 1; i <= useNow; i++) {
			if((currentClient = clientConnections[i].fd) < 0)
				continue;
			if(clientConnections[i].revents & (POLLIN | POLLERR)) {			
				bzero(buffer, sizeof(buffer));  // Clear buffer
				if((bytesRead = read(currentClient, buffer, sizeof(buffer))) < 0) {
                    printf("Failed to read from client!\n");
					close(currentClient);				
					clientConnections[i].fd = -1;
				}
                else if (bytesRead == 0) {
                    clientConnections[i].fd = -1;
                    close(currentClient);				
                    printf("-- Client has disconnected --\n");
                    pthread_mutex_lock(&mutex);
                    end = clock();
                    double time_taken = ((double) (end - start)) / CLOCKS_PER_SEC;
                    printf("Time taken to run thread: %lf seconds\n", time_taken);
                    pthread_mutex_unlock(&mutex);
                }
				else {
					printf("Client %d sent: %s\n", i, buffer);
                    long fact = factorial(atoi(buffer));
                    bzero(buffer, sizeof(buffer));
                    sprintf(buffer, "%ld", fact);
                    printf("Sending to client: %s\n", buffer);
                    pthread_mutex_lock(&mutex);  // Lock critical section to write to file.
                    results = fopen("resultsPoll.txt", "a");
                    fprintf(results, "From Client Thread: %ld\nClient Address: %s\nClient Port: %d\nFactorial: %ld\n\n", pthread_self(), inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port), fact);
                    fclose(results);
                    pthread_mutex_unlock(&mutex);  // Unlock after writing
					if((bytesWritten = write(currentClient, buffer, bytesRead)) != bytesRead) {
						perror("Couldn't write to the client socket!\n");
						break;
					}
                    bzero(buffer, sizeof(buffer));

				}
				if(0 >= --pollReturn) break;
			}
		}
	}
    pthread_mutex_destroy(&mutex);
	return 0;
}