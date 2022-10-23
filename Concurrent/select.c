#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>  
#include<sys/select.h>
#include<time.h>
#include<pthread.h>
 
#define MAX 1024
#define PORT 8080
#define CONCURRENT_CONNECTIONS 10
#define MAXIMUM_CONNECTIONS 11
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

int main()
{
    int serverSocket, clientAddressLength, flag = 1, *newClientSocket, selectReturn;
    fd_set readFDSet;
    struct sockaddr_in serverAddress, clientAddress; 
    clientAddressLength = sizeof(struct sockaddr_in);
    int clientConnections[MAXIMUM_CONNECTIONS + 1];
    clock_t start, end;
    
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
     
    // Binding the socket to specified IP and port
    if(bind(serverSocket,(struct sockaddr *)&serverAddress , sizeof(serverAddress)) < 0) {
        perror("Bind failed!\n");
        exit(0);
    }
    printf("Socket binding complete.\n");
     
    // Listen for up to 10 clients
	if ((listen(serverSocket, 11)) != 0) {
		perror("Listen failed!\n");
		exit(0);
	}
	printf("Server listening...\n");

    for(int i = 0; i < MAXIMUM_CONNECTIONS; i++) {
        clientConnections[i] = -1;
    }
    clientConnections[0] = serverSocket;

    pthread_mutex_init(&mutex, NULL);
    while(1) {  
        // Set the FDSet before passing to select call
        FD_ZERO(&readFDSet);

        for (int i = 0; i < MAXIMUM_CONNECTIONS; i++) {
            if (clientConnections[i] >= 0) {
                int sd = clientConnections[i];
                FD_SET(sd, &readFDSet);
            }
        }
        
        // Call select system call
        selectReturn = select(FD_SETSIZE, &readFDSet, NULL, NULL, NULL);

        if (selectReturn >= 0) {
            if (FD_ISSET(serverSocket, &readFDSet)) {
                int clientSocket = accept(serverSocket, (struct sockaddr*) &clientAddress, &clientAddressLength);
                if (clientSocket >= 0) {
                    printf("Fetched client on (%s | %d)\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
                    start = clock();
                    for (int i = 0; i < MAXIMUM_CONNECTIONS; i++) {
                        if (clientConnections[i] < 0) {
                            clientConnections[i] = clientSocket;
                            break;
                        }
                    }
                }
                else {
                    perror("Accept failed!\n");
                }
                selectReturn -= 1;
                if (!selectReturn) continue;
            }

            for (int i = 1; i < MAXIMUM_CONNECTIONS; i++) {
                if ((clientConnections[i] > 0) && (FD_ISSET(clientConnections[i], &readFDSet))) {
                    char buffer[MAX];  
                    int clientSocket = clientConnections[i];
                    bzero(buffer, sizeof(buffer));
                    int selectReturn = read(clientSocket, buffer, sizeof(buffer));
                    if (selectReturn == 0) {
                        printf("-- Client Disconnected --\n");
                        pthread_mutex_lock(&mutex);
                        end = clock();
                        double time_taken = ((double) (end - start)) / CLOCKS_PER_SEC;
                        printf("Time taken to run thread: %lf seconds\n", time_taken);
                        pthread_mutex_unlock(&mutex);
                        close(clientConnections[i]);
                        clientConnections[i]=-1;
                    }
                    if (selectReturn > 0) {
                        printf("Client sent: %s\n", buffer);
                        long fact = factorial(atoi(buffer));
                        bzero(buffer, sizeof(buffer));
                        sprintf(buffer, "%ld", fact);
                        printf("Sending to client: %s\n", buffer);
                        pthread_mutex_lock(&mutex);  // Lock critical section to write to file.
                        results = fopen("resultsSelect.txt", "a");
                        fprintf(results, "From Client Thread: %ld\nClient Address: %s\nClient Port: %d\nFactorial: %ld\n\n", pthread_self(), inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port), fact);
                        fclose(results);
                        pthread_mutex_unlock(&mutex);  // Unlock after writing
                        write(clientSocket, buffer, sizeof(buffer));
                        bzero(buffer, sizeof(buffer));
                    }
                    if (selectReturn == -1) {
                        printf("read failed!\n");
                        break;
                    }
                }
                selectReturn -= 1;
                if (!selectReturn) continue;
            }
        }
    }

    printf("Exiting code!\n");
    for (int i = 0; i < MAXIMUM_CONNECTIONS; i++) {
        if (clientConnections[i] > 0) {
            close(clientConnections[i]);
        }
    }
    pthread_mutex_destroy(&mutex);

    return 0;
}