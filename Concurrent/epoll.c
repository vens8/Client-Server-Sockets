#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <time.h>

#define PORT 8080
#define SIZE 1024
#define MAXIMUM_CONNECTIONS 10
#define MAXIMUM_EVENTS 10
FILE *results;
struct sockaddr_in clientAddress;
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
    int serverSocket, clientSocket, clientAddressLength, flag = 1, *newClientSocket, ePollWaitFDs, ePollFD;
    struct sockaddr_in serverAddress; 
    clientAddressLength = sizeof(struct sockaddr_in);
    struct epoll_event ePollEvent, events[MAXIMUM_EVENTS];
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
	if ((listen(serverSocket, MAXIMUM_CONNECTIONS)) != 0) {
		perror("Listen failed!\n");
		exit(0);
	}
	printf("Server listening...\n");

    ePollEvent.events = EPOLLIN;
    ePollEvent.data.fd = serverSocket;
    if ((ePollFD = epoll_create1(0)) < 0) {
        perror("epoll_create1 failed!\n");
    }

    if (epoll_ctl(ePollFD, EPOLL_CTL_ADD, serverSocket, &ePollEvent) < 0) {
        perror("epoll_ctl failed!");
    }

    int clientConnections[MAXIMUM_CONNECTIONS];
    for (int i = 0; i < MAXIMUM_CONNECTIONS; i++)
    {
        clientConnections[i] = 0;
    }
    
    pthread_mutex_init(&mutex, NULL);
    while (1)
    {
        if ((ePollWaitFDs = epoll_wait(ePollFD, events, MAXIMUM_EVENTS, -1)) < 0)
        {
            perror("epoll_wait unsuccessful!\n");
        }

        for (int n = 0; n < ePollWaitFDs; ++n)
        {
            if (events[n].data.fd == serverSocket)
            {
                clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength);
                if (clientSocket < 0) {
                    perror("Accept Failed!\n");
                    exit(0);
                }

                printf("Fetched client on (%s | %d)\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
                start = clock();

                for (int i = 0; i < MAXIMUM_CONNECTIONS; i++) {
                    if (clientConnections[i] == 0) {
                        ePollEvent.events = EPOLLIN | EPOLLET;
                        clientConnections[i] = clientSocket;
                        ePollEvent.data.fd = clientConnections[i];
                        if (epoll_ctl(ePollFD, EPOLL_CTL_ADD, clientConnections[i], &ePollEvent) < 0) {
                            perror("epoll_ctl Error!\n");
                        }
                        break;
                    }
                }
            }
            for (int i = 0; i < MAXIMUM_CONNECTIONS; i++) {
                if (clientConnections[i] > 0 && events[n].data.fd == clientConnections[i]) {   
                    char buffer[SIZE];
                    struct SEND{
                        struct sockaddr_in data;
                        int num;
                    };
                    int readStatus = read(clientConnections[i], buffer, sizeof(buffer));
                    if (readStatus == 0)
                    {
                        printf("-- Client has disconnected --\n");
                        end = clock();
                        double time_taken = ((double) (end - start)) / CLOCKS_PER_SEC;
                        pthread_mutex_lock(&mutex);
                        printf("Time taken to run thread: %lf seconds\n", time_taken);
                        pthread_mutex_unlock(&mutex);
                        epoll_ctl(ePollFD, EPOLL_CTL_DEL, clientConnections[i], NULL);
                    }
                    else if (readStatus < 0)
                    {
                        epoll_ctl(ePollFD, EPOLL_CTL_DEL, clientConnections[i], NULL);
                    }
                    else {
                        printf("Client sent: %s\n", buffer);
                        long fact = factorial(atoi(buffer));
                        bzero(buffer, sizeof(buffer));
                        sprintf(buffer, "%ld", fact);
                        printf("Sending to client: %s\n", buffer);
                        write(clientConnections[i], buffer, sizeof(buffer));
                        pthread_mutex_lock(&mutex);  // Lock critical section to write to file.
                        results = fopen("resultsEPoll.txt", "a");
                        fprintf(results, "From Client Thread: %ld\nClient Address: %s\nClient Port: %d\nFactorial: %ld\n\n", pthread_self(), inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port), fact);
                        fclose(results);
                        pthread_mutex_unlock(&mutex);  // Unlock after writing
                        bzero(buffer, sizeof(buffer));
                    }
                }
            }
        }
    }

    printf("Exited the program!\n");
    pthread_mutex_destroy(&mutex);
    close(serverSocket);
    return 0;
}