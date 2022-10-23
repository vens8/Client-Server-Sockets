#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>  
#include<pthread.h> 
#include<time.h>
 
#define MAX 1024
#define PORT 8080
#define CONCURRENT_CONNECTIONS 10
#define MAXIMUM_CONNECTIONS 10
double total = 0;
int connections = 0;
pthread_mutex_t mutex;
FILE *results;

// Structure to store client data 
struct clientData {
    int clientSocket;
    char *IPAddr;
    uint16_t portNum;
    int connectionNumber;
} *clientData;

// Function to calculate and return factorial
long factorial(int x) {
    long fact = 1;
    for (int i = 2; i <= x; i++) {
        fact *= i;
    }
    return fact;
}

// Client thread handler function
void *clientHandler(void *auxClientInfo)
{   
    clock_t start, end;
    start = clock();
    // Get the client data structure
    struct clientData clientData = *(struct clientData*)auxClientInfo;
    int clientSocket = clientData.clientSocket;
    char buffer[MAX];   
    
    // Ideally, open file here  
     
    // Infinite while
    while(1) {   
        bzero(buffer, sizeof(buffer));
        int readStatus = read(clientSocket, buffer, sizeof(buffer));
        if (readStatus == -1) {
            perror("Failed to read from client!\n");
        }
        if (strlen(buffer) == 0) {
            printf("-- Client has disconnected --\n");
            break;
        }
        else {
            // If client is active and sent auxClientInfo valid, respond with factorial
            printf("Client sent: %s\n", buffer);
            long fact = factorial(atoi(buffer));
            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "%ld", fact);
            printf("Sending to client: %s\n", buffer);
            pthread_mutex_lock(&mutex);  // Lock critical section to write to file.
            fprintf(results, "From Client Thread: %ld\nClient Address: %s\nClient Port: %d\nFactorial: %ld\n\n", pthread_self(), clientData.IPAddr, clientData.portNum, fact);
            pthread_mutex_unlock(&mutex);  // Unlock after writing
            write(clientSocket, buffer, sizeof(buffer));
            bzero(buffer, sizeof(buffer));
        }
    }
    end = clock();
    double time_taken = ((double) (end - start)) / CLOCKS_PER_SEC;
    pthread_mutex_lock(&mutex);
    --connections;  // Reduce number of concurrent connections after completion
    total += time_taken;
    printf("Time taken to run thread: %lf seconds\nTime taken so far: %lf seconds\n", time_taken, total);
    pthread_mutex_unlock(&mutex);

    // Below block of code is hardcoded only to meet assignment requirements
    if (clientData.connectionNumber == MAXIMUM_CONNECTIONS - 1) {  // Assuming this is last thread, close file
        printf("File closed\n");
        fclose(results);  // Ideally, close without condition
    }
} 


int main()
{
    int serverSocket, clientSocket, clientAddressLength, flag = 1, *newClientSocket;
    struct sockaddr_in serverAddress, clientAddress; 
    clientAddressLength = sizeof(struct sockaddr_in);
    
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
	
    pthread_t thread_id;
    pthread_attr_t detachedThread;
    pthread_attr_init(&detachedThread);
    pthread_attr_setdetachstate(&detachedThread, PTHREAD_CREATE_DETACHED);
    pthread_mutex_init(&mutex, NULL);

    while(1)
    {   
        // Accept a client
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, (socklen_t*) &clientAddressLength);
        if (clientSocket < 0) {
            perror("Accept Failed!\n");
            exit(0);
        }
        else {
            printf("Connections: %d\n", connections);
            if (connections < CONCURRENT_CONNECTIONS) {
                printf("Fetched client on (%s | %d)\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
                
                clientData = malloc(sizeof(struct clientData));
                clientData->clientSocket = clientSocket;
                clientData->IPAddr = inet_ntoa(clientAddress.sin_addr);
                clientData->portNum = ntohs(clientAddress.sin_port);
                clientData->connectionNumber = connections;

                // Below block of code is hardcoded only to meet assignment requirements
                if (connections == 0) {  // Open file on first client connection
                    printf("File opened!\n");
                    results = fopen("resultsThread.txt", "a");
                }

                if(pthread_create(&thread_id, &detachedThread, clientHandler, (void*) clientData) < 0) {
                    perror("Failed to create a thread!\n");
                    close(clientSocket);
                    exit(0);
                }
                else {
                    printf("Thread created and assigned!\n");
                    ++connections;
                }
            }
            else {
                printf("Connection limit reached. Server busy.\n");
            }
        }
    }

    pthread_attr_destroy(&detachedThread);
    pthread_mutex_destroy(&mutex);

    return 0;
}
