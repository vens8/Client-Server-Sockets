#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<time.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<sys/shm.h>
#include<sys/ipc.h>
#include<signal.h>
#include<semaphore.h>

#define MAX 1024
#define PORT 8080
#define CONCURRENT_CONNECTIONS 10
#define MAXIMUM_CONNECTIONS 10
sem_t semaphore;
FILE *results;

// Function to calculate and return factorial
long factorial(int x) {
    long fact = 1;
    for (int i = 2; i <= x; i++) {
        fact *= i;
    }
    return fact;
}

int main(){

	int serverSocket, clientSocket, flag = 1;
	struct sockaddr_in serverAddress, clientAddress;

	socklen_t clientAddressLength = sizeof(struct sockaddr_in);

	char buffer[MAX];
	pid_t childpid;

	int *connections = mmap(NULL, sizeof(*connections), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	double *total = mmap(NULL, sizeof(*total), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*total = 0;
	*connections = 0;

	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(serverSocket < 0){
		perror("Couldn't create server socket!\n");
		exit(0);
	}

	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
	perror("Couldn't set socket options!\n");
	exit(0);
	}
	printf("Successfully created a server socket.\n");

	// Specify address and port of servers
	bzero(&serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	// Binding the socket to specified IP and port
	if(bind(serverSocket,(struct sockaddr *)&serverAddress , sizeof(serverAddress)) < 0) {
	perror("Bind failed!\n");
	exit(0);
	}
	printf("Socket binding complete.\n");
     
    // Listen for up to 10 clients
	if ((listen(serverSocket, MAXIMUM_CONNECTIONS)) < 0) {
		perror("Listen failed!\n");
		exit(0);
	}
	printf("Server listening...\n"); 

	// Initialise semaphore
	sem_init(&semaphore, 1, 1);

	while(1){
        // Accept a client
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, (socklen_t*) &clientAddressLength);
        if (clientSocket < 0) {
			perror("Accept Failed!\n");
			exit(0);
        }
	else {
		// printf("Connections: %d\n", *connections + 1);
		if (*connections < CONCURRENT_CONNECTIONS) {
		printf("Fetched client on (%s | %d)\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

			sem_wait(&semaphore);
			*connections += 1;  // Increment connections count once established
			sem_post(&semaphore);

			// Create child process using fork on accepting client
			if((childpid = fork()) == 0){
				clock_t start, end;
			start = clock();
				printf("Child process created and assigned!\n");
				close(serverSocket);
				while(1){
					int readStatus = read(clientSocket, buffer, sizeof(buffer));
					if (readStatus == -1) {
						perror("Failed to read from client!\n");
					}
					if (strlen(buffer) == 0) {
						printf("-- Client has disconnected --\n");
						end = clock();  // End clock time for current child process
						sem_wait(&semaphore);
						double time_taken = ((double) (end - start)) / CLOCKS_PER_SEC;
						*total += time_taken;
						*connections -= 1;
						sem_post(&semaphore);
						printf("Time taken to run thread: %lf seconds\nTime taken so far: %lf seconds\n", time_taken, *total); 
						kill(getpid(), SIGTERM);  // Kill child process if client disconnected
					}
					else {
						printf("Client sent: %s\n", buffer);
						long fact = factorial(atoi(buffer));
						bzero(buffer, sizeof(buffer));
						sprintf(buffer, "%ld", fact);
						printf("Sending to client: %s\n", buffer);

						// Semaphore to prevent race conditions
						sem_wait(&semaphore);
						results = fopen("resultsFork.txt", "a");
						fprintf(results, "From Client Process: %ld\nClient Address: %s\nClient Port: %d\nFactorial: %ld\n\n", getpid(), inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port), fact);
						fclose(results);
						sem_post(&semaphore);
						write(clientSocket, buffer, strlen(buffer));
						bzero(buffer, sizeof(buffer));
					}
				}
			}
		}
            else {
            	printf("Connection limit reached. Server busy.\n");
            	}
	    }
	}
	sem_destroy(&semaphore);
	close(clientSocket);
	return 0;
}
